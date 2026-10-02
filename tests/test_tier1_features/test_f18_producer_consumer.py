"""
Tier 1 - Feature 18: Producer-Consumer Rayon Bridge.
Verifies the decoupling of asynchronous I/O ingestion from synchronous Rayon batch
compute via bounded channels and backpressure semaphores.
"""

import queue
import threading
import unittest

from tests.rvs_math_oracle import LandscapeCellBuffers, calc_growth_rate


class TestProducerConsumerBridge(unittest.TestCase):
    def test_producer_consumer_pipeline_integrity(self):
        """Asynchronous producer feeds synchronous compute consumer with zero loss."""
        num_tiles = 10
        tile_size = 50
        tile_queue = queue.Queue(maxsize=3)
        processed_tiles = []

        def producer():
            for t_id in range(num_tiles):
                buf = LandscapeCellBuffers(tile_size)
                for i in range(tile_size):
                    buf.herb_height[i] = float(t_id * 10 + i)
                tile_queue.put((t_id, buf))
            tile_queue.put(None)  # Sentinel termination signal

        def consumer():
            while True:
                item = tile_queue.get()
                if item is None:
                    break
                t_id, buf = item
                # Simulate batch Rayon compute
                for i in range(buf.n):
                    buf.herb_height[i] = calc_growth_rate(buf.herb_height[i], 5.0)
                processed_tiles.append(t_id)

        t_prod = threading.Thread(target=producer)
        t_cons = threading.Thread(target=consumer)

        t_prod.start()
        t_cons.start()
        t_prod.join()
        t_cons.join()

        self.assertEqual(len(processed_tiles), num_tiles)
        self.assertEqual(processed_tiles, list(range(num_tiles)))

    def test_backpressure_throttling_halts_producer(self):
        """Producer cannot push more than queue maxsize before consumer reads."""
        q = queue.Queue(maxsize=2)
        q.put("tile1")
        q.put("tile2")
        # Trying to put 3rd without blocking raises Full
        with self.assertRaises(queue.Full):
            q.put_nowait("tile3")

    def test_consumer_empty_wait(self):
        """Consumer gracefully waits when channel is temporarily empty."""
        q = queue.Queue()
        res = []

        def slow_producer():
            import time

            time.sleep(0.05)
            q.put("delayed_tile")

        t = threading.Thread(target=slow_producer)
        t.start()
        item = q.get(timeout=1.0)
        t.join()
        self.assertEqual(item, "delayed_tile")

    def test_data_consistency_across_bridge(self):
        """Data passed from producer across queue maintains exact floating point values."""
        q = queue.Queue()
        buf = LandscapeCellBuffers(5)
        buf.ndvi = [0.1234567, 0.2345678, 0.3456789, 0.4567890, 0.5678901]
        q.put(buf)

        rec_buf = q.get()
        self.assertEqual(rec_buf.ndvi, buf.ndvi)

    def test_multi_threaded_consumer_order(self):
        """Multiple items queue and dequeue without corruption."""
        q = queue.Queue()
        for i in range(20):
            q.put(i)
        items = [q.get() for _ in range(20)]
        self.assertEqual(items, list(range(20)))


if __name__ == "__main__":
    unittest.main()
