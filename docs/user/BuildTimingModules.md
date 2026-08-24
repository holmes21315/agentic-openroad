# Building OpenROAD STA and Timing Modules with CMake

This guide explains how to compile only the Static Timing Analysis (STA) and timing-related components of OpenROAD using CMake, rather than building the entire project.

## Table of Contents

- [Overview](#overview)
- [Module Architecture](#module-architecture)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Selective Compilation](#selective-compilation)
- [Testing](#testing)
- [Advanced Build Options](#advanced-build-options)
- [Troubleshooting](#troubleshooting)

## Overview

OpenROAD's timing analysis and optimization functionality is distributed across several modules:

| Module | Purpose | Primary Use |
|--------|---------|-------------|
| **sta** | Core Static Timing Analysis engine (OpenSTA) | Timing path analysis, slack calculation |
| **odb** | OpenDB - Design database | Data model for all modules |
| **dbSta** | Bridge between OpenSTA and OpenDB | STA-database integration layer |
| **rsz** | Gate Resizer - Timing fixing engine | Setup/hold violation repair, design optimization |
| **cts** | Clock Tree Synthesis (TritonCTS) | Clock network construction |
| **est** | Parasitics estimation | Pre-routing RC estimation |
| **rcx** | Parasitic extraction (OpenRCX) | Post-routing RC extraction |
| **stt** | Steiner tree construction | Timing-driven net topology |
| **grt** | Global routing | Timing-aware routing |

## Module Architecture

The dependency hierarchy for timing modules is:

```
OpenSTA (src/sta/)          ← Core timing analysis engine
    ↓
OpenDB (src/odb/)           ← Design database
    ↓
dbSta (src/dbSta/)          ← STA-database bridge
    ↓
┌───┴────────┬──────────┬──────────┬──────────┐
│            │          │          │          │
rsz         cts        est        rcx       stt     ← Timing tools
(resizer)   (CTS)   (estimate) (extract) (steiner)
```

**Key Points:**
- CMake automatically builds dependencies, so building `rsz_lib` also builds `dbSta_lib`, `OpenSTA`, and `odb`
- The `src/sta/` directory is an external submodule (OpenSTA) managed upstream
- Most timing optimizations are performed by the `rsz` (Resizer) module

## Prerequisites

### System Requirements

Before building, ensure you have the required dependencies installed.

#### Install Base Dependencies

```bash
cd /path/to/OpenROAD

# For Ubuntu/Debian:
sudo ./etc/DependencyInstaller.sh -base

# For macOS (using virtual environment - recommended):
python3 -m venv .venv
source .venv/bin/activate
./etc/DependencyInstaller.sh -base
```

#### Install Common Dependencies

```bash
# For Linux:
./etc/DependencyInstaller.sh -common -local

# For macOS (with virtual environment activated):
./etc/DependencyInstaller.sh -common -local
```

```{note}
The `-local` flag installs dependencies to `./dependencies/install` instead of `/usr/local`.
Use `-prefix <PATH>` to specify a custom installation location.
```

### Required Tools

- **CMake** ≥ 3.16
- **C++ Compiler** with C++20 support (GCC ≥ 9, Clang ≥ 10, AppleClang ≥ 14)
- **SWIG** ≥ 4.3
- **Tcl** ≥ 8.6
- **Python3** (optional, for Python bindings)

### Verify Installation

```bash
cmake --version     # Should be ≥ 3.16
swig -version       # Should be ≥ 4.3
gcc --version       # Should support C++20
```

## Quick Start

### Complete Initial Build

For first-time users, build the entire project once to ensure all dependencies are satisfied:

```bash
cd /path/to/OpenROAD

# Method 1: Using Build.sh (recommended for first build)
./etc/Build.sh

# Method 2: Manual CMake configuration
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j$(nproc)
```

```{note}
On macOS, replace `$(nproc)` with `$(sysctl -n hw.ncpu)` to get the CPU core count.
```

After the initial build completes, you can use selective compilation for faster rebuilds.

## Selective Compilation

### Build Individual Timing Modules

Once the initial configuration is complete, you can rebuild specific modules:

```bash
cd /path/to/OpenROAD/build

# Build only specific timing libraries
make OpenSTA -j$(nproc)        # Core STA engine
make odb -j$(nproc)            # Design database
make dbSta_lib -j$(nproc)      # STA-database integration
make rsz_lib -j$(nproc)        # Resizer (timing fixing)
make cts_lib -j$(nproc)        # Clock tree synthesis
make est_lib -j$(nproc)        # Parasitics estimation
make rcx_lib -j$(nproc)        # Parasitics extraction
make stt_lib -j$(nproc)        # Steiner tree
make grt_lib -j$(nproc)        # Global routing
```

### Build Multiple Modules at Once

```bash
cd /path/to/OpenROAD/build

# Build all core timing modules
make dbSta_lib rsz_lib cts_lib -j$(nproc)

# Build full timing stack (including parasitics)
make dbSta_lib rsz_lib cts_lib est_lib rcx_lib -j$(nproc)
```

### Build the OpenROAD Binary

To use the timing tools interactively, you need the `openroad` executable:

```bash
cd /path/to/OpenROAD/build

# Build just the openroad binary (dependencies built automatically)
make openroad -j$(nproc)

# Run openroad
./bin/openroad
```

### Common Build Scenarios

#### Scenario 1: Timing Analysis Only

If you only need STA capabilities:

```bash
cd /path/to/OpenROAD/build
make dbSta_lib -j$(nproc)
make openroad -j$(nproc)
```

This builds:
- OpenSTA (timing engine)
- OpenDB (database)
- dbSta (integration layer)
- openroad binary

#### Scenario 2: Timing Fixing and Optimization

For timing violation repair:

```bash
cd /path/to/OpenROAD/build
make rsz_lib -j$(nproc)
make openroad -j$(nproc)
```

This builds:
- Everything from Scenario 1
- Resizer (repair_design, repair_timing)
- Supporting libraries (stt, est)

#### Scenario 3: Complete Timing Flow

For full timing closure including CTS and parasitics:

```bash
cd /path/to/OpenROAD/build
make rsz_lib cts_lib est_lib rcx_lib -j$(nproc)
make openroad -j$(nproc)
```

This builds the complete timing toolchain.

## Testing

### Running Module-Specific Tests

Each module has its own test suite. To build and run tests:

```bash
# Build tests for a specific module
cd /path/to/OpenROAD/build
make dbSta_tests -j$(nproc)
make rsz_tests -j$(nproc)
make cts_tests -j$(nproc)

# Run tests for a specific module
cd /path/to/OpenROAD/src/dbSta
./test/regression              # Run all dbSta tests
./test/regression read_verilog1  # Run specific test

cd /path/to/OpenROAD/src/rsz
./test/regression              # Run all resizer tests
./test/regression repair_timing1  # Run specific test
```

### Running Flow Tests

Flow tests verify end-to-end timing functionality:

```bash
cd /path/to/OpenROAD
./test/regression flow
```

### Test Configuration Files

Test reference files are located in each module's `test/` directory:

- `*.ok` - Expected output logs
- `*.metrics` - Performance metrics
- `*.metrics_limits` - Acceptable metric ranges

### Updating Test References

If you modify code and tests fail, update the reference files:

```tcl
# In openroad Tcl shell
save_ok <TEST_NAME>                    # Update .ok files
save_flow_metrics <TEST_NAME>          # Update .metrics files
save_flow_metrics_limits <TEST_NAME>   # Update .metrics_limits files
```

## Advanced Build Options

### Build Types

```bash
# Debug build (for development and debugging)
cmake .. -DCMAKE_BUILD_TYPE=DEBUG

# Release build (optimized, default)
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
```

```{note}
Debug builds enable assertions and add debugging symbols but run slower.
Release builds are optimized for performance.
```

### Custom Install Prefix

```bash
# Install to custom location
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/openroad

# After building
make install
```

### Disable Link-Time Optimization (LTO)

LTO improves runtime by ~11% but adds ~1 minute to compile time:

```bash
# Disable LTO for faster compilation
cmake .. -DLINK_TIME_OPTIMIZATION=OFF
```

### Enable Address Sanitizer

For debugging memory errors:

```bash
cmake .. -DASAN=ON
make rsz_lib -j$(nproc)
```

```{warning}
ASAN builds are slower and consume more RAM. Use only for development.
```

### Custom Tcl Library Path

If Tcl is not in the default location:

```bash
cmake .. -DTCL_LIBRARY=/path/to/tcl/lib -DTCL_HEADER=/path/to/tcl.h
```

### Disable Tests

To skip building test infrastructure:

```bash
cmake .. -DENABLE_TESTS=OFF
```

### Use System Libraries

To use system-installed dependencies instead of submodules:

```bash
# Use system OpenSTA
cmake .. -DUSE_SYSTEM_OPENSTA=ON -DOPENSTA_HOME=/path/to/opensta

# Use system Boost
cmake .. -DUSE_SYSTEM_BOOST=ON
```

## Troubleshooting

### Common Issues

#### Issue: "OpenSTA not found"

**Solution:** Ensure submodules are initialized:

```bash
git submodule update --init --recursive
```

#### Issue: "SWIG version too old"

**Solution:** Install SWIG ≥ 4.3:

```bash
# Ubuntu/Debian
sudo apt-get install swig

# macOS
brew install swig
```

#### Issue: "Tcl library not found"

**Solution:** Install Tcl development packages:

```bash
# Ubuntu/Debian
sudo apt-get install tcl-dev

# macOS
brew install tcl-tk
```

Then specify the path if needed:

```bash
cmake .. -DTCL_LIBRARY=/usr/local/lib/libtcl8.6.so \
         -DTCL_HEADER=/usr/local/include/tcl.h
```

#### Issue: Build fails with "undefined reference" errors

**Solution:** Clean build directory and reconfigure:

```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make rsz_lib -j$(nproc)
```

#### Issue: Tests pass in Debug but fail in Release

**Cause:** Debug builds zero-initialize memory; Release builds leave garbage.

**Solution:** Check for uninitialized variables in your code. See [Build Pitfalls](../agents/build.md) for details.

### Incremental Builds

After modifying code, rebuild only affected targets:

```bash
cd /path/to/OpenROAD/build

# Rebuild single module
make rsz_lib -j$(nproc)

# Rebuild and re-link openroad binary
make openroad -j$(nproc)
```

### Clean Builds

For a complete rebuild:

```bash
# Clean all build artifacts
cd /path/to/OpenROAD/build
make clean

# Or remove and recreate build directory
cd /path/to/OpenROAD
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make rsz_lib -j$(nproc)
```

### Verbose Build Output

To see full compilation commands:

```bash
make VERBOSE=1 rsz_lib
```

### Checking Build Logs

Build logs are saved to:

```bash
cat /path/to/OpenROAD/build/openroad_build.log
```

Upload this log when filing issues.

## Module Source Locations

| Module | CMakeLists.txt | Source Code | Headers |
|--------|----------------|-------------|---------|
| sta | `src/sta/CMakeLists.txt` | `src/sta/` (submodule) | `src/sta/include/` |
| odb | `src/odb/CMakeLists.txt` | `src/odb/src/db/` | `src/odb/include/odb/` |
| dbSta | `src/dbSta/CMakeLists.txt` | `src/dbSta/src/` | `src/dbSta/include/db_sta/` |
| rsz | `src/rsz/CMakeLists.txt` | `src/rsz/src/` | `src/rsz/include/rsz/` |
| cts | `src/cts/CMakeLists.txt` | `src/cts/src/` | `src/cts/include/cts/` |
| est | `src/est/CMakeLists.txt` | `src/est/src/` | `src/est/include/` |
| rcx | `src/rcx/CMakeLists.txt` | `src/rcx/src/` | `src/rcx/include/rcx/` |

## Key Files to Understand

### Build Configuration

- `/CMakeLists.txt` - Top-level CMake configuration
- `/src/CMakeLists.txt` - Module registration (lines 248-290)
- `/src/{module}/CMakeLists.txt` - Module-specific build rules

### Timing Module Entry Points

- `src/dbSta/src/dbSta.cc` - Main STA integration (52 KB)
- `src/dbSta/src/dbNetwork.cc` - Database network adapter (190 KB)
- `src/rsz/src/Resizer.cc` - Timing fixing implementation
- `src/rsz/src/RepairDesign.cc` - Design repair logic (81 KB)
- `src/rsz/src/RepairTiming.cc` - Setup/hold violation repair
- `src/cts/src/TritonCTS.cpp` - Clock tree synthesis

## Critical Development Notes

```{warning}
**IMPORTANT:** Follow these rules when modifying timing code:
```

1. **Never modify `src/sta/` files** - OpenSTA is managed upstream. Prefer fixes in `src/dbSta/` or `src/rsz/` when possible. Ask before modifying `src/sta/`.

2. **Always run `clang-format`** before committing C++ files:
   ```bash
   clang-format -i src/rsz/src/Resizer.cc
   ```
   **NEVER** format `src/sta/*` or `*.i` files.

3. **Always use `git commit -s`** for DCO (Developer Certificate of Origin) compliance:
   ```bash
   git commit -s -m "rsz: Fix timing optimization bug"
   ```

4. **Update submodule references** after amending submodule commits:
   ```bash
   git submodule update --init --recursive
   ```

5. **Trace bugs upstream** - When a bug appears in output (e.g., Verilog), find the data creation point (e.g., `buffer_ports`, `remove_buffers`), not the serialization point (e.g., `VerilogWriter`).

## Next Steps

- See [Resizer Documentation](../main/src/rsz/README.md) for timing fixing commands
- See [Testing Guide](../agents/testing.md) for detailed testing procedures
- See [Coding Patterns](../agents/coding.md) for development best practices
- See [Git & CI Guide](../agents/ci.md) for commit and CI workflows

## Additional Resources

- **OpenSTA Documentation**: [https://github.com/The-OpenROAD-Project/OpenSTA](https://github.com/The-OpenROAD-Project/OpenSTA)
- **OpenROAD Documentation**: [https://openroad.readthedocs.io](https://openroad.readthedocs.io)
- **OpenROAD-flow-scripts**: [https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts](https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts)

## License

BSD 3-Clause License. See [LICENSE](../../LICENSE) file.
