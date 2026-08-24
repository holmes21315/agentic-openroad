# OpenROAD Compilation Guide - Full and Incremental Builds

Complete guide for compiling OpenROAD timing modules using Bazel and CMake.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Full Compilation (First Time)](#full-compilation-first-time)
- [Incremental Compilation](#incremental-compilation)
- [Selective Compilation](#selective-compilation)
- [Adding GUI Incrementally](#adding-gui-incrementally)
- [Build Performance Comparison](#build-performance-comparison)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### System Requirements

- **OS:** macOS, Linux, or WSL2
- **RAM:** 8GB minimum, 16GB recommended
- **Disk:** 20GB free space
- **CPU:** Multi-core recommended (8+ cores ideal)

### One-Time Setup

```bash
cd /path/to/OpenROAD

# 1. Initialize git submodules (REQUIRED)
git submodule update --init --recursive

# 2. Install dependencies (macOS)
python3 -m venv .venv
source .venv/bin/activate
./etc/DependencyInstaller.sh -base
./etc/DependencyInstaller.sh -common -local

# 3. Install Bazel (recommended) or CMake
# Option A: Bazel (recommended)
brew install bazelisk

# Option B: CMake (alternative)
brew install cmake swig boost tcl-tk
```

**Important:** Git submodules MUST be initialized before any build!

---

## Full Compilation (First Time)

### Method 1: Bazel (Recommended)

**Why Bazel?**
- ✅ Better incremental builds
- ✅ Automatic dependency management
- ✅ Cross-platform consistency
- ✅ No version conflicts

**Build All Timing Modules (Recommended)**

```bash
cd /Users/sherlock/OpenROAD

# Build timing modules with all tests
bazelisk build //src/dbSta/... //src/rsz/... //src/cts/... //src/est/... //src/rcx/...

# Result:
# - Time: ~1.5-2 hours (first build)
# - Compiles: ~32,000 source files
# - Output: bazel-bin/src/{dbSta,rsz,cts,est,rcx}/
```

**Build Only Libraries (Faster - Skip Tests)**

```bash
# Build just the libraries (60% faster)
bazelisk build \
  //src/dbSta:dbSta_lib \
  //src/rsz:rsz_lib \
  //src/cts:cts_lib \
  //src/est:est_lib \
  //src/rcx:rcx_lib

# Result:
# - Time: ~30-40 minutes (first build)
# - Skips: Test executables, development tools
# - Same functionality: All timing features included
```

**Build Complete OpenROAD Binary**

```bash
# Build openroad executable (without GUI)
bazelisk build //:openroad

# Build with GUI
bazelisk build --//:platform=gui //:openroad

# Run it
./bazel-bin/openroad
```

**Install System-Wide**

```bash
# Install to /usr/local
bazelisk run //:install

# Install to custom location
bazelisk run //:install -- /opt/openroad
```

### Method 2: CMake (Alternative)

**Full Build**

```bash
cd /Users/sherlock/OpenROAD

# Configure
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE

# Build everything
make -j$(nproc)

# Build specific timing modules
make dbSta_lib rsz_lib cts_lib est_lib rcx_lib -j$(nproc)

# Install
sudo make install
```

**Without GUI (Faster)**

```bash
# Edit src/CMakeLists.txt first:
# Comment out: add_subdirectory(gui)

# Then build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j$(nproc)
```

---

## Incremental Compilation

**Incremental builds only recompile what changed - 10-100x faster!**

### How Incremental Builds Work

Bazel tracks:
- ✅ File content (MD5 hash)
- ✅ Compiler flags
- ✅ Dependency graph
- ✅ Build commands

**Bazel recompiles ONLY if:**
- Source file content changed
- Header file included by source changed
- Compiler flags changed
- Dependencies changed

### Common Incremental Build Scenarios

#### Scenario 1: Edit One .cpp File

```bash
# Edit a single file
vim src/rsz/src/Resizer.cc

# Rebuild (INCREMENTAL - fast!)
bazelisk build //src/rsz/...

# What happens:
# ✅ Recompiles: Resizer.cc → Resizer.o (1 file)
# ✅ Re-links: rsz_lib
# ✅ Skips: All other 31,964 files (cached!)
# ⏱️ Time: 10-30 seconds
```

#### Scenario 2: Edit One .h File (Local Header)

```bash
# Edit a header used by a few files
vim src/rsz/src/RepairDesign.hh

# Rebuild (INCREMENTAL)
bazelisk build //src/rsz/...

# What happens:
# ✅ Recompiles: All .cpp files that include RepairDesign.hh
# ✅ Re-links: Affected libraries
# ✅ Skips: Files that don't use this header
# ⏱️ Time: 1-3 minutes (depends on usage)
```

#### Scenario 3: Edit Core Header (Wide Impact)

```bash
# Edit a widely-used header
vim src/sta/include/sta/Sta.hh

# Rebuild (INCREMENTAL but broader)
bazelisk build //src/rsz/...

# What happens:
# ✅ Recompiles: All files that transitively include Sta.hh
# ✅ Could be 100-500 files
# ✅ Still skips: Files not affected
# ⏱️ Time: 5-15 minutes
```

#### Scenario 4: No Changes

```bash
# Run build without any changes
bazelisk build //src/rsz/...

# What happens:
# ✅ Cache hit on all files
# ✅ No recompilation
# ⏱️ Time: Instant (< 1 second)
```

### Incremental Build Performance

| What Changed | Files Recompiled | Build Time | Example |
|--------------|------------------|------------|---------|
| Nothing | 0 | < 1 sec | Cache hit |
| 1 .cpp file | 1 | 10-30 sec | Edit Resizer.cc |
| Local .h file | 5-20 | 1-3 min | Edit private header |
| Public .h file | 20-100 | 3-8 min | Edit module interface |
| Core .h file | 100-500 | 5-15 min | Edit sta/Sta.hh |
| All files | 31,965 | 1.5-2 hours | Clean rebuild |

### CMake Incremental Builds

```bash
# After editing code
cd /Users/sherlock/OpenROAD/build

# Rebuild (incremental)
make rsz_lib -j$(nproc)

# Time: Similar to Bazel but slightly slower
# - 1 file: 20-60 seconds
# - Header: 2-10 minutes
```

---

## Selective Compilation

Build only the modules you need.

### Build Individual Modules

```bash
# Build only Resizer
bazelisk build //src/rsz:rsz_lib

# Build only Clock Tree Synthesis
bazelisk build //src/cts:cts_lib

# Build only STA integration
bazelisk build //src/dbSta:dbSta_lib

# Build multiple specific modules
bazelisk build //src/rsz:rsz_lib //src/cts:cts_lib
```

### Build Module + Dependencies

```bash
# Build rsz and everything it depends on
bazelisk build //src/rsz/...

# This includes:
# - src/rsz (resizer)
# - src/dbSta (STA integration)
# - src/sta (OpenSTA)
# - src/odb (database)
# - All tests
```

### Build Libraries Only (Skip Tests)

```bash
# Just the library, no tests or tools
bazelisk build //src/rsz:rsz_lib

# Faster than:
bazelisk build //src/rsz/...  # includes tests
```

### Query What Will Be Built

```bash
# See all targets that will be built
bazelisk query //src/rsz/...

# See dependencies
bazelisk query "deps(//src/rsz:rsz_lib)"

# See reverse dependencies (what depends on this)
bazelisk query "rdeps(//..., //src/rsz:rsz_lib)"
```

---

## Adding GUI Incrementally

**GUI can be added AFTER timing modules without full rebuild!**

### Why GUI is Incremental

GUI is a separate module with minimal overlap:

```
Timing modules (ALREADY COMPILED):
  sta ✅ → odb ✅ → dbSta ✅ → rsz ✅ → cts ✅

GUI (NEW - incremental compile):
  Qt5 🆕 → gui 🆕 → openroad binary 🆕
           ↓
        odb ✅ (reuses existing cache!)
```

### Step-by-Step: Add GUI Later

```bash
# Step 1: Initial build WITHOUT GUI (done)
bazelisk build //src/rsz/... //src/cts/...
# Time: ~2 hours (first time)
# Result: All timing modules compiled ✅

# Step 2: Later, add GUI (incremental!)
bazelisk build --//:platform=gui //:openroad
# Time: ~5-10 minutes (NOT 2 hours!)
# Compiles: Only GUI + Qt5 (~500-1000 files)
# Reuses: All timing modules from cache ✅

# Step 3: Run with GUI
./bazel-bin/openroad -gui
```

### What Gets Compiled When Adding GUI

| Component | Status | Compile Time | Size |
|-----------|--------|--------------|------|
| src/sta | ✅ Cached | 0 sec | Reused |
| src/odb | ✅ Cached | 0 sec | Reused |
| src/dbSta | ✅ Cached | 0 sec | Reused |
| src/rsz | ✅ Cached | 0 sec | Reused |
| src/cts | ✅ Cached | 0 sec | Reused |
| Qt5 libraries | 🆕 New | ~3 min | ~50 files |
| src/gui | 🆕 New | ~2 min | ~100 files |
| openroad binary | 🆕 Re-link | ~30 sec | Link step |
| **Total** | | **~5-10 min** | ~150 files |

### CMake: Adding GUI

```bash
# Edit src/CMakeLists.txt
# Uncomment: add_subdirectory(gui)

# Reconfigure and build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make gui -j$(nproc)
make openroad -j$(nproc)

# Note: CMake may rebuild more than Bazel
```

---

## Build Performance Comparison

### First Build (Clean Compile)

| Method | Target | Time | Files Compiled |
|--------|--------|------|----------------|
| Bazel | All modules + tests | 1.5-2 hours | 31,965 |
| Bazel | Libraries only | 30-40 min | ~10,000 |
| CMake | All modules | 1.5-2 hours | ~30,000 |
| CMake | Specific targets | 40-60 min | ~12,000 |

### Incremental Build (After 1 File Change)

| Method | What Changed | Time |
|--------|--------------|------|
| Bazel | 1 .cpp file | 10-30 sec |
| Bazel | 1 .h file (local) | 1-3 min |
| Bazel | 1 .h file (core) | 5-10 min |
| CMake | 1 .cpp file | 20-60 sec |
| CMake | 1 .h file (local) | 2-5 min |
| CMake | 1 .h file (core) | 5-15 min |

### Adding GUI (Incremental)

| Method | Time | Notes |
|--------|------|-------|
| Bazel | 5-10 min | Smart caching |
| CMake | 10-20 min | May rebuild more |

---

## Development Workflow

### Typical Edit-Compile-Test Cycle

```bash
# 1. Make changes
vim src/rsz/src/Resizer.cc

# 2. Build (incremental - fast!)
bazelisk build //src/rsz/...
# Time: 10-30 seconds ✅

# 3. Test specific module
bazelisk test //src/rsz/test:repair_timing_test

# 4. Run openroad
./bazel-bin/openroad

# 5. Format code before commit
clang-format -i src/rsz/src/Resizer.cc
# NEVER format src/sta/* or *.i files!

# 6. Commit with DCO signoff
git add src/rsz/src/Resizer.cc
git commit -s -m "rsz: Improve timing optimization algorithm"
```

### Working on Multiple Modules

```bash
# Edit files in multiple modules
vim src/rsz/src/Resizer.cc
vim src/cts/src/TritonCTS.cpp

# Build both (incremental)
bazelisk build //src/rsz/... //src/cts/...
# Only changed files are recompiled ✅

# Test both
bazelisk test //src/rsz/test/... //src/cts/test/...
```

### Parallel Development

```bash
# Build with more jobs (if you have CPU cores)
bazelisk build --jobs=16 //src/rsz/...

# Default uses all cores automatically
bazelisk build //src/rsz/...  # Uses --jobs=auto
```

---

## Troubleshooting

### Clean Rebuild (Nuclear Option)

```bash
# Bazel: Remove all build artifacts
bazelisk clean

# Full clean (removes external dependencies too)
bazelisk clean --expunge

# Then rebuild
bazelisk build //src/rsz/...
```

### Check What Will Be Rebuilt

```bash
# Dry run - see what Bazel will do
bazelisk build --explain=explain.txt //src/rsz/...

# View explanation
cat explain.txt
```

### Rebuild Specific Target

```bash
# Force rebuild even if cached
bazelisk build --nocache_test_results //src/rsz/test:test_name

# Rebuild with verbose output
bazelisk build --verbose_failures //src/rsz/...
```

### Build Fails After Git Pull

```bash
# Update submodules (REQUIRED after pull!)
git submodule update --init --recursive

# Clean and rebuild
bazelisk clean
bazelisk build //src/rsz/...
```

### Disk Space Issues

```bash
# Check Bazel cache size
du -sh ~/.cache/bazel

# Clean old artifacts
bazelisk clean

# Limit cache size (edit ~/.bazelrc)
build --disk_cache=/tmp/bazel-cache
```

### Slow Incremental Builds

```bash
# Check if files are actually cached
bazelisk info

# Check server status
bazelisk info server_pid

# Restart Bazel server
bazelisk shutdown
```

---

## Build Output Locations

### Bazel

```bash
# Compiled libraries
bazel-bin/src/dbSta/libdbSta.a
bazel-bin/src/rsz/librsz.a
bazel-bin/src/cts/libcts.a

# Executables
bazel-bin/openroad

# Build artifacts
bazel-out/darwin_arm64-opt/bin/

# Symbolic links
bazel-bin -> bazel-out/darwin_arm64-opt/bin
```

### CMake

```bash
# Compiled libraries
build/src/dbSta/libdbSta_lib.a
build/src/rsz/librsz_lib.a
build/src/cts/libcts_lib.a

# Executables
build/bin/openroad

# Build log
build/openroad_build.log
```

---

## Advanced Options

### Build with Different Optimization Levels

```bash
# Debug build (slower, easier debugging)
bazelisk build -c dbg //src/rsz/...

# Optimized build (default, faster runtime)
bazelisk build -c opt //src/rsz/...

# Fast build (faster compile, slower runtime)
bazelisk build -c fastbuild //src/rsz/...
```

### Build for Different Platforms

```bash
# Build for current platform
bazelisk build //src/rsz/...

# Cross-compile (if configured)
bazelisk build --platforms=@platforms//os:linux //src/rsz/...
```

### Profile Build Performance

```bash
# Generate build profile
bazelisk build --profile=build_profile.json //src/rsz/...

# Analyze with Bazel profiler
bazelisk analyze-profile build_profile.json
```

---

## Quick Reference

### Common Commands

```bash
# First build (all timing modules)
bazelisk build //src/dbSta/... //src/rsz/... //src/cts/... //src/est/... //src/rcx/...

# Incremental build (after code change)
bazelisk build //src/rsz/...

# Build just libraries (skip tests)
bazelisk build //src/rsz:rsz_lib

# Add GUI incrementally
bazelisk build --//:platform=gui //:openroad

# Clean rebuild
bazelisk clean && bazelisk build //src/rsz/...

# Install system-wide
bazelisk run //:install
```

### Build Times Summary

| Build Type | First Time | Incremental | When |
|------------|------------|-------------|------|
| Full (all tests) | 1.5-2 hours | 30 sec | Once |
| Libraries only | 30-40 min | 30 sec | Alternative |
| Single file edit | N/A | 10-30 sec | Common |
| Header edit | N/A | 1-5 min | Occasional |
| Add GUI | 5-10 min | 30 sec | When needed |
| Clean rebuild | 30-40 min | N/A | Rarely |

---

## Summary

✅ **Full compilation:** 1.5-2 hours (one-time)
✅ **Incremental compilation:** 10-30 seconds (every code change)
✅ **Adding GUI:** 5-10 minutes (incremental)
✅ **Bazel advantage:** Faster incremental builds, better caching
✅ **Best practice:** Use Bazel for development, incremental builds for iteration

**The key insight:** The 2-hour first build is one-time pain. All future builds are fast!

---

## Related Documentation

- **Quick Start Guide:** `/Users/sherlock/OpenROAD/QUICK_START.md`
- **Build Status:** `/Users/sherlock/OpenROAD/BUILD_STATUS.md`
- **Detailed Build Guide:** `/Users/sherlock/OpenROAD/docs/user/BuildTimingModules.md`
- **Resizer Commands:** `/Users/sherlock/OpenROAD/src/rsz/README.md`
- **OpenROAD Documentation:** https://openroad.readthedocs.io
