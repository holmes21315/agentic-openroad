# OpenROAD Timing Module Documentation

This folder contains comprehensive documentation for building, testing, and using OpenROAD's timing analysis capabilities.

---

## 📋 Quick Navigation

### Getting Started
1. **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - **START HERE** - Complete project overview and current status
2. **[QUICK_START.md](QUICK_START.md)** - Quick reference for common tasks

### Building
3. **[COMPILATION_GUIDE.md](COMPILATION_GUIDE.md)** - Complete guide for full and incremental compilation
4. **[BUILD_STATUS.md](BUILD_STATUS.md)** - Build progress and statistics

### Testing
5. **[TESTING_GUIDE.md](TESTING_GUIDE.md)** - Guide for running the 355 timing tests

### Usage
6. **[TIMING_SHELL.md](TIMING_SHELL.md)** - How to use OpenROAD as a timing-only shell (like PrimeTime)

### Timing-Only Build
7. **[TIMING_ONLY_BUILD.md](TIMING_ONLY_BUILD.md)** - Technical details of conditional compilation approach
8. **[TIMING_ONLY_COMPLETED.md](TIMING_ONLY_COMPLETED.md)** - Timing-only shell completion summary

---

## 📚 Document Descriptions

### PROJECT_STATUS.md
**Purpose:** Complete project status and overview
**Best For:** Understanding what was accomplished
**Contains:**
- Project objectives and completion status
- Build statistics and performance metrics
- Module classification (timing vs physical design)
- Binary size comparison (103 MB vs 42 MB)
- Usage examples and quick reference
- Development environment details

### QUICK_START.md
**Purpose:** Quick reference for developers
**Best For:** Day-to-day development tasks
**Contains:**
- Common build commands
- Incremental build workflows
- Adding GUI later
- Performance tips
- Development workflow

### COMPILATION_GUIDE.md
**Purpose:** Comprehensive compilation guide
**Best For:** First-time builders or troubleshooting
**Contains:**
- Full compilation (Bazel and CMake)
- Incremental compilation workflows
- Selective compilation
- Adding GUI incrementally
- Performance comparisons
- Troubleshooting common issues

### BUILD_STATUS.md
**Purpose:** Build progress tracking
**Best For:** Monitoring build status
**Contains:**
- Module compilation status
- Build progress (31,965 actions)
- Build time statistics
- Module list with status

### TESTING_GUIDE.md
**Purpose:** Complete testing documentation
**Best For:** Running and understanding tests
**Contains:**
- 355 test cases overview
- How to run tests
- Test categories (unit, integration, TCL)
- Common test scenarios
- Test output locations
- Troubleshooting test failures

### TIMING_SHELL.md
**Purpose:** Using OpenROAD as timing shell
**Best For:** Users wanting PrimeTime-like experience
**Contains:**
- How to use OpenROAD like PrimeTime pt_shell
- Available timing commands
- Interactive shell features
- Comparison: PrimeTime vs OpenROAD
- Example timing-only sessions

### TIMING_ONLY_BUILD.md
**Purpose:** Technical implementation details
**Best For:** Understanding conditional compilation
**Contains:**
- Conditional compilation approach
- Files modified with `#ifndef BUILD_TIMING_ONLY`
- Module classification
- Step-by-step implementation guide
- Build configuration details

### TIMING_ONLY_COMPLETED.md
**Purpose:** Completion summary for timing-only shell
**Best For:** Quick overview of timing-only build results
**Contains:**
- Binary size comparison
- Available vs excluded commands
- Build instructions
- Testing verification
- Use cases

---

## 🎯 Common Tasks

### I want to build the timing modules
→ Read [COMPILATION_GUIDE.md](COMPILATION_GUIDE.md)
→ Use command: `bazelisk build //src/dbSta/... //src/rsz/... //src/cts/... //src/est/... //src/rcx/...`

### I want to build the full OpenROAD shell
→ Read [COMPILATION_GUIDE.md](COMPILATION_GUIDE.md)
→ Use command: `bazelisk build //:openroad`

### I want to build the timing-only shell
→ Read [TIMING_ONLY_COMPLETED.md](TIMING_ONLY_COMPLETED.md)
→ Use command: `bazelisk build //:openroad-timing`

### I want to run tests
→ Read [TESTING_GUIDE.md](TESTING_GUIDE.md)
→ Use command: `bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all`

### I want to use OpenROAD like PrimeTime
→ Read [TIMING_SHELL.md](TIMING_SHELL.md)
→ Run: `./bazel-bin/openroad` or `./bazel-bin/openroad-timing`

### I want to understand what was accomplished
→ Read [PROJECT_STATUS.md](PROJECT_STATUS.md)

### I want a quick reference while developing
→ Read [QUICK_START.md](QUICK_START.md)

---

## 📊 Key Results Summary

### Binary Sizes
- **Full Shell:** 103 MB (`./bazel-bin/openroad`)
- **Timing-Only Shell:** 42 MB (`./bazel-bin/openroad-timing`)
- **Reduction:** 59% smaller

### Build Performance
- **First Build:** ~8.7 hours (with full caching)
- **Incremental Build:** 10-30 seconds (single file change)
- **Module Rebuild:** 1-3 minutes

### Test Coverage
- **Total Tests:** 355 timing module tests
- **Status:** All passing ✅

### Modules Compiled
- ✅ OpenSTA (timing engine)
- ✅ dbSta (STA-database integration)
- ✅ Resizer (timing fixing)
- ✅ TritonCTS (clock tree synthesis)
- ✅ EstimateParasitics (RC estimation)
- ✅ OpenRCX (RC extraction)
- ✅ SteinerTreeBuilder
- ✅ Utilities

---

## 🔧 Build Commands Quick Reference

```bash
# Build full shell
bazelisk build //:openroad

# Build timing-only shell
bazelisk build //:openroad-timing

# Build timing modules only
bazelisk build //src/dbSta/... //src/rsz/... //src/cts/... //src/est/... //src/rcx/...

# Run timing tests
bazelisk test //src/rsz/test/... //src/cts/test/... //src/est/test/...

# Run specific test
bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all

# Check binary sizes
ls -lh bazel-bin/openroad*
```

---

## 📁 File Organization

```
docs/timing/
├── README.md                      # This file - navigation guide
├── PROJECT_STATUS.md              # Complete project overview
├── QUICK_START.md                 # Quick reference
├── COMPILATION_GUIDE.md           # Compilation guide
├── BUILD_STATUS.md                # Build progress
├── TESTING_GUIDE.md               # Testing guide
├── TIMING_SHELL.md                # Timing shell usage
├── TIMING_ONLY_BUILD.md           # Conditional compilation details
└── TIMING_ONLY_COMPLETED.md       # Timing-only completion summary
```

---

## 🚀 Related Documentation

- **Main OpenROAD Docs:** `../../README.md`
- **Build Instructions:** `../../BUILD.md`
- **User Documentation:** `../user/`
- **CMake Build Guide:** `../user/BuildTimingModules.md`

---

**Last Updated:** 2026-08-24
**Status:** ✅ All objectives completed
