# OpenROAD Timing Modules - Quick Start Guide

## What You Built Tonight

✅ **Core STA and Timing Modules:**
- `src/sta/` - OpenSTA (timing analysis engine)
- `src/odb/` - OpenDB (design database)
- `src/dbSta/` - STA-database integration
- `src/rsz/` - Resizer (timing fixing: repair_timing, repair_design)
- `src/cts/` - Clock Tree Synthesis
- `src/est/` - Parasitics estimation
- `src/rcx/` - Parasitic extraction
- `src/stt/` - Steiner tree construction

## Build Output Locations

```bash
# Compiled libraries are in:
bazel-bin/src/dbSta/
bazel-bin/src/rsz/
bazel-bin/src/cts/
bazel-bin/src/est/
bazel-bin/src/rcx/

# Build artifacts:
bazel-out/darwin_arm64-opt/bin/
```

## Incremental Builds (Fast Rebuilds)

### After Editing Code

```bash
# Edit a file
vim src/rsz/src/Resizer.cc

# Rebuild (only ~30 seconds!)
/opt/homebrew/bin/bazelisk build //src/rsz/...

# Bazel only recompiles changed files ✅
```

### Common Scenarios

| What Changed | Rebuild Time | What Recompiles |
|--------------|--------------|-----------------|
| 1 .cpp file | 10-30 sec | Just that file + re-link |
| 1 .h file (local) | 1-3 min | Files that include it |
| Core header (sta/Sta.hh) | 5-10 min | Many files (transitively) |
| No changes | Instant | Nothing (cache hit) |

## Adding GUI Later (Incremental)

```bash
# Build GUI incrementally (won't recompile timing modules!)
/opt/homebrew/bin/bazelisk build --//:platform=gui //:openroad

# Time: ~5-10 minutes (not 30+ min)
# Reuses: All timing module caches ✅
# Compiles: Only GUI + Qt5 dependencies 🆕
```

## Faster Future Builds

### Build Only Libraries (Skip Tests)

```bash
# Current build included all tests (~32k files)
# Next time, build just libraries:

/opt/homebrew/bin/bazelisk build \
  //src/dbSta:dbSta_lib \
  //src/rsz:rsz_lib \
  //src/cts:cts_lib \
  //src/est:est_lib \
  //src/rcx:rcx_lib

# Time: ~10 minutes (60% faster than current build)
```

### Build Specific Module

```bash
# Just rebuild resizer
/opt/homebrew/bin/bazelisk build //src/rsz:rsz_lib

# Just rebuild CTS
/opt/homebrew/bin/bazelisk build //src/cts:cts_lib
```

## Using the Timing Tools

### Option 1: Build OpenROAD Binary

```bash
# Build the openroad executable
/opt/homebrew/bin/bazelisk build //:openroad

# Run it
./bazel-bin/openroad

# Or with GUI
/opt/homebrew/bin/bazelisk build --//:platform=gui //:openroad
./bazel-bin/openroad -gui
```

### Option 2: Install OpenROAD

```bash
# Install to default location
/opt/homebrew/bin/bazelisk run //:install

# Install to custom location
/opt/homebrew/bin/bazelisk run //:install -- /opt/openroad

# Then use
/usr/local/bin/openroad
```

## Key Timing Commands (After Build)

```tcl
# In openroad Tcl shell:

# Read design
read_lef sky130.lef
read_def design.def
read_liberty sky130.lib

# Estimate parasitics
set_wire_rc -layer metal2
estimate_parasitics -placement

# Timing analysis
report_checks
report_tns
report_wns

# Timing fixing
repair_design         # Fix max slew/cap/fanout
repair_timing -setup  # Fix setup violations
repair_timing -hold   # Fix hold violations

# Clock tree
clock_tree_synthesis

# Parasitic extraction (after routing)
extract_parasitics
report_checks
```

## Build Troubleshooting

### Clean Rebuild

```bash
# Remove all build artifacts
/opt/homebrew/bin/bazelisk clean

# Full rebuild
/opt/homebrew/bin/bazelisk build //src/rsz/...
```

### Check Build Status

```bash
# See what Bazel will build
/opt/homebrew/bin/bazelisk query //src/rsz/...

# Explain why something was rebuilt
/opt/homebrew/bin/bazelisk build //src/rsz/... --explain=explain.txt
```

### Increase Parallelism

```bash
# Use more CPU cores (if available)
/opt/homebrew/bin/bazelisk build --jobs=16 //src/rsz/...

# Default is --jobs=auto (uses all cores)
```

## Documentation

- **Full build guide:** `/Users/sherlock/OpenROAD/docs/user/BuildTimingModules.md`
- **Build status:** `/Users/sherlock/OpenROAD/BUILD_STATUS.md`
- **Resizer commands:** `/Users/sherlock/OpenROAD/src/rsz/README.md`
- **OpenROAD docs:** https://openroad.readthedocs.io

## Development Workflow

```bash
# 1. Make changes
vim src/rsz/src/Resizer.cc

# 2. Build (fast incremental)
bazelisk build //src/rsz/...

# 3. Test
bazelisk test //src/rsz/test:test_name

# 4. Run
./bazel-bin/openroad

# 5. Format code (before commit)
clang-format -i src/rsz/src/Resizer.cc

# 6. Commit (with DCO signoff)
git add src/rsz/src/Resizer.cc
git commit -s -m "rsz: Fix timing optimization bug"
```

## Next Steps

1. ✅ Wait for current build to finish (~93% complete)
2. ✅ Verify build success
3. ✅ Test basic timing commands
4. 📚 Read resizer documentation: `src/rsz/README.md`
5. 🚀 Start using timing analysis and optimization!

## Build Time Summary

| Build Type | Time | When |
|------------|------|------|
| **First build** (now) | ~1.5-2 hours | One time only |
| **Incremental** (1 file) | ~30 seconds | Every code change |
| **Incremental** (header) | ~1-5 minutes | Occasional |
| **Add GUI** | ~5-10 minutes | When needed |
| **Clean rebuild** | ~30 minutes | Rarely needed |
| **Library-only build** | ~10 minutes | Alternative to full build |

Remember: The 2-hour first build is **one-time pain for long-term gain**. All future builds will be fast!
