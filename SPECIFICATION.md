# Ternary ISA + GCC Plugin Specification

## Overview

This specification defines a balanced-ternary execution model, a minimal ISA surface
that matches binary capabilities, and a GCC plugin/toolchain contract for lowering
ternary operations from C/C++. The goal is parity with binary systems: any computation
expressible in binary should be expressible and performant in ternary.

## Design Goals

- Balanced ternary with trits in {-1, 0, +1}.
- Binary-addressed memory for compatibility.
- IEEE binary floating point preserved (ternary integer ops only for now).
- Deterministic mapping to C/C++ semantics.
- Stable ABI for helper functions and builtins.

## Numeric Model

### Trit Encoding

Balanced ternary digit values are {-1, 0, +1}. The storage encoding is an ISA detail
and may use two-bit codes (00, 01, 10) or another compact encoding. The logical value
of a trit is independent of encoding.

### Value

For trits t0..t(n-1), value = sum(ti * 3^i).

### Range

For n trits, representable range is [-(3^n-1)/2, +(3^n-1)/2].

### Signedness

Balanced ternary is inherently signed. If an "unsigned" view is required, it is a
reinterpretation of the same trit pattern with a different comparison rule.

## Type System

### Ternary Integer Types

The ISA defines ternary integer types by trit width:

- t6, t12, t24, t48, t96, t192 (suggested), with platform-specific availability.

The toolchain maps these to C integer types via a storage ABI that packs trits into
binary containers. A recommended mapping using 2-bit trit packing is:

- t6  -> stored in 16-bit container (12 bits used)
- t12 -> stored in 32-bit container (24 bits used)
- t24 -> stored in 64-bit container (48 bits used)
- t48 -> stored in 128-bit container (96 bits used)

Packing is logical; the physical encoding is an ABI detail. Compilers must preserve
bit patterns across loads/stores and calls.

### Boolean and Condition Types

Condition values are trits. Comparisons produce {-1, 0, +1}. Branches and selects
treat 0 as false and non-zero as true. C/C++ condition values are lowered to the
exact integral type used in source unless otherwise specified.

### Floating Point

IEEE-754 binary float and double are preserved. Ternary select supports floats, but
ternary arithmetic is limited to integer types for now.

## Instruction Semantics

### Arithmetic

- tadd, tsub, tmul, tdiv, tmod
- Unary: tneg, tnot (tritwise inversion: -1 <-> +1, 0 -> 0)

### Logic

Ternary logic is defined with min/max semantics:

- tand(a,b) = min(a,b)
- tor(a,b)  = max(a,b)
- txor(a,b) = a + b - 2 * min(a,b) - 2 * max(a,b) (ISA may choose alternative ternary XOR)

Implementations may choose a different ternary XOR definition, but it must be
documented and stable.

### Comparisons

All compares yield a trit:

- tcmp(a,b) returns -1 if a < b, 0 if a == b, +1 if a > b

Derived condition codes can be implemented by comparing the result against 0.

### Select

tsel(cond, t, f) returns t if cond != 0, otherwise f.

### Shifts and Rotates

Shifts operate on trit positions, not bits. Left shift introduces 0 trits. Right
shift is arithmetic in balanced ternary (preserves sign in the ternary sense).

### Conversions

- tb2t: binary integer container to ternary logical value
- tt2b: ternary logical value to binary container
- t2f / f2t: ternary integer <-> IEEE float conversions (f2t rounds toward zero)

## Ternary Base ISA v0

This section defines a software-visible ISA contract. Encodings are intentionally
left open, but operand formats and semantics are fully specified. The ISA operates
on packed 2-bit trits, using the encoding 00 = -1, 01 = 0, 10 = +1, 11 = reserved.

### Operand Types

- Tn: ternary integer register (packed trits, 2 bits per trit).
- Rn: binary integer register (for addresses and binary containers).
- Fn: IEEE floating-point register (f32/f64).
- Imm: immediate value (binary-encoded).

Ternary registers have a fixed trit width per instruction variant (e.g., t6, t12,
t24, t48, t96, t192). The width is encoded in the mnemonic suffix.

### Register and Instruction Formats

The base ISA assumes a three-operand format for ternary ALU operations:

- tOP.tN Td, Ta, Tb   (ternary arithmetic/logic on N trits)
- tOP.tN Td, Ta       (ternary unary operations on N trits)
- tsel.tN Td, Tc, Ta, Tb (select with ternary condition Tc)
- tcmp.tN Rd, Ta, Tb  (binary result -1/0/+1 in Rd)

Where:
- Td/Ta/Tb/Tc are ternary registers of width tN.
- Rd is a binary integer register holding {-1,0,+1} in two's complement.

### Instruction Semantics

#### Select

- tsel.tN Td, Tc, Ta, Tb
  - If Tc != 0, Td := Ta, else Td := Tb.
  - Tc is a ternary value; non-zero is true.

#### Arithmetic (ternary integers)

- tadd.tN Td, Ta, Tb: Td := Ta + Tb
- tsub.tN Td, Ta, Tb: Td := Ta - Tb
- tmul.tN Td, Ta, Tb: Td := Ta * Tb
- tdiv.tN Td, Ta, Tb: Td := (Tb == 0) ? 0 : Ta / Tb
- tmod.tN Td, Ta, Tb: Td := (Tb == 0) ? 0 : Ta % Tb
- tneg.tN Td, Ta:     Td := -Ta

All arithmetic is defined over the integer value of the packed ternary operand,
with results reduced to the tN range using balanced ternary representation.

#### Logic (ternary)

- tnot.tN Td, Ta: Td := tritwise inversion (-1 <-> +1, 0 -> 0)
- tand.tN Td, Ta, Tb: Td := min(Ta, Tb) per-trit
- tor.tN  Td, Ta, Tb: Td := max(Ta, Tb) per-trit
- txor.tN Td, Ta, Tb: Td := ternary XOR (ISA-defined, must be stable)

#### Compare

- tcmp.tN Rd, Ta, Tb
  - Rd := -1 if Ta < Tb, 0 if Ta == Tb, +1 if Ta > Tb.

#### Shifts and Rotates

- tshl.tN Td, Ta, Imm: shift left by Imm trits, fill with 0
- tshr.tN Td, Ta, Imm: arithmetic right shift by Imm trits
- trol.tN Td, Ta, Imm: rotate left by Imm trits
- tror.tN Td, Ta, Imm: rotate right by Imm trits

Shift counts are masked to the trit width.

#### Conversions

- tb2t.tN Td, Ra: convert binary integer in Ra to tN (balanced ternary encoding)
- tt2b.tN Rd, Ta: convert tN to binary integer in Rd
- t2f.tN Fd, Ta: convert tN to IEEE float (f32/f64)
- f2t.tN Td, Fa: convert IEEE float to tN (rounded toward zero)

### Memory and Control

Memory addresses are binary. Load/store operate on packed ternary containers:

- tld.tN Td, [Ra + Imm]: load packed ternary value into Td
- tst.tN Ta, [Ra + Imm]: store packed ternary value from Ta

Control flow uses ternary conditions:

- brt Rc, label: branch if Rc != 0
- brf Rc, label: branch if Rc == 0

### Software Mapping

In the absence of hardware support, the ISA mnemonics are mapped to helper calls
in C. The helper ABI mirrors the instruction set and operates on packed trit
containers using the 2-bit encoding.

## Memory Model

- Addresses are binary.
- Ternary data is loaded/stored via binary containers with defined packing.
- Alignment follows the container size.

## Control Flow

Branches use ternary conditions:

- brt cond, label: branch if cond != 0
- brf cond, label: branch if cond == 0

Comparisons produce ternary results and are used directly by branches.

## ABI and Calling Convention

### Registers

Registers are ternary-width, but stored in binary containers per ABI.

### Argument Passing

- Ternary integers passed in ternary registers if available, otherwise in
  binary containers in integer registers/stack slots.
- Floating point uses existing FP registers (IEEE).

### Return Values

Return ternary integers in designated ternary register or integer container register.

### Varargs

Varargs are passed using binary containers. The callee interprets the container based
on the promoted ternary type.

## Toolchain Contract

### Builtins

- __builtin_ternary_select(cond, t, f)
- __builtin_ternary_add(a, b)
- __builtin_ternary_sub(a, b)
- __builtin_ternary_mul(a, b)
- __builtin_ternary_div(a, b)
- __builtin_ternary_mod(a, b)
- __builtin_ternary_neg(a)
- __builtin_ternary_not(a)
- __builtin_ternary_and(a, b)
- __builtin_ternary_or(a, b)
- __builtin_ternary_xor(a, b)
- __builtin_ternary_cmp(a, b)
- __builtin_ternary_shl(a, b)
- __builtin_ternary_shr(a, b)
- __builtin_ternary_rol(a, b)
- __builtin_ternary_ror(a, b)
- __builtin_ternary_tb2t(a)
- __builtin_ternary_tt2b(a)
- __builtin_ternary_t2f(a)
- __builtin_ternary_f2t(a)

### Custom Types

When `-fplugin-arg-ternary_plugin-types` is enabled, the plugin registers builtin
types `t6_t`, `t12_t`, `t24_t`, `t48_t`, `t96_t`, and `t192_t` with packed 2-bit
trit storage (precision = trit_count * 2).

### Helper ABI

Helper functions implement ISA-visible operations in C:

- __ternary_select_[i|u]N(cond_t, true_val, false_val)
- __ternary_select_f32(cond_t, true_val, false_val)
- __ternary_select_f64(cond_t, true_val, false_val)
- __ternary_add(a, b)
- __ternary_sub(a, b)
- __ternary_mul(a, b)
- __ternary_div(a, b)
- __ternary_mod(a, b)
- __ternary_neg(a)
- __ternary_not(a)
- __ternary_and(a, b)
- __ternary_or(a, b)
- __ternary_xor(a, b)
- __ternary_cmp(a, b)
- __ternary_shl(a, b)
- __ternary_shr(a, b)
- __ternary_rol(a, b)
- __ternary_ror(a, b)
- __ternary_tb2t(a)
- __ternary_tt2b(a)
- __ternary_t2f32(a) / __ternary_t2f64(a)
- __ternary_f2t32(a) / __ternary_f2t64(a)

cond_t must match the exact condition type used at the call site. The helper header
should expose a macro to select cond_t (default: bool). The reference helper header
uses a packed 2-bit trit encoding (00 = -1, 01 = 0, 10 = +1).

### GCC Plugin Behavior

- GIMPLE pass scans COND_EXPR assignments.
- Optional warnings and stats.
- Lowering replaces ternary selects with helper calls.
- Builtin calls are lowered to helper calls when enabled.

## Testing and Validation

- Ternary select: int, unsigned, float, double, and custom ternary types (t6_t, t12_t, t24_t).
- Arithmetic builtins: add, sub, mul, div, mod, neg.
- Logic builtins: not.
- Comparison builtins: cmp (returns -1, 0, +1).
- Nested ternary expressions.
- Compare/branch semantics in ternary conditions.
- ABI round-trip: pass/return ternary values between functions.
- Helper function implementations: C fallbacks for testing without hardware ISA.

## Implementation Status

### Completed Features
- GCC plugin infrastructure with GIMPLE pass registration
- Extended arithmetic builtin lowering (add, sub, mul, div, mod, neg)
- Logic builtin lowering (not)
- Comparison builtin lowering (cmp)
- Helper function ABI with packed C implementations for testing
- Test suite covering all implemented operations
- Builtin ternary type registration (t6_t, t12_t, t24_t, t48_t, t96_t, t192_t)

### Known Limitations
- GCC 15 API compatibility may still require adjustments
- Placeholder ISA instructions (tsel, tadd, tmul, etc.) not implemented in hardware
- No ternary logic operations beyond 'not' (tand, tor, txor not implemented)
- No shift/rotate operations
- No conversion operations (tb2t, tt2b, t2f/f2t)

## Future Extensions

- Native ternary floating point formats.
- Vector/SIMD ternary operations.
- Full ternary logic instruction set (tand, tor, txor).
- Shift and rotate operations.
- Conversion operations between binary and ternary representations.
- Binary/ternary mixed-mode codegen and scheduling.
- GCC version compatibility fixes.

## Still Needs Implementation

1. Custom ternary types
   - Implemented: builtin types t6, t12, t24, t48, t96, t192 (packed 2-bit trits)
   - Implemented: storage ABI for 2-bit trit packing
   - Remaining: helper typedefs for t48/t96/t192 if needed for C-only testing
2. Extended arithmetic operations
   - Implemented: tsub, tdiv, tmod, tneg builtins and helpers
   - Remaining: hardware ISA backing for these mnemonics
3. Ternary logic operations
   - Implemented: tand, tor, txor builtins and helpers (min/max/xor semantics)
   - Remaining: hardware ISA backing for these mnemonics
4. Comparison operations
   - Implemented: tcmp builtins and helpers (-1/0/+1)
   - Remaining: hardware ISA backing for these mnemonics
5. Shifts and rotates
   - Implemented: trit-based shl/shr/rol/ror builtins and helpers
   - Remaining: hardware ISA backing for these mnemonics
6. Type conversions
   - Implemented: tb2t/tt2b and t2f/f2t builtins and helpers
   - Remaining: hardware ISA backing and precise rounding semantics
7. Exact condition type handling
   - Issue: Helpers use fixed TERNARY_COND_T (defaults to bool)
   - Required: Condition type must match exact type used at call site
   - Impact: ABI compatibility issues
8. Varargs support
   - Missing: Handling of ternary types in varargs
   - Impact: Functions with variable arguments won't work with ternary types
9. Full ISA integration
   - Missing: Complete mapping to all specified ISA instructions
   - Missing: Proper register allocation and calling conventions for ternary types

## Priority Order

- High: Hardware ISA support for ternary mnemonics
- High: Exact condition type handling in helpers
- Medium: Varargs support for ternary types
- Low: Helper typedefs for larger ternary widths (t48/t96/t192)
