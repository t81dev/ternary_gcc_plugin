# Ternary GCC Plugin

This is a GCC plugin and helper ABI for a balanced-ternary ISA. It analyzes ternary
expressions in C/C++ and can lower them to helper calls that map to ternary ISA
instructions.

**Note:** This project is a passion/research effort exploring ternary logic, which has theoretical advantages in arithmetic and AI contexts. It is currently at an early stage, compiling and running toy examples, rather than a mature toolchain component.

The plugin supports:
- Packed ternary types: `t32_t`, `t64_t`, `t128_t` (32/64/128 trits; 2-bit packed encoding)
- Extended arithmetic operations: add, sub, mul, div, mod, neg
- Logic operations: not
- Comparison operations: cmp (returns -1, 0, +1)
- Conditional selection for all supported types

See [SPECIFICATION.md](SPECIFICATION.md) for detailed design and implementation information.
The Ternary Base ISA v0 definition (mnemonics, operand formats, and semantics) is
described in `SPECIFICATION.md`.

## Ternary vs. Binary Comparison

Balanced ternary offers symmetric range and simpler arithmetic compared to two's complement binary.

| Aspect | Balanced Ternary (e.g., 32 trits, 64 bits) | Two's Complement (e.g., 64-bit int) |
|--------|-------------------------------------------|-------------------------------------|
| Range  | -926510094425920 to 926510094425920       | -9223372036854775808 to 9223372036854775807 |
| Dynamic Range | 3^32 vs 2^64                        | Standard binary range              |
| Rounding | Symmetric (no bias toward positive/negative) | Biased toward negative             |
| Arithmetic Simplicity | Easier carry-free ops in hardware | Complex carry propagation          |

## Compatibility Matrix

| Platform      | Compiler | Version Range | Plugin Loading | Tests / Notes |
|---------------|----------|---------------|----------------|---------------|
| Linux x86_64  | GCC      | 9 - 15        | Yes            | Full support, CI-validated |
| Linux aarch64 | GCC      | 9 - 15        | Yes            | Expected to work (untested in CI) |
| macOS arm64   | GCC      | 14 - 15       | Yes            | Use Homebrew `gcc-14`/`gcc-15` |
| macOS (any)   | Apple Clang | any        | No             | No GCC plugin API compatibility |
| Windows       | MinGW/MSYS2 GCC | -      | Untested       | Potential future support |

ABI stability checks are planned in CI for supported GCC versions (see `ROADMAP.md`).

## Installation

To install the plugin and headers:

```bash
make install
```

This installs to `/usr/local/lib` and `/usr/local/include/ternary` by default. Adjust `DESTDIR` if needed.

**Note:** On macOS, Apple Clang does not support GCC plugins. Use Homebrew GCC (e.g., `gcc-14` or `gcc-15`) instead.

### macOS Users - Important!

Apple Clang (even when invoked as `gcc`) does not support GCC plugins. You may
see errors like "symbol not found in flat namespace '_g'". Use real GCC from
Homebrew instead:

```bash
brew install gcc@15
```

Then compile with:

```bash
gcc-15 -fplugin=./ternary_plugin.so yourfile.c -o out
# or after `make install`:
gcc-15 -fplugin=ternary_plugin yourfile.c -o out
```

For Homebrew (macOS):

```bash
# Placeholder: If a formula is created, e.g.
# brew install ternary-gcc-plugin
```

For Docker:

```bash
docker build -t ternary-gcc .
docker run -it ternary-gcc
```

Use the `ternary-gcc` wrapper script for `-fternary` flag convenience.

## Building the Plugin

To build the plugin, you need GCC with plugin support enabled.

```bash
make
```

On macOS, prefer Homebrew GCC and set the toolchain explicitly:

```bash
make CXX=g++-15 CC=gcc-15
```

Alternatively, with CMake (requires GMP and other deps):

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

Load the plugin when compiling:

```bash
gcc -fplugin=./ternary_plugin.so -c source.c
```

Optional arguments:

- `-fplugin-arg-ternary_plugin-warn` emits a warning for each ternary operator.
- `-fplugin-arg-ternary_plugin-stats` prints a summary at the end of compilation.
- `-fplugin-arg-ternary_plugin-version` prints the plugin version and GCC version.
- `-fplugin-arg-ternary_plugin-selftest` prints a brief self-test summary of enabled features.
- `-fplugin-arg-ternary_plugin-trace` logs lowering decisions for matching statements.
- `-fplugin-arg-ternary_plugin-dump-gimple` dumps matching GIMPLE statements before rewriting.
- `-fplugin-arg-ternary_plugin-lower` replaces ternary operators with calls to
  `__ternary_select_[i|u]<bits>` for integral result types (e.g., `__ternary_select_i32`)
  and `__ternary_select_f32` or `__ternary_select_f64` for floating-point types.
- `-fplugin-arg-ternary_plugin-arith` enables lowering of ternary arithmetic builtins like
  `__builtin_ternary_add`, `__builtin_ternary_mul`, `__builtin_ternary_sub`, `__builtin_ternary_div`, `__builtin_ternary_mod`, and `__builtin_ternary_neg`.
- `-fplugin-arg-ternary_plugin-logic` enables lowering of ternary logic builtins like
  `__builtin_ternary_not`, `__builtin_ternary_and`, `__builtin_ternary_or`, and `__builtin_ternary_xor`.
- `-fplugin-arg-ternary_plugin-cmp` enables lowering of ternary comparison builtins like
  `__builtin_ternary_cmp`.
- `-fplugin-arg-ternary_plugin-shift` enables lowering of ternary shift/rotate builtins like
  `__builtin_ternary_shl`, `__builtin_ternary_shr`, `__builtin_ternary_rol`, and `__builtin_ternary_ror`.
- `-fplugin-arg-ternary_plugin-conv` enables lowering of ternary conversion builtins like
  `__builtin_ternary_tb2t`, `__builtin_ternary_tt2b`, `__builtin_ternary_t2f`, and `__builtin_ternary_f2t`.
- `-fplugin-arg-ternary_plugin-types` enables builtin ternary integer types `t32_t`, `t64_t`,
  `t128_t` with packed 2-bit trit storage.
- `-fplugin-arg-ternary_plugin-prefix=<name>` sets the base helper prefix used by lowering
  (default: `__ternary`). For example, select helpers become `<prefix>_select_i32` and arithmetic
  helpers become `<prefix>_add`, `<prefix>_sub`, etc.

Example with trace/dumps enabled:

```bash
gcc -fplugin=./ternary_plugin.so -fplugin-arg-ternary_plugin-lower \
  -fplugin-arg-ternary_plugin-trace -fplugin-arg-ternary_plugin-dump-gimple \
  -Iinclude -c source.c
```

## Implementing the Helpers

When using `-lower`, the plugin emits calls to external functions like `__ternary_select_i32`.
You need to provide implementations for these functions, tailored to your ternary ISA.

Include `include/ternary_helpers.h` in your code, which provides example implementations using
placeholder ternary ISA instructions (for example, `tsel`, `tadd`, `tmul`, `tnot`). Adjust the
assembly to match your ISA. `TERNARY_COND_T` defines the condition type used by select helpers
and is fixed to `ternary_cond_t` (default: `int64_t`). The plugin lowers conditions to
`ternary_cond_t` before helper calls.

For testing without the plugin, the header provides C implementations of all ternary operations.
Packed ternary types use a 2-bit encoding per trit (00 = -1, 01 = 0, 10 = +1).
Define `TERNARY_USE_BUILTIN_TYPES` before including the header when compiling with
`-fplugin-arg-ternary_plugin-types` to avoid typedef conflicts.
Helper/runtime support is currently provided for t32/t64 only; t128 requires
custom runtime implementations. The reference runtime uses 64-bit logical
conversion helpers, so t64 correctness is limited to values that fit in int64.

For example:

```c
#include "ternary_helpers.h"

int main() {
    int x = 1 ? 2 : 3;  // With -lower, becomes __ternary_select_i32(1, 2, 3)
    return x;
}
```

Compile with:

```bash
gcc -fplugin=./ternary_plugin.so -fplugin-arg-ternary_plugin-lower -Iinclude -c main.c
```

## Runtime Library Skeleton

For a minimal out-of-line runtime, use the reference implementation in `runtime/ternary_runtime.c`
with the public header `include/ternary_runtime.h`. It implements the same packed 2-bit-trit
semantics as the helpers (t32/t64) and is intended as a starting point for a real ISA-backed
library. Packed helpers for t128 are not provided in this reference runtime.

Example build:

```bash
cc -Iinclude -c runtime/ternary_runtime.c -o ternary_runtime.o
```

The `runtime_skeleton/` directory includes a tiny standalone helper set plus a sanity-check
test you can build and run:

```bash
mkdir -p build
cc -Iruntime_skeleton/include runtime_skeleton/src/ternary_runtime_skeleton.c \
  runtime_skeleton/test_runtime_skeleton.c -o build/runtime_skeleton_test
./build/runtime_skeleton_test
```

## Godbolt Recipe (Local-Equivalent)

Godbolt does not allow custom GCC plugins, so use the local equivalent command line to reproduce
what you would run in Compiler Explorer with a plugin-enabled GCC build:

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
```

Link with the reference runtime object to satisfy helper symbols:

```bash
cc build/ternary_basic.o runtime/ternary_runtime.o -o build/ternary_basic
```

## Testing

Use the provided test file:

```bash
gcc -fplugin=./ternary_plugin.so -fplugin-arg-ternary_plugin-lower \
  -fplugin-arg-ternary_plugin-arith -fplugin-arg-ternary_plugin-logic \
  -fplugin-arg-ternary_plugin-cmp -fplugin-arg-ternary_plugin-shift \
  -fplugin-arg-ternary_plugin-conv -Iinclude -c test_ternary.c
```

On macOS, run:

```bash
make test CXX=g++-15 CC=gcc-15
```

## Description

This plugin analyzes ternary conditional expressions in the code and can optionally
lower ternary operations to helper calls suitable for targeting a balanced-ternary ISA.

## Balanced Ternary Literals

Use balanced-ternary strings to construct packed values:

```c
t32_t a = T32_BT_STR("1 0 -1 1");
t64_t b = T64_BT_STR("1,0,0,-1");
```

The parser consumes trits from left to right (most significant to least significant).

## Known Limitations

- No auto-vectorization for ternary operations yet.
- Performance is reference implementation; optimized ternary hardware would excel.
- Limited interaction testing with GCC optimizations (-O3 may affect lowering).
- No native literal syntax; use the balanced-ternary string macros.
- Plugin requires GCC with plugin support (not available in all distributions).

## Examples

See `examples/` for demo programs showcasing basic usage, calculator, and vectors.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Releases

Run `./make_release.sh` to build a tarball.
