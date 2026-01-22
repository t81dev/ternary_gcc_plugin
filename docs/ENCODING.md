# Ternary ISA Encoding Notes

This file sketches how the new helpers (TBRANCH/TMUX/TMAJ/TNET, etc.) might be encoded in a future ternary ISA that favors tryte-aligned opcodes and asynchronous control.

## Tryte-friendly Opcode Layout

Balanced ternary instructions can pack more information per opcode than binary. A 6-trit “tryte” can be divided into:

- **2 trits**: data width tag
- **2 trits**: instruction family (e.g., logic, control, neural)
- **2 trits**: immediate flags / predicate bits

For example, TBRANCH and TMUX can share the same family slot (control) but use different predicate bits to indicate whether the instruction targets code or data. This layout keeps the decoder simple and lets the hardware dispatch a full control triple in one cycle.

| Tryte Field | Value (balanced ternary) | Meaning |
|-------------|--------------------------|---------|
| Width       | `-1` / `0` / `+1`        | selects 32/64/128 trit widths |
| Family      | `-1` / `0` / `+1`        | logic / control / arithmetic |
| Flags       | `-1` / `0` / `+1`        | condition inversion / speculation bits |

A `TBRANCH` instruction can carry three relative offsets/labels plus a flag that enables speculative dispatch, while `TMUX` uses the same control bits to choose among data paths.

## Async Control Handshake

Following Setun’s clockless transitions, the ternary branch path can be modeled as a handshake:

1. A ternary condition trit is produced by the comparison logic.
2. The arbiter observes the trit and activates one of three ready signals (negative/zero/positive).
3. The pipeline selects the matching target tag (one of the triple encoded next to the opcode).
4. Speculative bits indicate whether the branch outcome should be confirmed in a later cycle or aborted.

Because ternary conditions expose sign symmetry, `TSIGNJMP`/`TMUX` can reuse the same handshake: the runtime helper returns the chosen target (or data lane) immediately, and an arbiter verifies the trit feeds through to the next cycle before committing.

## TNN-Ready Units

Vectorized primitives such as `TMAJ` and `TNET` benefit from this encoding because they can receive their operand tag, width, and mask in a single tryte. For example, a `TNET` instruction on a 128-trit register might set the width field to `+1`, the family to `+1` (neural), and use the flags to toggle “fold-over” behavior for training-aware rounding.

These sketches guide future ASIC/FPGA work; the runtime helpers and skeleton demos mirror the same semantics in software.
