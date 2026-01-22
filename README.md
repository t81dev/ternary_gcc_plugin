# Ternary GCC Plugin

This repository ships a GCC plugin, helper ABI, and runtime reference that together define a
research-grade balanced-ternary toolchain slice: the plugin rewrites ternary-typed expressions into
calls to a normative helper ABI, the helper list documents the symbols any backend/ISA/runtime must
implement, and the runtime/skeleton code provides a correctness reference (including t32/t64/t128
paths and TMUX/TNET/TXOR helpers). The work is suitable for backend exploration and ISA prototyping
rather than being a drop-in replacement for standard integer arithmetic.

## Intended Audience
- Compiler/IR developers evaluating ternary-typed operations and helper lowering strategies.
- ISA designers prototyping balanced-ternary encodings, helper contracts, and SIMD semantics.
- Systems researchers benchmarking alternative arithmetic models for AI/ML or DSLs that accept ternary inputs.
- Contributors who can help harden the ABI, add backend integrations, or document the ISA surface.

## What This Is
1. **GCC frontend pass** — Scans GIMPLE conditionals and optional builtins, and rewrites ternary constructs
   into helper calls that expose ternary semantics explicitly.
2. **Normative helper ABI** — Lists all `__ternary_*` symbols (selects, arithmetic, logic, conversions, TMUX/TNET),
   with versions for t32/t64/t128/vector widths. Any runtime or hardware backend targeting this plugin must
   implement these helpers with the semantics described in `SPECIFICATION.md`.
3. **Reference runtime + skeleton** — Provides a correctness oracle (packed 2-bit trit encoding, helper
   implementations, neural-routing demo tests) so ISA-backed runtimes can validate behavior before custom
   hardware is available.

## Execution Model
- **Packed ternary representation** — Each trit occupies two bits (00 = -1, 01 = 0, 10 = +1), so t32/t64/t128
  values fit into 64/128/256-bit containers; the helper header and runtime share this encoding.
- **Condition semantics** — Ternary conditions are represented as `ternary_cond_t` (default `int64_t`),
  with helpers such as `__ternary_tbranch`, `__ternary_tsignjmp_*`, and TMUX/TNET helper functions exposing
  the true/false/unknown trit outcomes cleanly.
- **Helper ABI philosophy** — The helper list is the contract between frontend lowering and backend/hardware.
  If you add helpers, update `include/ternary_runtime.h`, the helper header, `SPECIFICATION.md`, and run
  `make check-helper-docs` (or `python3 tools/check_helper_docs.py`), so the documentation mirrors the header.

## Implementation Status

### Supported Types & Operations
- **Packed ternary types**: `t32_t`, `t64_t`, `t128_t` (when `_BitInt(256)` is available).
- **Vector ternary types**: `tv32_t`, `tv64_t` (vectors of 2 × t32_t and 2 × t64_t).
- **Arithmetic helpers**: add, sub, mul, div, mod, neg, mul-add, biases, selection helpers, etc.
- **Logic helpers**: not/inversion, TMUX selectors, TEQUIV/TXOR/TNET semantics.
- **Comparison helpers**: `__ternary_cmplt_*`, `__ternary_cmpeq_*`, `__ternary_cmpgt_*`, `__ternary_cmpneq_*`.
- **Conversion helpers**: tb2t/tt2b for t32/t64/t128 plus floating point conversions (`t2f`, `f2t`).
  These conversions are now documented explicitly for t128 so the helper list stays complete.
- **Extended helper ABI**: The runtime skeleton/executable demos exercise t32/t64/t128 TMUX/TNET/TXO R. The
  helper inventory is enumerated in `SPECIFICATION.md` and `include/ternary_runtime.h`.

## SIMD & Vector Trajectory

### SIMD: Implemented Today
- `tv32_t` vectors (2 × 32-trit) with element-wise arithmetic, logic, and comparison helpers.
- Builtins defined for these vectors already lower to helper calls, and the reference runtime implements
  them in terms of scalar t32 helpers so correctness is guaranteed.

### SIMD: Architectural Exploration
- **AVX-512 integration**: Work in progress for wider vector widths (8 × 32-trit or 4 × 64-trit).
- **Trit-level parallelism**: Future custom units could manipulate trits in parallel instead of scalar loops.
- **Hardware acceleration opportunities**: This ABI is designed so targeted hardware can expose the same helper
  names (`__ternary_add_t32`, `__ternary_tmux_t64`, etc.) even if the implementation uses custom encodings.

## Compatibility Matrix

| Platform      | Compiler | Version Range | Plugin Loading | Tests / Notes |
|---------------|----------|---------------|----------------|---------------|
| Linux x86_64  | GCC      | 9 - 15        | Yes            | Full support, CI-validated |
| Linux aarch64 | GCC      | 9 - 15        | Yes            | Expected to work (untested in CI) |
| macOS arm64   | GCC      | 14 - 15       | Yes            | Use Homebrew `gcc-14`/`gcc-15` |
| macOS (any)   | Apple Clang | any        | No             | No GCC plugin API compatibility |
| Windows       | MinGW/MSYS2 GCC | -      | Untested       | Potential future support |

ABI stability checks are planned in CI for supported GCC versions (see `ROADMAP.md`).

## Installation & Build

```bash
make
make install            # installs into /usr/local/lib and /usr/local/include/ternary by default
```

Adjust `DESTDIR`/prefix as needed. On macOS, use Homebrew GCC (e.g., `gcc-15`):

```bash
brew install gcc@15
make CXX=g++-15 CC=gcc-15
```

For CMake-driven builds (requires GMP + deps):

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

Load the plugin with:

```bash
gcc -fplugin=./ternary_plugin.so -c source.c
```

Optional arguments (many lower ternary builtins into helpers, emit stats, trace behavior, etc.):
- `-fplugin-arg-ternary_plugin-warn`, `-stats`, `-version`, `-selftest`, `-trace`, `-dump-gimple`
- `-fplugin-arg-ternary_plugin-lower` (selects), `-arith`, `-logic`, `-cmp`, `-shift`, `-conv`, `-mem`, `-vector`
- `-fplugin-arg-ternary_plugin-prefix=<name>` to change helper prefixes from `__ternary_*`.

Example with trace/dumps:

```bash
gcc -fplugin=./ternary_plugin.so -fplugin-arg-ternary_plugin-lower \
  -fplugin-arg-ternary_plugin-trace -fplugin-arg-ternary_plugin-dump-gimple \
  -Iinclude -c source.c
```

When lowering, helper calls such as `__ternary_select_i32`, `__ternary_tmux_t64`, and
`__ternary_tb2t_t128` are emitted. Link against the runtime (or your ISA-specific implementation)
to satisfy those symbols.

## Implementing Helpers & Runtime

Include `include/ternary_helpers.h` for example implementations that assume placeholder ISA ops
(e.g., `tsel`, `tadd`, `tmul`, `tnot`). These helpers share the same packed encoding and condition type.
`TERNARY_COND_T` defaults to `ternary_cond_t` (`int64_t`), and you can override it for custom runtimes.

`include/ternary_runtime.h` exposes the public ABI; `tools/check_helper_docs.py` verifies the documentation
lists exactly those symbols. Run `make check-helper-docs` whenever you extend the helper list so docs stay
aligned.

Reference runtime example:

```bash
cc -Iinclude -c runtime/ternary_runtime.c -o ternary_runtime.o
```

The `runtime_skeleton/` folder holds a standalone helper set plus a test harness (`runtime_skeleton/test_runtime_skeleton.c`)
and demo scripts (`runtime_skeleton/run_tnn_demo.sh`) that exercise t32/t64/t128 semantics.

## Testing

```bash
make test                   # runs helper/runtime tests plus literal/promotions coverage
```

`make test` compiles `tests/test_logic_helpers.c`, `tests/test_abi.c`, `tests/test_promotion.c`, and several other
files with the plugin enabled, then runs the resulting executables. On macOS, run `make test CXX=g++-15 CC=gcc-15`.

### Godbolt Recipe (Local Equivalent)

Godbolt ignores custom GCC plugins, so replicate the behavior locally:

```bash
cc -fplugin=./ternary_plugin.so \
  -fplugin-arg-ternary_plugin-types \
  -fplugin-arg-ternary_plugin-lower \
  -fplugin-arg-ternary_plugin-arith \
  -fplugin-arg-ternary_plugin-logic \
  -fplugin-arg-ternary_plugin-cmp \
  -fplugin-arg-ternary_plugin-shift \
  -fplugin-arg-ternary_plugin-conv \
  -Iinclude -c examples/ternary_basic.c -o build/ternary_basic.o
cc build/ternary_basic.o runtime/ternary_runtime.o -o build/ternary_basic
```

## SIMD Section

### SIMD: Implemented Today
- `tv32_t` vector type (2 × t32_t) with arithmetic, logic, and comparison helpers implemented in the runtime.
- Builtins such as `__builtin_ternary_add_tv32` lower to these helpers so the plugin already covers tv32 operations.

### SIMD: Strategic Exploration
- Plan to support wider AVX-512-style vectors (8 × 32-trit / 4 × 64-trit).
- Trit-level parallelism and custom ternary SIMD units are future directions that can reuse the same helper symbols.

## ISA Context

For helpers that require three-valued semantics (TMIN/TMAX, TIMPL/TLIMP, TMAJ, TQUANT, TNOT/TINV) and
control flow helpers (TBRANCH/TSIGNJMP), see `MASTER_ISA.md`. `ENCODING.md` documents the tryte field layout
for future hardware encodings.

### Helper Symbol Inventory (abridged)

```text
__ternary_tbias_t32 __ternary_tbias_t64 __ternary_tbias_t128 __ternary_tbranch __ternary_tequiv_t32
__ternary_tequiv_t64 __ternary_tequiv_t128 __ternary_tinv_t32 __ternary_tinv_t64 __ternary_tinv_t128
__ternary_tlimp_t32 __ternary_tlimp_t64 __ternary_tlimp_t128 __ternary_tlimp_tv32 __ternary_tmaj_t32
__ternary_tmaj_t64 __ternary_tmaj_t128 __ternary_tmaj_tv32 __ternary_tmax_t32 __ternary_tmax_t64
__ternary_tmax_t128 __ternary_tmax_tv32 __ternary_tmin_t32 __ternary_tmin_t64 __ternary_tmin_t128
__ternary_tmin_tv32 __ternary_tmuladd_t32 __ternary_tmuladd_t64 __ternary_tmuladd_t128 __ternary_tmux_t32
__ternary_tmux_t64 __ternary_tmux_t128 __ternary_tnet_t32 __ternary_tnet_t64 __ternary_tnet_t128
__ternary_tnormalize_t32 __ternary_tnormalize_t64 __ternary_tnormalize_t128 __ternary_tnot_t32
__ternary_tnot_t64 __ternary_tnot_t128 __ternary_tquant_t32 __ternary_tquant_t64 __ternary_tquant_t128
__ternary_tquant_tv32 __ternary_tround_t32 __ternary_tround_t64 __ternary_tround_t128 __ternary_tround_tv32
__ternary_tsignjmp_t32 __ternary_tsignjmp_t64 __ternary_txor_t32 __ternary_txor_t64 __ternary_txor_t128
__ternary_tb2t_t32 __ternary_tb2t_t64 __ternary_tb2t_t128 __ternary_tt2b_t32 __ternary_tt2b_t64 __ternary_tt2b_t128
```

This inventory is the contract: any runtime or hardware backend must implement these symbols with the semantics described in `SPECIFICATION.md`.

## Control Flow

Lowering focuses on making ternary control dependencies explicit in GIMPLE; actual binary branching encodings
are left to RTL/backend work. Control helpers (`__ternary_tbranch`, `__ternary_tsignjmp_*`) return a ternary-controllable
target so backends can emit `brt`/`brf` analogs without guessing on behalf of the plugin.

## Calling Conventions (Planned)

- **Arguments**: Prefer ternary registers when available; otherwise pass packed containers.
- **Returns**: Ternary results return via ternary registers or binary containers depending on hardware support.
- **Register Allocation**: Ternary-typed variables consume ternary registers, with fallbacks to binary registers.

GCC backend work remains to wire this into any given target.

## Known Limitations

- GCC plugin lowering currently depends on GCC with plugin support (Apple Clang cannot load it).
- No automatic ternary auto-vectorization or optimized hardware-specific code paths yet.
- Performance remains at reference-level; optimized ternary hardware would provide the expected benefits.
- Literal ternary syntax is not yet available—use helper macros or string helpers from the helper header.
- Limited cross-platform testing for some GCC versions (Windows/MinGW untested).

## Examples

See `examples/` for demo programs covering basic usage, calculator-style logic, and vector helpers.

## Documentation & Releases

- `SPECIFICATION.md` documents the helper ABI, ISA mnemonics, and execution model.
- `ROADMAP.md` sketches future work (CI, ABI stability, runtime verification).
- `ENCODING.md` details the tryte-level encoding for the ISA.
- Run `./make_release.sh` to build a tarball release with headers and plugin artifacts.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines, coding standards, and how to run tests locally.

