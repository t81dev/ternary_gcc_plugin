# Ternary GCC Plugin Roadmap

This roadmap focuses on getting a usable, testable ternary toolchain surfaced
through a GCC plugin, with a stable ABI and runtime.

## Phase 0: Baseline and Scope Lock

- Define supported GCC versions and host platforms.
- Freeze the ternary ABI surface (type sizes, packing, calling convention).
- Decide on ternary XOR semantics and rounding rules for conversions.

Deliverables:
- ABI section in `SPECIFICATION.md` finalized.
- Version matrix in `README.md`.

## Phase 1: Runtime Library (libternary)

- Implement `__ternary_*` functions referenced by the plugin.
- Provide a portable reference implementation in C/C++.
- Define headers for builtins and public API.

Deliverables:
- `lib/` or `runtime/` with source and headers.
- ABI tests that verify cross-language calls (C and C++).

## Phase 2: Front-End Surface

- Add user-facing types/macros and builtins in `include/ternary_plugin.h`.
- Define how ternary literals and casts are written in C/C++.
- Validate type promotion and conversion rules.

Deliverables:
- Updated header APIs and example usage in `README.md`.
- Compile-time tests for type promotion rules.

## Phase 3: Lowering and GIMPLE Coverage

- Expand lowering to cover all defined intrinsics and ternary ops.
- Ensure safe handling for unsupported types and conditions.
- Add diagnostics for unsupported or ambiguous cases.

Deliverables:
- Plugin coverage test suite in `tests/`.
- GIMPLE-based regression tests (golden output or dg tests).

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

Deliverables:
- Compatibility matrix in `README.md`.
- ABI validation step in CI.

## Phase 8: Extensions (Optional)

- Vector/packed ternary operations.
- Ternary-aware optimizations (strength reduction, peepholes).
- LLVM or Clang front-end experiment for parity comparisons.

Deliverables:
- Prototype branch with targeted experiments and benchmarks.

## Success Criteria

- Stable ABI for ternary types and intrinsics.
- Runtime and plugin pass tests pass on supported GCC versions.
- Clear documentation and examples for end users.
