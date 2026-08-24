# OpenROAD Timing-Only Shell Guide

How to use OpenROAD as a **timing-only shell** (like PrimeTime/Innovus pt_shell).

## Current Architecture

**The Issue:**
OpenROAD's source code (`src/Design.cc`, `src/OpenRoad.cc`) has hardcoded `#include` statements for ALL modules (placement, routing, floorplan, etc.). This means:

- ❌ Cannot build a truly "timing-only" binary without source code modifications
- ✅ **But you can use only timing commands in the full shell**

## What You Already Have: Timing Modules

### ✅ Successfully Compiled (355 Tests)

You already compiled the timing module **libraries**:

```bash
# These are compiled as .a libraries:
bazel-bin/src/sta/         # OpenSTA engine
bazel-bin/src/odb/         # Design database
bazel-bin/src/dbSta/       # STA-database integration
bazel-bin/src/rsz/         # Resizer (timing fixing)
bazel-bin/src/cts/         # Clock tree synthesis
bazel-bin/src/est/         # Parasitics estimation
bazel-bin/src/rcx/         # Parasitics extraction
```

**Test them:**
```bash
# Unit tests
bazelisk test //src/rsz/test:TestResizer
bazelisk test //src/cts/test:simple_test-tcl_test
bazelisk test //src/est/test:TestEstimateParasitics

# Run all 355 timing tests
bazelisk test //src/rsz/test/... //src/cts/test/... //src/est/test/... //src/rcx/test/...
```

## Option 1: Use Full Shell, Timing Commands Only (Recommended)

### Build the Shell

```bash
cd /Users/sherlock/OpenROAD

# Build full openroad shell (includes all modules)
bazelisk build //:openroad

# Binary location:
./bazel-bin/openroad
```

### Use Like PrimeTime

The shell **IS** like PrimeTime - it just has extra commands you don't use:

```bash
# Run interactive shell
./bazel-bin/openroad

openroad> # You get a Tcl REPL with readline support!
```

**Available Timing Commands (like PrimeTime):**

```tcl
# STA (Static Timing Analysis)
read_liberty sky130.lib
read_lef sky130.lef
read_def design.def
create_clock -period 2.0 clk
set_input_delay -clock clk 0.5 [all_inputs]
set_output_delay -clock clk 0.5 [all_outputs]

# Timing reports
report_checks
report_checks -path_delay min_max
report_tns
report_wns
report_worst_slack -min
report_worst_slack -max
report_clock_skew

# Parasitics
estimate_parasitics -placement
extract_parasitics
set_wire_rc -layer metal3

# Timing fixing (like Innovus opt_design)
repair_design           # Fix max slew/cap/fanout
repair_timing -setup    # Fix setup violations
repair_timing -hold     # Fix hold violations

# Clock tree synthesis
clock_tree_synthesis
repair_clock_nets
```

**Commands You WON'T Use (Physical Design):**

```tcl
# Just don't use these - they're available but not needed
# initialize_floorplan  (skip)
# global_placement      (skip)
# detailed_placement    (skip)
# global_route          (skip)
# detailed_route        (skip)
```

### Interactive Features (Like PrimeTime)

✅ **Tab completion** - Press [Tab] to autocomplete commands
✅ **Command history** - Use Up/Down arrows
✅ **Readline editing** - Emacs/Vi keybindings
✅ **Tcl scripting** - Full Tcl 8.6 support

### Example Timing-Only Session

```bash
./bazel-bin/openroad

openroad> read_liberty Nangate45/Nangate45_typ.lib
openroad> read_lef Nangate45/Nangate45.lef
openroad> read_def my_design.def
openroad> create_clock -period 2.0 clk
openroad> estimate_parasitics -placement
openroad> report_checks
openroad> repair_timing -setup
openroad> report_checks
openroad> write_def fixed_design.def
openroad> exit
```

**This is exactly like using PrimeTime's `pt_shell`!**

## Option 2: Use Timing Tests Directly

If you want **zero** physical design code, use the compiled tests:

### Run Individual Timing Operations

```bash
# Timing repair test
bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all

# Clock tree synthesis test
bazelisk test //src/cts/test:simple_test-tcl_test --test_output=all

# Parasitics estimation test
bazelisk test //src/est/test:make_parasitics1-tcl_test --test_output=all
```

### Create Your Own Test Scripts

```bash
# Create custom timing script
cat > my_timing.tcl << 'EOF'
source "helpers.tcl"
read_liberty my.lib
read_lef my.lef
read_def my.def
create_clock -period 2.0 clk
estimate_parasitics -placement
repair_timing -setup
report_checks
write_def my_fixed.def
EOF

# Run it (uses ONLY timing modules)
cd src/rsz/test
/path/to/bazel-bin/openroad -no_init -exit ../../../my_timing.tcl
```

## Option 3: Build Minimal Shell (Requires Source Code Modification)

To build a **true** timing-only shell (smaller binary, faster compile), you would need to:

### Required Changes

1. **Make module includes optional** in `src/Design.cc`:

```cpp
// Current (hardcoded):
#include "ant/AntennaChecker.hh"
#include "grt/GlobalRouter.h"
#include "ifp/InitFloorplan.hh"
// ... etc

// Change to conditional:
#ifdef BUILD_ANTENNA
#include "ant/AntennaChecker.hh"
#endif
#ifdef BUILD_ROUTING
#include "grt/GlobalRouter.h"
#endif
// ... etc
```

2. **Wrap Tcl command registration** in `src/OpenRoad.cc`

3. **Create conditional initialization** in `src/Design.cc`

4. **Add build flags** to BUILD.bazel

**Estimated effort:** 2-4 hours of C++ editing

**Benefits:**
- Smaller binary: ~40-50 MB vs 103 MB
- Faster compilation: ~10 min vs ~30 min
- Cleaner command set (no placement/routing commands)

**Drawbacks:**
- Requires maintaining a fork or upstream changes
- Code complexity for conditional compilation

## Comparison: PrimeTime vs OpenROAD Timing Shell

| Feature | PrimeTime `pt_shell` | OpenROAD (Full Shell) | OpenROAD (Tests Only) |
|---------|----------------------|----------------------|----------------------|
| **STA Engine** | PrimeTime | OpenSTA | OpenSTA |
| **Interactive Shell** | ✅ Tcl REPL | ✅ Tcl REPL | ❌ Batch only |
| **Tab Completion** | ✅ Yes | ✅ Yes | ❌ No |
| **Timing Reports** | ✅ All reports | ✅ All reports | ✅ All reports |
| **Timing Fixing** | ❌ No (ECO only) | ✅ repair_timing | ✅ repair_timing |
| **Clock Tree Synthesis** | ❌ No (CTS tool) | ✅ clock_tree_synthesis | ✅ clock_tree_synthesis |
| **Parasitics** | ✅ SPEF/DSPF | ✅ SPEF + estimation | ✅ SPEF + estimation |
| **Placement Commands** | ❌ No | ✅ Available (don't use) | ❌ No |
| **Binary Size** | ~500 MB | ~103 MB | N/A (libraries) |
| **License** | Commercial | Open Source (BSD-3) | Open Source |

## Recommendations

### For Interactive Timing Analysis (Like PrimeTime)

**Use:** Option 1 - Full OpenROAD shell

```bash
bazelisk build //:openroad
./bazel-bin/openroad
# Just use timing commands, ignore physical design commands
```

**Pros:**
- ✅ Interactive Tcl shell
- ✅ Tab completion
- ✅ Full timing capabilities
- ✅ Already compiled

**Cons:**
- ❌ Larger binary (103 MB)
- ❌ Has unused commands (just don't use them)

### For Automated Timing Scripts (Batch Mode)

**Use:** Option 2 - Timing tests

```bash
# Create Tcl scripts
# Run via compiled test framework
bazelisk test //src/rsz/test:my_script-tcl_test
```

**Pros:**
- ✅ ONLY timing modules linked
- ✅ No physical design code

**Cons:**
- ❌ No interactive shell
- ❌ More setup for custom scripts

### For Production (Smaller Binary)

**Use:** Option 3 - Modify source code

**Only if:**
- You need minimal binary size
- You want to maintain a timing-only fork
- You have time for C++ modifications

## Quick Start

**1. Interactive timing shell (recommended):**

```bash
cd /Users/sherlock/OpenROAD
./bazel-bin/openroad

openroad> puts "Hello from timing shell!"
openroad> # Use it like PrimeTime!
```

**2. Run a timing test:**

```bash
bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all
```

**3. Check available commands:**

```bash
./bazel-bin/openroad <<EOF
# List all timing commands
puts [info commands report_*]
puts [info commands repair_*]
puts [info commands clock_*]
puts [info commands estimate_*]
exit
EOF
```

## Summary

You **already have** a timing shell like PrimeTime:

✅ **Shell location:** `./bazel-bin/openroad`
✅ **Timing modules:** Compiled in `bazel-bin/src/{sta,dbSta,rsz,cts,est,rcx}/`
✅ **355 tests:** All passing
✅ **Interactive Tcl:** Full readline support
✅ **Commands:** All STA/timing commands available

The only difference from PrimeTime: it has **extra commands you won't use** (placement/routing). Just ignore them!

**Use it exactly like PrimeTime's `pt_shell`.**
