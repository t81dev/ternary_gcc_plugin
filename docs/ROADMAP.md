# Ternary GCC Plugin Roadmap

This roadmap focuses on getting a usable, testable ternary toolchain surfaced
through a GCC plugin, with a stable ABI and runtime on binary substrates.

## Phase 0: Baseline and Scope Lock

- Define supported GCC versions and host platforms.
- Freeze the ternary ABI surface (type sizes, packing, calling convention).
- Decide on ternary XOR semantics and rounding rules for conversions.
- Confirm the software-first contract (helpers as reference semantics).

Deliverables:
- ABI section in `SPECIFICATION.md` finalized.
- Version matrix in `README.md`.

## Phase 1: Runtime Library (libternary)

- Implement `__ternary_*` functions referenced by the plugin.
- Provide a portable reference implementation in C/C++.
- Define headers for builtins and public API.
- Add container helpers for t6/t12/t24 and a plan for larger widths.

Deliverables:
- `lib/` or `runtime/` with source and headers.
- ABI tests that verify cross-language calls (C and C++).

## Phase 2: Front-End Surface

- Add user-facing types/macros and builtins in `include/ternary_plugin.h`.
- Define how ternary literals and casts are written in C/C++.
- Validate type promotion and conversion rules.
- Lock down exact condition type handling in helper signatures.

Deliverables:
- Updated header APIs and example usage in `README.md`.
- Compile-time tests for type promotion rules.

## Phase 3: Lowering and GIMPLE Coverage

- Expand lowering to cover all defined intrinsics and ternary ops.
- Ensure safe handling for unsupported types and conditions.
- Add diagnostics for unsupported or ambiguous cases.
- Track Phase 3/4 regression coverage (lowering + diagnostics) with a dedicated script/log so each milestone emits a clear artifact.

Deliverables:
- Plugin coverage test suite in `tests/`.
- GIMPLE-based regression tests (golden output or dg tests).
- `tests/run_phase34_coverage.sh` + `tests/PHASE34_COVERAGE.md` document the targeted entries and log file for upcoming checkpoints.

## Phase 4: Optimization and Codegen Validation

- Evaluate placement of the pass and interactions with other passes.
- Add optional transforms for constant folding and select simplification.
- Verify no regressions in non-ternary code paths.

Deliverables:
- Benchmarks and sanity checks for generated code.
- Performance report for key kernels (add, mul, select, cmp).

## Phase 5: Packaging and Tooling

- Build scripts for plugin + runtime (Makefile or CMake).
- Provide installation and usage steps for end users.
- Add CI workflow for build and tests.

Deliverables:
- `make install` or equivalent workflow.
- CI config covering at least one GCC version.

## Phase 6: Developer Ergonomics

- Add a `make test` target that selects GCC on macOS (Homebrew gcc-14/15) and
  uses `gcc` on Linux by default.
- Add a prominent macOS note that Apple Clang does not support GCC plugins.
- Optional: add plugin `--version`/`--selftest` and trace/dump flags.

Deliverables:
- `make test` runs the test suite with the correct compiler.
- README macOS note near install/usage instructions.

## Phase 7: Compatibility and ABI Matrix

- Document supported platform/compiler versions and current status.
- Add ABI validation checks in CI (symbol presence or hash).
- Ensure the documented helper ABI (`SPECIFICATION.md`/README) stays aligned with the headers
  (`include/ternary_runtime.h`, `include/ternary_helpers.h`), ideally via a CI regression triggered by
  `tools/check_helper_docs.py` so README/SPEC restructure stays aligned with the header order.

Deliverables:
- Compatibility matrix in `README.md`.
- ABI validation step in CI.
- Regression/check that the helper ABI list matches the headers/docs.

## Phase 8: Optional Hardware Backend

- Define an ISA encoding that preserves the helper ABI semantics.
- Add a codegen path that targets the hardware mnemonics.
- Provide a fallback mechanism to the helper runtime when unavailable.

Deliverables:
- Hardware backend design note and prototype.
- Compatibility tests against the helper ABI.

## Phase 9: Extensions (Optional)

- Vector/packed ternary operations.
- Ternary-aware optimizations (strength reduction, peepholes).
- LLVM or Clang front-end experiment for parity comparisons.

Deliverables:
- Prototype branch with targeted experiments and benchmarks.

## Success Criteria

- Stable ABI for ternary types and intrinsics.
- Runtime and plugin pass tests pass on supported GCC versions.
- Clear documentation and examples for end users.
