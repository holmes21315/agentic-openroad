# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## Build System

OpenROAD uses **dual build systems** (Bazel and CMake). Both must be maintained.

### Bazel (Recommended for Development)

```bash
# Full build
bazelisk build //:openroad

# Build specific modules
bazelisk build //src/rsz/... //src/cts/... //src/est/...

# Timing-only shell (minimal binary, 59% smaller)
bazelisk build //:openroad-timing

# Run tests
bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all
bazelisk test //src/rsz/test/...  # All tests in module

# Incremental builds
bazelisk build //src/rsz/...  # Only changed files (10-30 seconds)

# Linting
bazelisk build --config=lint //src/utl/...
bazelisk build --config=lint -- //src/... //third-party/... -//src/sta/... -//third-party/abc/...
```

### CMake (Legacy)

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
ctest -R "rsz\.buffer_ports1\.tcl"  # Run single test
```

### Binary Outputs

- **Full shell**: `./bazel-bin/openroad` (103 MB)
- **Timing-only shell**: `./bazel-bin/openroad-timing` (42 MB)

---

## Architecture Overview

### RTL-to-GDSII Flow

OpenROAD implements a complete autonomous flow:
```
Verilog → Synthesis → Floorplan → Placement → CTS → Routing → Finishing → GDSII
```

### Module Organization

**Core Infrastructure:**
- `src/utl/` - Utilities (Logger, ServiceRegistry)
- `src/odb/` - Design database (OpenDB)
- `src/sta/` - **Upstream OpenSTA** (managed externally, see Critical Rules)

**Timing Modules** (always included):
- `src/dbSta/` - STA-database integration
- `src/rsz/` - Resizer (timing fixing: `repair_timing`, `repair_design`)
- `src/cts/` - Clock tree synthesis (TritonCTS)
- `src/est/` - Parasitics estimation
- `src/rcx/` - Parasitics extraction (OpenRCX)
- `src/stt/` - Steiner tree builder

**Physical Design Modules** (excluded in timing-only build):
- `src/ifp/` - Init floorplan
- `src/gpl/` - Global placement
- `src/dpl/` - Detailed placement
- `src/mpl/` - Macro placer
- `src/ppl/` - Pin placer
- `src/grt/` - Global routing
- `src/drt/` - Detailed routing (TritonRoute)
- `src/fin/` - Finale (routing finalization)
- `src/pdn/` - Power distribution network
- `src/ant/` - Antenna checker
- `src/tap/` - Tapcell insertion
- `src/cgt/` - Clock gating
- `src/dft/` - Design for test
- `src/rmp/` - Restructure
- `src/psm/` - Power simulation
- `src/par/` - Partitioning
- `src/pad/` - Pad placement (ICeWall)
- `src/ram/` - RAM generation
- `src/dst/` - Distributed computing
- `src/syn/` - Synthesis
- `src/gui/` - GUI interface
- `src/web/` - Web server

### Service Registry Pattern

Modules expose abstract interfaces through `utl::ServiceRegistry` using the `Service` suffix:
- `est::ParasiticsService`
- `drt::PinAccessService`

These live in `include/<module>/` as header-only targets, keeping the dependency graph acyclic. Distinct from `Abstract*` prefix used for graphics/visualization mock-swappable classes (e.g., `AbstractSteinerRenderer`).

### API Bindings

OpenROAD exposes APIs via:
- **Tcl**: SWIG-generated bindings (`src/OpenRoad.i`, `src/Design.i`)
- **Python**: Python bindings (when `BUILD_PYTHON=true`)

Main entry points:
- `src/Main.cc` - CLI entry point
- `src/OpenRoad.cc` - Core application orchestration
- `src/Design.cc` - Design database wrapper

### Conditional Compilation

Timing-only build uses `#ifndef BUILD_TIMING_ONLY` to exclude physical design modules while preserving code. Modified files:
- `src/Design.cc`, `include/ord/Design.h`
- `src/OpenRoad.cc`, `include/ord/OpenRoad.hh`
- `src/OpenRoad.i`, `src/Main.cc`
- `BUILD.bazel` (defines `TIMING_ONLY_DEPS`)

---

## Testing

### CRITICAL: Dual Registration Required

**Every new test MUST be registered in BOTH build systems:**

| Build System | File | Macro |
|--------------|------|-------|
| CMake | `src/<module>/test/CMakeLists.txt` | `or_integration_tests(...)` |
| Bazel | `src/<module>/test/BUILD` | `regression_test(...)` |

Forgetting Bazel registration is the most common mistake - test will pass locally but be missing from CI.

### Integration Tests (Tcl)

```bash
# Run from module test directory
cd src/rsz/test
./regression -R buffer_ports1    # Single test
./regression -R "repair.*hier"   # Pattern match
./regression -j4 -V              # All tests, parallel, verbose

# Or from build directory
cd build
ctest -R "rsz\.buffer_ports1\.tcl"
```

**Test Structure:**
```tcl
# [Brief description of the test]
set test_name <integration_test_name>

# Use make_result_file for outputs
set out_verilog [make_result_file "${test_name}.v"]
write_verilog $out_verilog

# Compare against golden files
diff_file ${test_name}.vok $out_verilog
```

**Golden Files:**
- `.ok` - Log golden file (full stdout)
- `.vok` - Verilog golden file

**Note:** `diff_file` reports only first difference - regenerate entire output when updating.

### Unit Tests (C++)

Located at `src/<module>/test/cpp/`. Registered in both CMake and Bazel `BUILD` files.

### Issue Reproduction Tests

Keep minimal - when floorplan/placement isn't part of the bug, encode the design directly in DEF using `read_def` instead of running full flow stages. This keeps tests simpler and more stable.

---

## Critical Rules

1. **OpenSTA is managed upstream** - `src/sta/` contains OpenSTA (`Sdc.cc`, `Power.cc`, `Sdc.tcl`, etc.). Prefer fixes in OpenROAD code (`src/dbSta/`, `src/rsz/`) when possible. Ask before modifying `src/sta/` files.

2. **Code formatting:**
   - Run `clang-format -i <files>` for C++ files before commit
   - **NEVER** format `src/sta/*` or `*.i` (SWIG) files

3. **DCO compliance** - Always use `git commit -s`. CI handles compliance checks.

4. **Git submodules** - After merge/pull, run `git submodule update --init --recursive`. When amending submodule commits, parent repo submodule reference must also be updated.

5. **Trace bugs upstream** - When a bug appears in output (e.g., Verilog), find the data creation point (e.g., `buffer_ports`, `remove_buffers`), not the serialization point (e.g., `VerilogWriter`).

---

## Common Build Pitfalls

### Debug vs Release Builds

Tests passing locally (Debug) but failing in CI (Release) indicate:
- **Uninitialized members** - Debug zero-initializes, Release has garbage
- **nullptr through custom hashers** - e.g., `PinIdHash::operator()(nullptr)` segfaults in Release only
- **Subnormal float values** - Uninitialized floats may be zero in Debug but subnormal in Release

### Shared Library RPATH

External tool binaries (e.g., kepler-formal) have RPATH baked in. After rebuilding a dependency, must `cmake --install` to RPATH location or use `LD_PRELOAD`/`patchelf`. Setting `LD_LIBRARY_PATH` alone may be insufficient.

---

## CI Workflow

### Retriggering CI

Never close/reopen a PR to retrigger - `gh pr reopen` requires admin permissions. Use:
```bash
git commit --allow-empty -s -m "retrigger CI"
```

### Clang-Tidy CI

Can fail for:
- Code issues
- **Unresolved review comments** - Must resolve on GitHub web UI (API can't resolve on public repos)

### `gh pr checks` Exit Code 8

Exit code 8 means checks are pending, not an error.

---

## Coding Standards

### Style Guidelines

- Follow Google C++ Style Guide
- Use modern C++20 style and features
- Apply `const` qualifiers whenever possible
- Use C++ casts, not C-style casts
- Use `{...}` braces even for single-line statements
- All comments in English
- Factor out functions >100 lines (except unit tests)
- Avoid defensive null checks if object cannot be null

### Message ID Duplicate Checker

`etc/find_messages.py` uses regex to match literal numeric IDs in `logger->xxx(MODULE, NNNN, ...)`. When sharing message IDs across modules, use named constants:
```cpp
constexpr int kMsgIdBufferInserted = 1234;
logger_->info(RSZ, kMsgIdBufferInserted, "...");
```

---

## AI Agent Skills

Located in `.agents/skills/` (symlinked from `.claude/skills`):

| Skill | Purpose | Invocation |
|-------|---------|------------|
| `triage-issue` | Reproduce bug and minimize test case | `/triage-issue <issue#>` |
| `fix-bug` | Trace root cause, implement fix, create tests | `/fix-bug <issue#>` |
| `add-test` | Add integration/unit tests with dual registration | `/add-test <module> [description]` |
| `review-pr` | Draft local PR review notes | `/review-pr <pr#>` |

---

## Documentation

- **Main docs**: https://openroad.readthedocs.io/en/latest/
- **Agent guides**: `docs/agents/` (build.md, testing.md, ci.md, coding.md)
- **Timing documentation**: `docs/timing/` (compilation, testing, timing-only build)
- **User guides**: `docs/user/`

### Documentation Organization

**IMPORTANT**: Do not create markdown files in the root directory. Place documentation in appropriate subdirectories:

- **Agent/AI guidance**: `docs/agents/`
- **Timing-related docs**: `docs/timing/`
- **User-facing guides**: `docs/user/`
- **Contributor guides**: `docs/contrib/`

The root directory should only contain essential project files (README.md, LICENSE, CONTRIBUTING.md, etc.).

### Building Documentation

```bash
cd docs
virtualenv .venv
source .venv/bin/activate
pip install -r requirements.txt

# HTML docs
make html

# Man pages
make preprocess && make all -j16
man openroad
```

---

## Quick Reference

```bash
# Build full shell
bazelisk build //:openroad

# Build timing-only shell
bazelisk build //:openroad-timing

# Run interactive shell
./bazel-bin/openroad

# Run test with output
bazelisk test //src/rsz/test:repair_setup1-tcl_test --test_output=all

# Lint code
bazelisk build --config=lint //src/rsz/...

# Format code
clang-format -i src/rsz/src/buffer.cc  # Never for src/sta/* or *.i files

# Check binary sizes
ls -lh bazel-bin/openroad*
```
