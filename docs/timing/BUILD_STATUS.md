# OpenROAD STA and Timing Modules - Build Status

## Build Method: Bazel

**Date:** 2026-08-24
**Status:** In Progress (88% complete)

## Successfully Initialized

✅ Git submodules (OpenSTA, ABC, etc.)
✅ Build tools (Bazelisk 1.29.0, Bazel 9.1.1)
✅ Compiler (Homebrew Clang 22.1.8)
✅ All dependencies installed

## Modules Being Compiled

| Module | Purpose | Status |
|--------|---------|--------|
| **src/sta** | OpenSTA - Core timing analysis engine | ✅ Compiled |
| **src/odb** | OpenDB - Design database | ✅ Compiled |
| **src/dbSta** | STA-database integration layer | ✅ Compiled |
| **src/rsz** | Resizer - Timing fixing engine | ✅ Compiled |
| **src/cts** | Clock Tree Synthesis (TritonCTS) | ✅ Compiled |
| **src/est** | Parasitics estimation | ✅ Compiled |
| **src/rcx** | Parasitic extraction (OpenRCX) | ✅ Compiled |
| **src/stt** | Steiner tree construction | ✅ Compiled |
| src/drt | Detailed routing | 🔄 Compiling |

## Build Command Used

```bash
/opt/homebrew/bin/bazelisk build \
  //src/dbSta/... \
  //src/rsz/... \
  //src/cts/... \
  //src/est/... \
  //src/rcx/...
```

## Build Progress

- **Total actions:** 31,965
- **Completed:** ~28,200 (88%)
- **Remaining:** ~3,700 (12%)

## Build Time

- **First build:** ~20-30 minutes (compiling from scratch)
- **Incremental builds:** ~1-3 minutes (Bazel caches compiled files)

## Next Steps After Build

Once complete, the compiled libraries will be in:
```
bazel-bin/src/dbSta/
bazel-bin/src/rsz/
bazel-bin/src/cts/
bazel-bin/src/est/
bazel-bin/src/rcx/
```

## Faster Builds Next Time

```bash
# Incremental build (10x faster)
bazelisk build //src/rsz/...

# Build only specific target
bazelisk build //src/rsz:rsz_lib

# Increase parallelism
bazelisk build --jobs=16 //src/rsz/...

# Clean and rebuild
bazelisk clean
bazelisk build //src/rsz/...
```

## Documentation

See `/Users/sherlock/OpenROAD/docs/user/BuildTimingModules.md` for complete CMake/Bazel build guide.
