CC ?= gcc
CP ?= g++
DEPFLAGS := -MMD -MP
CPflags := -g -std=c++20 -Wno-write-strings -pedantic -c $(DEPFLAGS)
SHAREFLAGS := -fPIC

libver := 0.3

biopath := Biomass
dmpath := DataManagement
fuelpath := Fuels
succpath := Succession
distpath := Disturbance
buildpath := build

# -----------------------------------------------------------------------------
# Rust & CXX Bridge Paths
# -----------------------------------------------------------------------------
REPO_ROOT := ..
RUST_CRATE_DIR := $(REPO_ROOT)/crates/rvs_core
RUST_TARGET_DIR := $(REPO_ROOT)/target
CXX_BRIDGE_DIR := $(RUST_TARGET_DIR)/cxxbridge
RUST_LIB_DIR := $(RUST_TARGET_DIR)/release
RUST_STATIC_LIB := $(RUST_LIB_DIR)/librvs_core.a
RUST_SOURCES := $(wildcard $(RUST_CRATE_DIR)/src/*.rs) $(RUST_CRATE_DIR)/Cargo.toml $(RUST_CRATE_DIR)/build.rs

# Include directories: local librvs headers, CXX bridge generated headers, repo root
INCLUDES := -I. -I$(CXX_BRIDGE_DIR) -I$(REPO_ROOT)

# Linker flags for Rust static library and system runtime dependencies
RUST_LINK_FLAGS := -Wl,--whole-archive $(RUST_STATIC_LIB) -Wl,--no-whole-archive -lpthread -ldl -lm

# -----------------------------------------------------------------------------
# Source & Target Enumeration
# -----------------------------------------------------------------------------
vpath %.cpp $(biopath) $(dmpath) $(fuelpath) $(succpath) $(distpath)

sources := $(wildcard $(biopath)/*.cpp $(dmpath)/*.cpp $(fuelpath)/*.cpp $(succpath)/*.cpp $(distpath)/*.cpp)
objects := $(patsubst %,$(buildpath)/%,$(notdir $(sources:.cpp=.o)))

# Concrete file targets
SO_VERSIONED := $(buildpath)/librvs.so.$(libver)
SO_SYMLINK   := $(buildpath)/librvs.so
RVS_EXE      := $(buildpath)/rvs

.PHONY: all rust_core lib exe clean cleangit

all: rust_core lib exe

# -----------------------------------------------------------------------------
# 1. Rust Core Compilation & CXX Header Generation
# -----------------------------------------------------------------------------
rust_core:
	cargo build --release --manifest-path $(RUST_CRATE_DIR)/Cargo.toml

$(RUST_STATIC_LIB): $(RUST_SOURCES)
	cargo build --release --manifest-path $(RUST_CRATE_DIR)/Cargo.toml

# -----------------------------------------------------------------------------
# 2. Shared Library (librvs.so)
# -----------------------------------------------------------------------------
lib: $(SO_SYMLINK)

$(SO_SYMLINK): $(SO_VERSIONED)
	ln -sf librvs.so.$(libver) $@

$(SO_VERSIONED): $(objects) $(RUST_STATIC_LIB) | $(buildpath)
	$(CP) -shared -Wl,-soname,librvs.so.$(libver) \
		$(objects) \
		$(RUST_LINK_FLAGS) -lsqlite3 \
		-o $@

# -----------------------------------------------------------------------------
# 3. Executable (rvs)
# -----------------------------------------------------------------------------
exe: $(RVS_EXE)

$(RVS_EXE): $(buildpath)/main.o $(SO_SYMLINK) | $(buildpath)
	$(CP) -o $@ $(buildpath)/main.o \
		-L$(buildpath) -lrvs -lsqlite3 -lpthread -ldl -lm \
		-Wl,-rpath,'$$ORIGIN'

$(buildpath)/main.o: main.cpp | rust_core $(buildpath)
	$(CP) $(CPflags) $(INCLUDES) $< -o $@

$(buildpath):
	mkdir -p $(buildpath)

# -----------------------------------------------------------------------------
# 4. Pattern Rule for C++ Object Files
# Order-only prerequisite | rust_core guarantees CXX headers exist before compiling
# -----------------------------------------------------------------------------
$(buildpath)/%.o: %.cpp | rust_core $(buildpath)
	$(CP) $(CPflags) $(INCLUDES) $(SHAREFLAGS) $< -o $@

# -----------------------------------------------------------------------------
# 5. Header Dependency Inclusion
# -----------------------------------------------------------------------------
-include $(wildcard $(buildpath)/*.d)

# -----------------------------------------------------------------------------
# 6. Clean Rules
# -----------------------------------------------------------------------------
clean: 
	rm -rfv $(buildpath)/*.o $(buildpath)/*.d $(buildpath)/*.so* $(buildpath)/rvs $(buildpath)/librvs

cleangit: 
	rm -rfv $(buildpath)
