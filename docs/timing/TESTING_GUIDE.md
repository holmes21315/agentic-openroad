# OpenROAD Timing Modules - Testing Guide

Complete guide for running and understanding tests for STA and timing modules.

## Table of Contents

- [Test Overview](#test-overview)
- [Running Tests](#running-tests)
- [Test Categories](#test-categories)
- [Common Test Scenarios](#common-test-scenarios)
- [Test Output Options](#test-output-options)
- [Interpreting Results](#interpreting-results)
- [Troubleshooting](#troubleshooting)

---

## Test Overview

### Total Test Coverage

The timing modules have **355 comprehensive test cases**:

| Module | Test Count | Types | Coverage |
|--------|------------|-------|----------|
| **src/rsz** (Resizer) | 263 tests | C++ unit + Tcl integration | Timing fixing, buffering, optimization |
| **src/cts** (Clock Tree) | 61 tests | C++ unit + Tcl integration | CTS algorithms, clock repair |
| **src/est** (Parasitics Est.) | 13 tests | C++ unit + Tcl integration | RC estimation, wire models |
| **src/rcx** (Parasitics Ext.) | 18 tests | Tcl integration | Post-route extraction |
| **Total** | **355 tests** | | Full timing flow |

### Test Types

**1. C++ Unit Tests**
- Test internal algorithms and data structures
- Fast execution (1-3 seconds)
- Example: `TestResizer`, `TestEstimateParasitics`

**2. Tcl Integration Tests**
- Test complete flows with real designs
- Verify command-line interface (Tcl commands)
- Compare output against golden reference files (`.ok` files)
- Example: `repair_setup1-tcl_test`, `array-tcl_test`

---

## Running Tests

### Testing ONLY Timing Modules (Recommended)

**These tests use ONLY the compiled timing modules** (sta, dbSta, rsz, cts, est, rcx) **without floorplanning, placement, or routing:**

```bash
# Test resizer (timing fixing)
bazelisk test //src/rsz/test:TestResizer
bazelisk test //src/rsz/test:repair_setup1-tcl_test

# Test clock tree synthesis
bazelisk test //src/cts/test:simple_test-tcl_test

# Test parasitics estimation
bazelisk test //src/est/test:TestEstimateParasitics
bazelisk test //src/est/test:make_parasitics1-tcl_test

# Run multiple timing tests together
bazelisk test \
  //src/rsz/test:TestResizer \
  //src/rsz/test:repair_setup1-tcl_test \
  //src/cts/test:simple_test-tcl_test \
  //src/est/test:TestEstimateParasitics
```

**Example output from timing-only tests:**

**Resizer (repair_setup1):**
- Reads design with setup violation (-0.333 ns)
- Runs `repair_timing`
- Removes 4 buffers, resizes 1 gate
- **Result: slack improved to +0.013 ns (MET)** ✅

**CTS (simple_test):**
- Builds clock tree for 16 sinks
- Inserts 3 clock buffers (CLKBUF_X3)
- Creates balanced H-tree topology
- **Result: Clock tree depth = 2, max fanout = 8** ✅

**Parasitics (make_parasitics1):**
- Estimates RC parasitics for nets
- Calculates delays with wire models
- **Result: Timing analysis with parasitics complete** ✅

### Quick Start
cd /Users/sherlock/OpenROAD

# Run a single test
bazelisk test //src/rsz/test:repair_setup1-tcl_test

# Run all tests in a module
bazelisk test //src/rsz/test/...

# Run all timing module tests
bazelisk test //src/rsz/test/... //src/cts/test/... //src/est/test/... //src/rcx/test/...
```

### Basic Test Commands

#### Run Single Test

```bash
# Run specific test by name
bazelisk test //src/rsz/test:TestResizer

# Run with summary output
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=summary

# Run with full output
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all
```

#### Run Multiple Tests

```bash
# Run multiple specific tests
bazelisk test \
  //src/rsz/test:repair_hold1-tcl_test \
  //src/rsz/test:buffer_ports1-tcl_test \
  //src/cts/test:array-tcl_test

# Run all tests in a directory
bazelisk test //src/rsz/test/...
```

#### Run All Timing Tests

```bash
# Run all 355 timing module tests
bazelisk test \
  //src/rsz/test/... \
  //src/cts/test/... \
  //src/est/test/... \
  //src/rcx/test/...

# Expected time: ~30-60 minutes (first run with caching)
# Expected time: ~5-10 minutes (subsequent runs)
```

### Finding Available Tests

```bash
# List all rsz tests
bazelisk query "tests(//src/rsz/test/...)"

# List all cts tests
bazelisk query "tests(//src/cts/test/...)"

# List all est tests
bazelisk query "tests(//src/est/test/...)"

# List all rcx tests
bazelisk query "tests(//src/rcx/test/...)"

# Search for specific test pattern
bazelisk query "tests(//src/rsz/test/...)" | grep repair_timing
bazelisk query "tests(//src/rsz/test/...)" | grep repair_hold
bazelisk query "tests(//src/cts/test/...)" | grep clock
```

---

## Test Categories

### Resizer (src/rsz) - 263 Tests

#### Timing Optimization Tests

```bash
# Setup timing repair tests (fix max delay violations)
bazelisk test //src/rsz/test:repair_setup1-tcl_test
bazelisk test //src/rsz/test:repair_setup2-tcl_test

# Hold timing repair tests (fix min delay violations)
bazelisk test //src/rsz/test:repair_hold1-tcl_test
bazelisk test //src/rsz/test:repair_hold2-tcl_test

# Design rule repair tests (fix slew/cap/fanout)
bazelisk test //src/rsz/test:repair_design1-tcl_test
bazelisk test //src/rsz/test:repair_slew1-tcl_test
bazelisk test //src/rsz/test:repair_fanout1-tcl_test
```

#### Buffer Insertion Tests

```bash
# Port buffering tests
bazelisk test //src/rsz/test:buffer_ports1-tcl_test
bazelisk test //src/rsz/test:buffer_ports2-tcl_test

# Buffer removal tests
bazelisk test //src/rsz/test:TestBufRem1
bazelisk test //src/rsz/test:TestBufferRemoval2
```

#### Resizer Unit Tests

```bash
# Core C++ unit tests
bazelisk test //src/rsz/test:TestResizer
bazelisk test //src/rsz/test:TestResizerMt       # Multi-threaded
bazelisk test //src/rsz/test:TestInsertBuffer
bazelisk test //src/rsz/test:TestRepairHoldRollback
```

#### Run All Resizer Tests

```bash
# All 263 resizer tests
bazelisk test //src/rsz/test/...

# Time: ~15-25 minutes (first run)
# Time: ~3-5 minutes (cached)
```

### Clock Tree Synthesis (src/cts) - 61 Tests

#### Basic CTS Tests

```bash
# Simple clock tree
bazelisk test //src/cts/test:simple_test-tcl_test

# Array-based designs
bazelisk test //src/cts/test:array-tcl_test
bazelisk test //src/cts/test:array_ins_delay-tcl_test
bazelisk test //src/cts/test:array_max_wl-tcl_test

# Clock net repair
bazelisk test //src/cts/test:array_repair_clock_nets-tcl_test
```

#### Buffer Inference Tests

```bash
# Buffer selection and inference
bazelisk test //src/cts/test:check_buffer_inference1-tcl_test
bazelisk test //src/cts/test:check_buffer_inference2-tcl_test
bazelisk test //src/cts/test:check_buffers-tcl_test
```

#### Advanced CTS Tests

```bash
# CTS with blockages
bazelisk test //src/cts/test:check_buffers_blockages-tcl_test

# Multilevel clock trees
bazelisk test //src/cts/test:multilevel_test-tcl_test

# No max cap constraints
bazelisk test //src/cts/test:buffer_no_max_cap-tcl_test
```

#### Run All CTS Tests

```bash
# All 61 CTS tests
bazelisk test //src/cts/test/...

# Time: ~5-10 minutes (first run)
# Time: ~1-2 minutes (cached)
```

### Parasitics Estimation (src/est) - 13 Tests

#### Wire RC Tests

```bash
# Basic parasitics estimation
bazelisk test //src/est/test:make_parasitics1-tcl_test
bazelisk test //src/est/test:make_parasitics2-tcl_test

# Wire RC configuration
bazelisk test //src/est/test:set_wire_rc_tech-tcl_test
bazelisk test //src/est/test:set_wire_rc_selectors-tcl_test

# RDL (redistribution layer) tests
bazelisk test //src/est/test:set_wire_rc_rdl_mixed_tech-tcl_test
```

#### Unit Tests

```bash
# Core C++ unit test
bazelisk test //src/est/test:TestEstimateParasitics
```

#### Run All EST Tests

```bash
# All 13 parasitics estimation tests
bazelisk test //src/est/test/...

# Time: ~1-2 minutes
```

### Parasitics Extraction (src/rcx) - 18 Tests

```bash
# List available rcx tests
bazelisk query "tests(//src/rcx/test/...)"

# Run all rcx tests
bazelisk test //src/rcx/test/...

# Time: ~2-5 minutes
```

---

## Common Test Scenarios

### After Code Changes

```bash
# Edit resizer code
vim src/rsz/src/Resizer.cc

# Run affected tests
bazelisk test //src/rsz/test:TestResizer
bazelisk test //src/rsz/test:repair_setup1-tcl_test

# Time: ~5-10 seconds (incremental, only recompiles changed code)
```

### Before Committing Changes

```bash
# Run all tests for the modified module
bazelisk test //src/rsz/test/...

# Or run specific regression tests
bazelisk test \
  //src/rsz/test:TestResizer \
  //src/rsz/test:repair_setup1-tcl_test \
  //src/rsz/test:repair_hold1-tcl_test \
  //src/rsz/test:buffer_ports1-tcl_test
```

### Testing Entire Timing Flow

```bash
# End-to-end timing flow tests
bazelisk test \
  //src/est/test:make_parasitics1-tcl_test \   # Estimation
  //src/rsz/test:repair_setup1-tcl_test \      # Setup fixing
  //src/rsz/test:repair_hold1-tcl_test \       # Hold fixing
  //src/cts/test:array-tcl_test                # Clock tree
```

### Regression Testing

```bash
# Run comprehensive regression (all 355 tests)
bazelisk test \
  //src/rsz/test/... \
  //src/cts/test/... \
  //src/est/test/... \
  //src/rcx/test/...

# Run with parallel execution (faster)
bazelisk test --jobs=8 \
  //src/rsz/test/... \
  //src/cts/test/... \
  //src/est/test/... \
  //src/rcx/test/...
```

---

## Test Output Options

### Output Modes

```bash
# 1. SUMMARY (default) - Shows pass/fail only
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=summary

# Output:
# //src/rsz/test:repair_hold1-tcl_test    PASSED in 1.4s

# 2. ERRORS - Shows output only for failed tests
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=errors

# 3. ALL - Shows full test output (useful for debugging)
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all

# Output includes:
# - Test execution log
# - OpenROAD command output
# - Tcl script execution details
```

### Verbose Options

```bash
# Show test execution details
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all -s

# Show which tests are being run
bazelisk test //src/rsz/test/... --test_summary=detailed

# Show test logs location
bazelisk test //src/rsz/test:repair_hold1-tcl_test --verbose_test_summary
```

### Filtering Test Output

```bash
# Run tests and save output
bazelisk test //src/rsz/test/... --test_output=all > test_results.log 2>&1

# Run and show only failures
bazelisk test //src/rsz/test/... --test_output=errors
```

---

## Interpreting Results

### Successful Test Run

```
INFO: Found 4 test targets...
INFO: Elapsed time: 22.433s, Critical Path: 21.64s
INFO: Build completed successfully, 5 total actions

//src/cts/test:array-tcl_test                    PASSED in 20.5s
//src/est/test:make_parasitics1-tcl_test         PASSED in 1.5s
//src/rsz/test:buffer_ports1-tcl_test            PASSED in 1.4s
//src/rsz/test:repair_hold1-tcl_test             PASSED in 1.4s

Executed 4 out of 4 tests: 4 tests pass.
```

**Meaning:**
- ✅ All tests passed
- Total time: 22.4 seconds
- 4 tests executed, 0 failures

### Failed Test Run

```
//src/rsz/test:repair_hold1-tcl_test             FAILED in 1.4s

Executed 1 out of 1 test: 1 fails locally.
```

**What to do:**
1. Run with `--test_output=all` to see detailed error
2. Check test log location (shown in output)
3. Compare output with expected `.ok` file

```bash
# Get detailed failure info
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all

# Check test log
cat bazel-testlogs/src/rsz/test/repair_hold1-tcl_test/test.log
```

### Test Output Files

```bash
# Test logs location
bazel-testlogs/src/rsz/test/repair_hold1-tcl_test/
├── test.log          # Full test output
├── test.xml          # JUnit XML format
└── test.outputs/     # Test artifacts

# View test log
cat bazel-testlogs/src/rsz/test/repair_hold1-tcl_test/test.log

# View all recent test logs
find bazel-testlogs -name "test.log" -mmin -5
```

### Understanding Tcl Test Execution

Tcl integration tests work by:
1. Running OpenROAD with a `.tcl` script
2. Capturing the output
3. Comparing against a `.ok` golden reference file
4. PASS if output matches, FAIL if different

```bash
# Test files for repair_hold1
src/rsz/test/repair_hold1.tcl     # Test script
src/rsz/test/repair_hold1.ok      # Expected output
src/rsz/test/repair_hold1.def     # Input design (if needed)

# When test runs:
# 1. openroad -no_init -exit repair_hold1.tcl > output.txt
# 2. diff output.txt repair_hold1.ok
# 3. PASS if identical, FAIL if different
```

---

## Troubleshooting

### Test Failures After Code Changes

```bash
# Problem: Test fails after modifying code

# Step 1: Run test with full output
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all

# Step 2: Check what changed
cat bazel-testlogs/src/rsz/test/repair_hold1-tcl_test/test.log

# Step 3: If output is correct but different, update .ok file
# (Only if your code change intentionally changes output!)
cp <new_output> src/rsz/test/repair_hold1.ok
```

### Slow Test Execution

```bash
# Problem: Tests run slowly

# Solution 1: Use parallel execution
bazelisk test --jobs=8 //src/rsz/test/...

# Solution 2: Run only affected tests
bazelisk test //src/rsz/test:TestResizer  # Just unit test

# Solution 3: Clean stale cache
bazelisk clean
bazelisk test //src/rsz/test/...
```

### Test Not Found

```bash
# Problem: "no such target"

# Solution: List available tests
bazelisk query "tests(//src/rsz/test/...)" | grep repair

# Check exact test name
bazelisk query "tests(//src/rsz/test/...)" | grep -i hold
```

### Cache Issues

```bash
# Problem: Test passes locally but fails in CI, or vice versa

# Solution 1: Clear test cache
bazelisk clean --expunge_async

# Solution 2: Force re-run without cache
bazelisk test //src/rsz/test:repair_hold1-tcl_test --nocache_test_results

# Solution 3: Clean and rebuild
bazelisk clean
bazelisk test //src/rsz/test/...
```

### Debugging Failed Tests

```bash
# Step 1: Run test alone with full output
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all

# Step 2: Examine test script
cat src/rsz/test/repair_hold1.tcl

# Step 3: Run manually in OpenROAD
./bazel-bin/openroad
source src/rsz/test/repair_hold1.tcl

# Step 4: Compare with expected output
cat src/rsz/test/repair_hold1.ok
```

### Test Timeout

```bash
# Problem: Test times out

# Solution: Increase timeout
bazelisk test //src/cts/test:array-tcl_test --test_timeout=300

# Or disable timeout
bazelisk test //src/cts/test:array-tcl_test --test_timeout=-1
```

---

## Test Performance

### Typical Test Execution Times

| Test Category | Single Test | All Tests in Module | Notes |
|---------------|-------------|---------------------|-------|
| **C++ Unit Tests** | 1-3 sec | 5-10 sec | Fast, algorithmic tests |
| **Tcl Integration (simple)** | 1-5 sec | 10-30 min | Depends on design size |
| **Tcl Integration (complex)** | 10-30 sec | 30-60 min | Large designs, full flows |

### First Run vs Cached

```bash
# First run (cold cache)
bazelisk test //src/rsz/test/...
# Time: ~20-30 minutes (compiles tests + runs)

# Second run (warm cache)
bazelisk test //src/rsz/test/...
# Time: ~3-5 minutes (cache hit, just runs)

# After code change (incremental)
bazelisk test //src/rsz/test:TestResizer
# Time: ~5-10 seconds (recompiles 1 file, runs test)
```

### Parallel Execution

```bash
# Default (uses all CPU cores)
bazelisk test //src/rsz/test/...

# Explicit parallelism
bazelisk test --jobs=8 //src/rsz/test/...

# Maximum parallelism
bazelisk test --jobs=auto //src/rsz/test/...
```

---

## Test Development Workflow

### Adding New Tests

See `docs/agents/testing.md` for detailed instructions on adding tests.

Quick overview:

```bash
# 1. Create test script
vim src/rsz/test/my_new_test.tcl

# 2. Run manually to generate output
./bazel-bin/openroad -no_init -exit src/rsz/test/my_new_test.tcl > output.ok

# 3. Add to BUILD file
vim src/rsz/test/BUILD
# Add tcl_test() entry

# 4. Run new test
bazelisk test //src/rsz/test:my_new_test-tcl_test
```

### Test-Driven Development

```bash
# 1. Write failing test first
vim src/rsz/test/new_feature.tcl
bazelisk test //src/rsz/test:new_feature-tcl_test  # FAILS

# 2. Implement feature
vim src/rsz/src/Resizer.cc

# 3. Rebuild and test (incremental - fast!)
bazelisk test //src/rsz/test:new_feature-tcl_test  # PASSES

# 4. Run regression
bazelisk test //src/rsz/test/...
```

---

## Quick Reference

### Most Common Commands

```bash
# Run single test
bazelisk test //src/rsz/test:TestResizer

# Run all tests in module
bazelisk test //src/rsz/test/...

# Run all timing tests (355 tests)
bazelisk test //src/rsz/test/... //src/cts/test/... //src/est/test/... //src/rcx/test/...

# Run with full output
bazelisk test //src/rsz/test:repair_hold1-tcl_test --test_output=all

# List available tests
bazelisk query "tests(//src/rsz/test/...)"

# Run after code change (fast!)
bazelisk test //src/rsz/test:TestResizer
```

### Test Counts

- **Total timing tests:** 355
- **Resizer (rsz):** 263 tests
- **Clock Tree (cts):** 61 tests
- **Parasitics Est (est):** 13 tests
- **Parasitics Ext (rcx):** 18 tests

---

## Summary

✅ **355 comprehensive test cases** covering all timing modules
✅ **Fast incremental testing** - 5-10 seconds after code changes
✅ **Both unit and integration tests** - Algorithm + full flow coverage
✅ **Bazel caching** - Tests only re-run when needed
✅ **Parallel execution** - Utilize all CPU cores

The timing modules have excellent test coverage ensuring correctness and preventing regressions!

---

## Related Documentation

- **Quick Start:** `/Users/sherlock/OpenROAD/QUICK_START.md`
- **Compilation Guide:** `/Users/sherlock/OpenROAD/COMPILATION_GUIDE.md`
- **Build Status:** `/Users/sherlock/OpenROAD/BUILD_STATUS.md`
- **Detailed Testing Guide:** `/Users/sherlock/OpenROAD/docs/agents/testing.md`
- **Resizer README:** `/Users/sherlock/OpenROAD/src/rsz/README.md`
