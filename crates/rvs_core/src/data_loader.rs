//! Asynchronous DataLoader and tile streaming pipeline adhering to `rust-async-patterns`.
//!
//! Decouples I/O ingestion (raster GeoTIFF / SQLite / vector) from CPU batch computation
//! using Tokio bounded channels, async traits, and cooperative cancellation tokens.

use std::sync::Arc;
use async_trait::async_trait;
use tokio::sync::mpsc;
use tokio_util::sync::CancellationToken;
use tracing::{info, instrument};
use thiserror::Error;
use crate::soa::LandscapeCellBuffers;
use crate::batch::ReferenceData;

/// Errors arising during asynchronous data ingestion.
#[derive(Debug, Error)]
pub enum DataLoaderError {
    #[error("I/O error during ingestion: {0}")]
    Io(#[from] std::io::Error),

    #[error("Channel communication error: {0}")]
    Channel(String),

    #[error("Data parsing or validation error: {0}")]
    Validation(String),

    #[error("Ingestion task cancelled")]
    Cancelled,
}

/// Spatial landscape tile holding canonical SoA cell buffers.
#[derive(Debug, Clone, PartialEq)]
pub struct LandscapeTileSoA {
    pub tile_id: u64,
    pub x_offset: usize,
    pub y_offset: usize,
    pub width: usize,
    pub height: usize,
    pub buffers: LandscapeCellBuffers,
}

/// Abstract async DataLoader interface decoupling simulation logic from storage schemas.
#[async_trait]
pub trait AsyncDataLoader: Send + Sync {
    /// Streams landscape tiles into a bounded mpsc sender with cooperative cancellation.
    async fn stream_landscape_tiles(
        &self,
        sender: mpsc::Sender<LandscapeTileSoA>,
        cancel: CancellationToken,
    ) -> Result<(), DataLoaderError>;

    /// Loads static reference coefficients catalog into memory.
    fn load_reference_data(&self) -> Result<ReferenceData, DataLoaderError>;
}

/// Producer-Consumer Pipeline Runner:
///
/// Decouples asynchronous tile I/O from synchronous Rayon batch processing via bounded
/// mpsc channels and structured concurrency.
#[instrument(skip(loader, process_tile))]
pub async fn run_producer_consumer_pipeline<F>(
    loader: Arc<dyn AsyncDataLoader>,
    channel_capacity: usize,
    mut process_tile: F,
) -> Result<(), DataLoaderError>
where
    F: FnMut(LandscapeTileSoA) -> Result<(), DataLoaderError> + Send,
{
    if channel_capacity == 0 {
        return Err(DataLoaderError::Validation(
            "channel_capacity must be greater than zero".into(),
        ));
    }

    let (tx, mut rx) = mpsc::channel::<LandscapeTileSoA>(channel_capacity);
    let cancel = CancellationToken::new();
    let cancel_producer = cancel.clone();
    let producer_loader = Arc::clone(&loader);

    // Background Tokio task prefetching tiles
    let producer_handle = tokio::spawn(async move {
        info!("Starting tile prefetching producer task");
        producer_loader
            .stream_landscape_tiles(tx, cancel_producer)
            .await
    });

    // Consumer loop processing tiles as they arrive without blocking async runtime
    let mut consumer_err = None;
    while let Some(tile) = rx.recv().await {
        let res = match tokio::runtime::Handle::try_current() {
            Ok(handle) if handle.runtime_flavor() == tokio::runtime::RuntimeFlavor::MultiThread => {
                tokio::task::block_in_place(|| process_tile(tile))
            }
            _ => process_tile(tile),
        };
        if let Err(err) = res {
            cancel.cancel();
            consumer_err = Some(err);
            break;
        }
    }

    // Explicitly drop receiver to close channel and awaken any producer blocked on send()
    drop(rx);

    // Await producer task completion and propagate any error
    let producer_res = producer_handle
        .await
        .map_err(|e| DataLoaderError::Channel(e.to_string()))?;

    if let Some(err) = consumer_err {
        return Err(err);
    }

    producer_res?;

    info!("Producer-consumer pipeline completed successfully");
    Ok(())
}
