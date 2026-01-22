# Ternary ISA + GCC Plugin Specification

## Overview

This specification defines a balanced-ternary execution model, a minimal ISA surface
implemented as a conservative extension over binary substrates, and a GCC
plugin/toolchain contract for lowering ternary operations from C/C++. The goal is
parity with binary systems: any computation expressible in binary should be
expressible and performant in ternary, without requiring ternary hardware.

## Design Goals

- Balanced ternary with trits in {-1, 0, +1}.
- Binary-addressed memory for compatibility.
- Binary containers as the canonical storage and ABI transport.
- IEEE binary floating point preserved (ternary integer ops only for now).
- Deterministic mapping to C/C++ semantics.
- Stable ABI for helper functions and builtins.

## Conservative Extension Model

The ternary ISA is a virtual contract layered over a binary machine model. All
ternary values are represented in binary containers with a defined packing, and
all operations must have a correct software implementation. Hardware ISA support
is optional and may accelerate the same contract without changing semantics.

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

- t32, t64, t128 (initial focus), with platform-specific availability.

The toolchain maps these to C integer types via a storage ABI that packs trits into
binary containers. A recommended mapping using 2-bit trit packing is:

- t32  -> stored in 64-bit container
- t64  -> stored in 128-bit container
- t128 -> stored in 256-bit container

Packing is logical; the physical encoding is an ABI detail. Compilers must preserve
bit patterns across loads/stores and calls.

### Source Literal Helpers

The current toolchain does not define a native ternary literal token. Instead, it
provides helpers that parse balanced-ternary strings left-to-right (most significant
trit first). Example:

```c
t32_t a = T32_BT_STR("1 0 -1 1");
t64_t b = T64_BT_STR("1,0,0,-1");
```

The runtime parser accepts `-1`, `0`, `1`, or `+1` tokens separated by whitespace or
commas. Invalid strings return zero.

### Boolean and Condition Types

Condition values are trits. Comparisons produce {-1, 0, +1}. Branches and selects
treat 0 as false and non-zero as true. C/C++ condition values are lowered to the
ternary condition ABI type (ternary_cond_t). The plugin lowers condition
expressions to ternary_cond_t before helper calls; packed ternary conditions are
decoded with tt2b first.

### Floating Point

IEEE-754 binary float and double are preserved. Ternary select supports floats, but
ternary arithmetic is limited to integer types for now.

### Type Promotion and Conversion Rules

Ternary types follow C-like promotion rules for operations:

- Usual arithmetic conversions: smaller types promote to larger (t32 -> t64 -> t128).
- Mixed ternary/binary: ternary promotes to binary container size for compatibility.
- Condition handling: ternary conditions are ternary_cond_t (int8_t with trit values).
- Conversions: explicit casts use helper functions; implicit conversions require plugin lowering.

## Instruction Semantics

### Arithmetic

- tadd, tsub, tmul, tdiv, tmod
- Unary: tneg, tnot (tritwise inversion: -1 <-> +1, 0 -> 0)

### Logic

Ternary logic is defined with min/max semantics:

- tand(a,b) = min(a,b)
- tor(a,b)  = max(a,b)
- txor(a,b) = 0 if a == b, else -a if b == 0, else -b if a == 0, else 0

This definition is stable and documented.

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

## Ternary Arithmetic Unit (TAU) Opcodes

This section enumerates ternary-native opcodes expected for a TAU. These are not
reused binary opcodes; they operate on trits or ternary values directly and are
intended to be exposed via the GCC plugin and helper ABI.

### Core Tritwise Ops

- tnot: tritwise inversion (-1 <-> +1, 0 -> 0)
- tand: per-trit min(a, b)
- tor: per-trit max(a, b)
- txor: ternary XOR (ISA-defined and stable)
- tequiv: tritwise equivalence (Kleene XNOR) that propagates unknowns and returns +1 only when the inputs match.
- tmin: per-trit minimum (alias of tand, optional)
- tmax: per-trit maximum (alias of tor, optional)

### Ternary Arithmetic (Value Ops)

- tadd: ternary add
- tsub: ternary subtract
- tmul: ternary multiply
- tdiv: ternary divide (division by zero returns 0)
- tmod: ternary modulo (division by zero returns 0)
- tneg: unary negate
- tmuladd: ternary multiply-add with symmetric rounding (A×B + C)
- tround: drop least-significant trits with symmetric rounding bias
- tabs: absolute value (optional helper if not encoded in hardware)
- tnet: net trit sum (count(+1) − count(−1)) for fast majority/balance checks.

### Ternary Comparison and Condition Ops

- tcmp: ternary compare returning {-1, 0, +1}
- tsel: select with ternary condition (cond != 0)
- tcmpz: compare against zero, returning {-1, 0, +1} (optional shortcut)
- tmux: data multiplexer that selects among three operands based on a ternary control trit (-1,0,+1).

### Trit Shifts and Rotates

- tshl: shift left by trits, fill with 0
- tshr: arithmetic shift right by trits
- trol: rotate left by trits
- tror: rotate right by trits

### Ternary Normalization and Conversion

- tclamp: clamp each trit to {-1, 0, +1} (for sanitize/repair)
- tnorm: normalize packed representation to canonical form
- tb2t: binary container to ternary logical value
- tt2b: ternary logical value to binary container
- tquant: quantize binary/float inputs to nearest trit values (useful for ternary-quantized neural nets)

## Ternary Base ISA v0

This section defines a virtual ISA contract. Encodings are intentionally left open,
but operand formats and semantics are fully specified. The ISA operates on packed
2-bit trits, using the encoding 00 = -1, 01 = 0, 10 = +1, 11 = reserved.

### Operand Types

- Tn: ternary integer register (logical value, stored as packed trits).
- Rn: binary integer register (for addresses and binary containers).
- Fn: IEEE floating-point register (f32/f64).
- Imm: immediate value (binary-encoded).

Ternary registers have a fixed trit width per instruction variant (e.g., t32, t64,
t128). The width is encoded in the mnemonic suffix.

### Register and Instruction Formats

The base ISA assumes a three-operand format for ternary ALU operations:

- tOP.tN Td, Ta, Tb   (ternary arithmetic/logic on N trits)
- tOP.tN Td, Ta       (ternary unary operations on N trits)
- tsel.tN Td, Tc, Ta, Tb (select with ternary condition Tc)
- tcmp.tN Rd, Ta, Tb  (binary result -1/0/+1 in Rd)

Where:
- Td/Ta/Tb/Tc are ternary registers of width tN.

## Roadmap / Next Steps

Priority | Suggestion | Why it helps | Status
--- | --- | --- | ---
High | Finish/clean lowering for common ops (add/sub/mul/neg/cmp/select/conv) | Lets users write straightforward ternary arithmetic without manual loops | In progress
High | Small portable runtime lib for t32/t64 | Enables real programs and benchmarking | Implemented (reference runtime in `runtime/`)
Medium | Trit-count-aware constant folding in plugin | Allows constants like `t32_t x = 42` to fold when representable | In progress
Medium | Better ternary type creation and user-facing syntax (e.g., `__ternary(32)` or attributes) | Easier adoption | Planned
Medium | Dump stats on how many ternary ops survive to RTL/assembly | Quantifies remaining work for hardware targets | Planned
Low | Larger trit counts (t256+) via limb-based runtime emulation | Future-proofs larger types | Planned
Low | Branch-free ternary↔binary conversion helpers | Improves emulation speed | Planned
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
- timpl.tN Td, Ta, Tb: Kleene implication per trit (see `MASTER_ISA.md` for the truth table)
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

The ISA mnemonics are always defined in terms of helper semantics over packed
binary containers. Hardware support, if present, must be bit-for-bit compatible
with the helper ABI.

## Memory Model

- Addresses are binary.
- Ternary data is loaded/stored via binary containers with defined packing.
- Alignment follows the container size.

## Control Flow

Branches use ternary conditions:

- brt cond, label: branch if cond != 0
- brf cond, label: branch if cond == 0

Hardware implementers can reference `ENCODING.md` for the tryte field layout that packs the
conditional helpers (TBRANCH/TMUX) plus the family/flag bits used by the async arbiter described in `MASTER_ISA.md`.

Ternary-aware helpers like `tbranch` and `tsignjmp` (see `MASTER_ISA.md`) allow
three-way control-flow selection or sign-directed jumps without chaining binary
flags, matching Setun’s triple-transition model.

Comparisons produce ternary results and are used directly by branches.

## ABI and Calling Convention

### Storage layout

- Balanced-ternary values are packed into binary containers with a 2-bit per trit encoding
  (00 = -1, 01 = 0, 10 = +1, 11 = reserved), and bit patterns must survive loads/stores/calls.
- The canonical containers map as follows: `t32_t` → 64-bit, `t64_t` → 128-bit, and `t128_t` →
  256-bit storage. Vector helpers extend this scheme (e.g., `tv32_t` is 128 bits for 2 × t32_t,
  `tv64_t` is 256 bits for 2 × t64_t).
- The helper/`runtime/ternary_runtime.h` headers expose the same layout so that software and
  hardware implementations remain bit-for-bit compatible.

### Condition type

- The plugin lowers every condition expression to `ternary_cond_t` before calling helpers.
- `ternary_cond_t` defaults to `int64_t` (per `ternary_helpers.h` and `ternary_runtime.h`) but
  may be overridden by consumer code; non-zero values mean true and zero means false.
- Packed ternary comparisons are decoded through `tt2b` before selective operations consult
  this ABI condition type.

### Registers

Registers are ternary-width, but stored in binary containers per ABI.

### Argument Passing

- Ternary integers are passed in ternary registers when the backend supports them, otherwise
  they travel in binary containers occupying the canonical storage width (e.g., `t64_t` takes
  128 bits in a register or stack slot).
- Floating point arguments reuse the existing IEEE FP register conventions.

### Return Values

Return ternary integers in the designated ternary register or the integer container register
that corresponds to the storage width.

### Varargs

Varargs are passed using binary containers. The callee interprets the container based on the
promoted ternary type. The helper headers provide `TERNARY_VA_ARG_T32`/`TERNARY_VA_ARG_T64`
macros (formerly referenced as T6/T12/T24) to decode promotions for packed ternary types.

## Toolchain Contract

### Builtins

The GCC plugin defines builtin functions that map directly to the helper ABI. They are grouped
into the following categories (see `include/ternary.h`):

- **Select and arithmetic**: `__builtin_ternary_select`, `__builtin_ternary_add`,
  `__builtin_ternary_sub`, `__builtin_ternary_mul`, `__builtin_ternary_div`,
  `__builtin_ternary_mod`, `__builtin_ternary_neg`.
- **Logic**: `__builtin_ternary_not`, `__builtin_ternary_and`, `__builtin_ternary_or`,
  `__builtin_ternary_xor`.
- **Integer comparisons (binary results)**: `__builtin_ternary_cmp`,
  `__builtin_ternary_eq`, `__builtin_ternary_ne`, `__builtin_ternary_lt`,
  `__builtin_ternary_le`, `__builtin_ternary_gt`, `__builtin_ternary_ge`.
- **Shift/rotate**: `__builtin_ternary_shl`, `__builtin_ternary_shr`, `__builtin_ternary_rol`,
  `__builtin_ternary_ror`.
- **Conversions**: `__builtin_ternary_tb2t`, `__builtin_ternary_tt2b`, `__builtin_ternary_t2f`,
  `__builtin_ternary_f2t`.
- **Ternary comparisons (ternary results)**: `__builtin_ternary_cmplt`, `__builtin_ternary_cmpeq`,
  `__builtin_ternary_cmpgt`, `__builtin_ternary_cmpneq`, and their `_t64` variants.
- **Memory helpers**: `__builtin_ternary_load_t32`, `__builtin_ternary_store_t32`,
  `__builtin_ternary_load_t64`, `__builtin_ternary_store_t64`.
- **Vector SIMD helpers**: `__builtin_ternary_add_tv32`, `__builtin_ternary_sub_tv32`,
  `__builtin_ternary_mul_tv32`, `__builtin_ternary_and_tv32`, `__builtin_ternary_or_tv32`,
  `__builtin_ternary_xor_tv32`, `__builtin_ternary_not_tv32`, `__builtin_ternary_cmp_tv32`,
  plus the `tv64` equivalents.

### Custom Types

When `-fplugin-arg-ternary_plugin-types` is enabled, the plugin registers builtin
types `t32_t`, `t64_t`, and `t128_t` with packed 2-bit
trit storage (precision = trit_count * 2).

### Helper ABI

Helper functions implement the ISA-visible operations and expose the storage ABI to
language front ends. Every helper name is mirrored in `include/ternary_runtime.h` so
helper implementations can be shared between plugin tests and downstream runtimes.

- **Select helpers** (integers, unsigned, floats, ternary scalars): `__ternary_select_i8`,
  `__ternary_select_i16`, `__ternary_select_i32`, `__ternary_select_i64`, `__ternary_select_u8`,
  `__ternary_select_u16`, `__ternary_select_u32`, `__ternary_select_u64`, `__ternary_select_f32`,
  `__ternary_select_f64`, `__ternary_select_t32`, `__ternary_select_t64`.
- **Integer arithmetic/logical helpers**: `__ternary_add`, `__ternary_sub`, `__ternary_mul`,
  `__ternary_div`, `__ternary_mod`, `__ternary_neg`, `__ternary_not`, `__ternary_and`,
  `__ternary_or`, `__ternary_xor`.
- **Integer comparisons (binary results)**: `__ternary_cmp`, `__ternary_cmp_t32`, `__ternary_cmp_t64`,
  `__ternary_eq`, `__ternary_ne`, `__ternary_lt`, `__ternary_le`, `__ternary_gt`,
  `__ternary_ge`.
- **Shift/rotate helpers**: `__ternary_shl`, `__ternary_shr`, `__ternary_rol`, `__ternary_ror`.
- **Packed ternary scalar helpers** (t32/t64): `__ternary_add_t32`, `__ternary_add_t64`,
  `__ternary_mul_t32`, `__ternary_mul_t64`, `__ternary_not_t32`, `__ternary_not_t64`,
  `__ternary_sub_t32`, `__ternary_sub_t64`, `__ternary_div_t32`, `__ternary_div_t64`,
  `__ternary_mod_t32`, `__ternary_mod_t64`, `__ternary_neg_t32`, `__ternary_neg_t64`,
  `__ternary_and_t32`, `__ternary_and_t64`, `__ternary_or_t32`, `__ternary_or_t64`,
  `__ternary_xor_t32`, `__ternary_xor_t64`, `__ternary_shl_t32`, `__ternary_shl_t64`,
  `__ternary_shr_t32`, `__ternary_shr_t64`, `__ternary_rol_t32`, `__ternary_rol_t64`,
  `__ternary_ror_t32`, `__ternary_ror_t64`.
- **Packed ternary logic helpers** (AI-friendly semantics):
  `__ternary_tmin_t32`, `__ternary_tmin_t64`, `__ternary_tmax_t32`, `__ternary_tmax_t64`,
  `__ternary_tmaj_t32`, `__ternary_tmaj_t64`, `__ternary_tlimp_t32`, `__ternary_tlimp_t64`,
  `__ternary_tquant_t32`, `__ternary_tquant_t64`, `__ternary_tquant_tv32`,
  `__ternary_tnot_t32`, `__ternary_tnot_t64`, `__ternary_tmuladd_t32`,
  `__ternary_tmuladd_t64`, `__ternary_tround_t32`, `__ternary_tround_t64`,
  `__ternary_tround_tv32`, `__ternary_tnormalize_t32`, `__ternary_tnormalize_t64`,
  `__ternary_tbias_t32`, `__ternary_tbias_t64`, `__ternary_tmux_t32`,
  `__ternary_tmux_t64`, `__ternary_tequiv_t32`, `__ternary_tequiv_t64`,
  `__ternary_txor_t32`, `__ternary_txor_t64`, `__ternary_tnet_t32`,
  `__ternary_tnet_t64`.
- **Packed ternary logic helpers (t128, when `_BitInt(256)` is available)**:
  `__ternary_tmin_t128`, `__ternary_tmax_t128`, `__ternary_tmaj_t128`,
  `__ternary_tlimp_t128`, `__ternary_tquant_t128`, `__ternary_tnot_t128`,
  `__ternary_tinv_t128`, `__ternary_tmuladd_t128`, `__ternary_tround_t128`,
  `__ternary_tnormalize_t128`, `__ternary_tbias_t128`, `__ternary_tmux_t128`,
  `__ternary_tequiv_t128`, `__ternary_txor_t128`, `__ternary_tnet_t128`.
- **Ternary control helpers**: `__ternary_tbranch`, `__ternary_tsignjmp_t32`,
  `__ternary_tsignjmp_t64`.
- **Packed ternary comparisons (ternary results)**: `__ternary_cmplt_t32`, `__ternary_cmplt_t64`,
  `__ternary_cmpeq_t32`, `__ternary_cmpeq_t64`, `__ternary_cmpgt_t32`, `__ternary_cmpgt_t64`,
  `__ternary_cmpneq_t32`, `__ternary_cmpneq_t64`.
- **Conversion helpers**: `__ternary_tb2t_t32`, `__ternary_tb2t_t64`, `__ternary_tt2b_t32`,
  `__ternary_tt2b_t64`, `__ternary_t2f32_t32`, `__ternary_t2f32_t64`, `__ternary_t2f64_t32`,
  `__ternary_t2f64_t64`, `__ternary_f2t32_t32`, `__ternary_f2t32_t64`, `__ternary_f2t64_t32`,
  `__ternary_f2t64_t64`.
- **Conversion helpers (t128, when `_BitInt(256)` is available)**:
  `__ternary_tb2t_t128`, `__ternary_tt2b_t128`.
- **Memory helpers**: `__ternary_load_t32`, `__ternary_store_t32`, `__ternary_load_t64`,
  `__ternary_store_t64`.
- **Vector SIMD helpers**: `__ternary_add_tv32`, `__ternary_sub_tv32`, `__ternary_mul_tv32`,
  `__ternary_and_tv32`, `__ternary_or_tv32`, `__ternary_xor_tv32`, `__ternary_not_tv32`,
  `__ternary_cmp_tv32`, `__ternary_add_tv64`, `__ternary_sub_tv64`, `__ternary_mul_tv64`,
  `__ternary_and_tv64`, `__ternary_or_tv64`, `__ternary_xor_tv64`, `__ternary_not_tv64`,
  `__ternary_cmp_tv64`.
- **Literal parsing helpers**: `__ternary_bt_str_t32`, `__ternary_bt_str_t64`.

Conversion helpers that return floating point scalars are still named `__ternary_t2f32`,
`__ternary_t2f64`, `__ternary_f2t32`, `__ternary_f2t64` for the standard `t32_t`/`t64_t`
interpretation.

cond_t is `ternary_cond_t` (default: `int64_t`). The plugin lowers conditions to
`ternary_cond_t` before calling helpers, and the reference helper header uses the same
packed 2-bit trit encoding (00 = -1, 01 = 0, 10 = +1).

### GCC Plugin Behavior

- GIMPLE pass scans COND_EXPR assignments.
- Optional warnings and stats.
- Lowering replaces ternary selects with helper calls.
- Builtin calls are lowered to helper calls when enabled.

## Testing and Validation

- Ternary select: int, unsigned, float, double, and custom ternary types (t32_t, t64_t).
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
- Logic builtin lowering (not, and, or, xor)
- Comparison builtin lowering (cmp)
- Helper function ABI with packed C implementations for testing
- Test suite covering all implemented operations
- Builtin ternary type registration (t32_t, t64_t, t128_t)
- Shift/rotate builtin lowering (t32/t64)
- Conversion builtin lowering (tb2t, tt2b, t2f, f2t)

### Extended helper ABI

- Three-way selector helpers: `__ternary_tmux_t32`, `__ternary_tmux_t64`, `__ternary_tmux_t128`.
- Kleene equivalence helpers: `__ternary_tequiv_t32`, `__ternary_tequiv_t64`, `__ternary_tequiv_t128`.
- Ternary-exclusive-or helpers: `__ternary_txor_t32`, `__ternary_txor_t64`, `__ternary_txor_t128`.
- Net-sum helpers: `__ternary_tnet_t32`, `__ternary_tnet_t64`, `__ternary_tnet_t128`.
- Demonstrated via the TMUX/TNET/TNN runtime skeleton demo, including t32/t64/t128 workloads.

### Known Limitations
- GCC 15 API compatibility may still require adjustments
- Packed ternary helpers require `__BITINT_MAXWIDTH__ >= 256` for native `t128_t`; fallback paths mirror the two-`t64_t` layout.
- Reference runtime/helpers only simulate t128 operations when the compiler exposes `_BitInt(256)` support.

## Future Extensions

- Track the **Master ISA Roadmap** (`MASTER_ISA.md`) so the new ternary-only helpers (TMIN/TMAX/TIMPL/TMAJ, etc.) are reflected inside this spec and the helper ABI.
- Native ternary floating point formats.
- Vector/SIMD ternary operations.
- Full ternary logic instruction set (tand, tor, txor).
- Shift and rotate operations.
- Conversion operations between binary and ternary representations.
- Binary/ternary mixed-mode codegen and scheduling.
- GCC version compatibility fixes.

## Still Needs Implementation

1. Custom ternary types
   - Implemented: builtin types t32, t64, t128 (packed 2-bit trits)
   - Implemented: storage ABI for 2-bit trit packing
   - Remaining: helper/runtime support for t128 (requires big-int support)
2. Extended arithmetic operations
   - Implemented: tsub, tdiv, tmod, tneg builtins and helpers
   - Remaining: validation coverage for edge cases and larger widths
3. Ternary logic operations
   - Implemented: tand, tor, txor builtins and helpers (min/max/xor semantics)
   - Remaining: validation coverage for edge cases and larger widths
4. Comparison operations
   - Implemented: tcmp builtins and helpers (-1/0/+1)
   - Remaining: validation coverage for edge cases and larger widths
5. Shifts and rotates
   - Implemented: trit-based shl/shr/rol/ror builtins and helpers (t32/t64)
   - Remaining: validation coverage and larger width support
6. Type conversions
   - Implemented: tb2t/tt2b and t2f/f2t builtins and helpers (t32/t64)
   - Remaining: precise rounding semantics for larger widths
7. Full ISA integration
   - Missing: Complete mapping to all specified ISA instructions
   - Missing: Proper register allocation and calling conventions for ternary types
8. Optional hardware backend
   - Missing: ISA encoding and codegen path for acceleration
   - Non-blocking: software helpers remain the reference

## Priority Order

- High: Helper support for larger ternary widths (t128)
- Medium: Varargs ABI validation tests for ternary types
- Low: Optional hardware ISA support for ternary mnemonics
