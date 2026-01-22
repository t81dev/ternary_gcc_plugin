# Master Ternary ISA Roadmap

This document sketches the instructions a balanced-ternary ISA should expose once we
grow beyond the core helpers already listed in `SPECIFICATION.md`. The goal is not to
duplicate binary primitives (add/mul/cmp are already covered there) but to capture the
**ternary-only logic** that emerges from Setun-style three-state arithmetic, Kleene logic,
and recent ternary/AI research. Each entry summarizes the semantics, why it is harder (or
impossible) to reproduce in binary, and what the plugin/runtime should provide.

## Existing foundation

The current GCC plugin and runtime already describe a complete helper ABI for the
standard ternary integer instructions (addition, compare, select, shifts, etc.).
Refer to `SPECIFICATION.md` for those helpers. This roadmap is intentionally additive and
focuses on instructions whose semantics *literally require* a three-valued perspective.

## Unique Ternary Operations

### 1. Three-valued logic primitives

- **TMIN / TMAX** – Combines two ternary operands per trit using Kleene-style min/max (−1
  < 0 < +1). These instructions directly propagate unknowns (`0`). Binary min/max
  instructions cannot distinguish three states without extra flags and masking, which
  multiplies latency. Useful as fast logical AND/OR substitutes for ternary boolean vectors.
- **TNOT** – Tritwise inversion that flips the sign of non-zero trits while keeping zero
  intact. Balanced ternary can negate entire words without a carry or add-one step, unlike
  two's-complement inversion. The helper should map to `__builtin_ternary_not` variants.
- **TIMPL** – Kleene implication (e.g., −1 implies anything = +1, 0 implies 0/ +1, etc.).
  It captures conditional reasoning with explicit “unknown” propagation, a common pattern in
  ternary neural nets and satisfiability solvers. Binary hardware would need a multi-instruction
  sequence to emulate the same partial truth table.
  | antecedent ↓ \ consequent → | -1 (false) | 0 (unknown) | +1 (true) |
  |----------------------------|-------------|--------------|------------|
  | -1 (false)                 | +1          | +1           | +1         |
  | 0 (unknown)                | -1          | 0            | +1         |
  | +1 (true)                  | -1          | 0            | +1         |
- **TMAJ** – Element-wise majority vote on three inputs (extendable to vectors). Returns
  the trit that appears at least twice; when `(−1, 0, +1)` all appear, it resolves to `0`
  (unknown). This matches REBEL-6’s majority voter and is useful for error-correcting memory
  and ensemble ternary inference.
  | inputs (a,b,c)                  | result |
  |---------------------------------|--------|
  | two or more `+1` values          | +1     |
  | two or more `0` values           | 0      |
  | two or more `−1` values          | −1     |
  | `(-1, 0, +1)` (all distinct)     | 0      |
- **TEQUIV** – Tritwise equivalence (balanced ternary XNOR). Returns +1 if both trits match and are non-zero, 0 if either trit is 0, and −1 otherwise. This provides a symmetric similarity test without losing unknown propagation (unlike chaining binary XNOR).
  | input ↓ \ input → | -1 | 0 | +1 |
  |------------------|----|---|----|
  | -1               | +1 | 0 | -1 |
  | 0                | 0  | 0 | 0  |
  | +1               | -1 | 0 | +1 |
- **TXOR** – Tritwise exclusive-or (a + b mod 3). Outputs 0 when inputs match, ±1 when they differ, and keeps unknowns as neutral. Ideal for ternary hashing/diff operations where sign matters but zero must stay neutral.
  | input ↓ \ input → | -1 | 0 | +1 |
  |------------------|----|---|----|
  | -1               | 0  | -1 | +1 |
  | 0                | -1 | 0  | +1 |
  | +1               | +1 | -1 | 0  |

### 2. Control flow that uses three-way decisions

- **TBRANCH** – Single instruction with three potential targets keyed by condition trit
  values (−1, 0, +1). Instead of chaining binary comparisons, a ternary CPU can implement
  decision trees or state machines with exactly one branch per comparison. This aligns with
  Setun’s three-way transition commands.
- **TSIGNJMP** – Jump based on the sign trit of a register (positive/zero/negative), letting
  late-stage code skip redundant compares or flags. A binary design must inspect zero and sign
  flags separately, but balanced ternary exposes the sign trit directly.
- **TMUX** – Three-way data multiplexer controlled by a ternary trit. Chooses operand A for −1,
  B for 0, and C for +1, mirroring `TBRANCH`’s select logic but for data paths. This avoids chained
  binary muxes in ternary state machines and pairs with the async control arbiter metadata.

### 3. Symmetric arithmetic helpers for AI / DSP workloads

- **TINV** – Multi-trit inversion operation that flips +1↔−1 while leaving zeros. In ternary
  inference kernels (e.g., ternary neural nets) this is often the only change between weights.
  It maps naturally to `__builtin_ternary_not` but it can be kept distinct for vector acceleration.
- **TMULADD** – Combined multiply-and-accumulate with symmetric rounding (A×B + C). Setun
  used this for polynomial evaluation, and ternary addition/multiplication avoid bias that
  plagues binary MAC units (no implicit +0.5 shifts). A TMULADD helper can replace the current
  multiply + add sequence with one lower-latency call.
- **TROUND** – Truncates the least-significant trits of a value using symmetric rounding (round
  toward zero with equal bias). Binary rounding must track sign + carry to avoid bias, but
  balanced ternary simply drops trits because truncation equals rounding.
- **TQUANT** – Quantizes binary or floating values into {−1, 0, +1} per trit, targeting low-precision
  AI weights. Binary quantization typically lands at {0,1} or 8-bit ints; ternary quantization
  captures more information with fewer bits. This helper is critical for runtime math that
  prepares data for `TMIN/TMAX`-style activations.
- **TNET** – Net trit sum: emit the count of `+1` values minus the count of `−1` values within a register. Balanced ternary makes this reduction carry-free and perfectly symmetric, serving as a lightweight “balance” check for ternary neural networks or error-detecting schemes.

### 4. Memory / conversion utilities beyond standard packing

- **TPACK / TUNPACK** – Encodes/decodes arbitrary ternary strings or vector lanes into the
  2-bit representation (00 = −1, 01 = 0, 10 = +1). They hide the packing format so higher-level
  code can consume ternary data from external sources without bit fiddling.
- **TBIAS** – Adds/subtracts a fixed bias to reinterpret balanced vs. biased ternary forms
  (useful for offsets or unsigned slices). Binary systems can do biasing, but the symmetry in
  ternary makes bias transitions cheaper and worth exposing explicitly.
- **TNORMALIZE** – Replays the trit encoding to clamp invalid bit patterns (like `11`). This
  keeps memory/IO sane when binary corruption bypasses helper validation.

## Implementation guidance

- Expose the above helpers through the existing plugin options (`-fplugin-arg-ternary_plugin-logic`)
  and helper headers (`include/ternary_helpers.h`). Prefix names with the current `<prefix>` so
  downstream runtimes can reuse `__ternary_tmin`/`__ternary_tbranch`, etc.
- Provide reference C fallbacks (in `runtime/ternary_runtime.c`) that mirror the ternary truth tables
  before introducing hardware acceleration.
- Add unit tests (e.g., `tests/test_logic.c`) that exercise the unknown propagations and three-way
  jumps. Use the runtime skeleton to demonstrate symmetric rounding and quantization.
- Document how each helper avoids binary duplication and cite Setun/REBEL-6 inspiration for the
  three-valued control-flow primitives.

### Hardware encoding & async control

- Tryte-friendly opcodes: reserve 6 trits per slot (e.g., 2 trits for vector width, 2 trits for the instruction family, 2 trits for immediates/flags). Instructions such as `TBRANCH`, `TMAJ`, and `TMULADD` can live in a single tryte with built-in width tags so the hardware decoder can dispatch without binary prefix tables.
- Async branch support: follow Setun’s clockless transitions by treating `TBRANCH`/`TSIGNJMP` as micro-instructions that grab a condition trit and target triple simultaneously, avoiding the binary flag expansion. Documenting this early helps map the runtime helpers to future hardware that may signal targets through a branch arbiter rather than the normal program counter updates.
- Async control handshake: model hardware such that the condition feed (ternary condition register) produces one of three ready signals, and an arbiter selects the matching target label/offset in the same cycle. `TBRANCH` should therefore expose a triple-target field plus a flag that indicates whether the jump is speculative; software helpers can mirror that by returning the chosen target value so forward progress can be verified even before hardware support arrives.

## Next steps

1. Align this roadmap with `SPECIFICATION.md` by referencing the new helpers in the “Future Extensions”
   or “Needs Implementation” lists.
2. Choose a canonical helper prefix (`__ternary` by default) and append the new operations to that ABI.
3. Prototype the quantization/branch helpers in `runtime_skeleton/` so plugin tests can depend on them.
4. Sketch hardware encodings (tryte/6-trit opcodes) once the semantics settle.
