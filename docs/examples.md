# Guided ternary examples

These examples are designed to be read top to bottom, one semantic step at a time. Each tier introduces exactly one new ternary concept, builds on the helper ABI from earlier tiers, and maps the pattern back to the roadmap phases described in `ROADMAP.md`. Agents and humans alike should treat the helper calls observed in the emitted code as the canonical representation of ternary semantics inside GCC.

| Tier | Example | Source | Plugin flags | Key helpers / semantics | Roadmap focus | What to notice |
|------|---------|--------|--------------|-------------------------|---------------|----------------|
| 1 | **Hello ternary select** — write a ternary condition and select between two `int` values so new users see how `TERNARY_COND_T` flows into `__ternary_select_i32`. | `examples/example1_select.c` | `-fplugin-arg-ternary_plugin-lower` | `TERNARY_COND_T` value → `__ternary_select_i32` | Phase 0/2 (C surface) | Conditional semantics become data |
| 2 | **Binary-to-ternary conversion** — call `__ternary_tb2t_t32`/`__ternary_tt2b_t32` plus the balanced-ternary string helper (`__ternary_bt_str_t32`) to show how binary ints become packed trits and back. | `examples/example2_conversion.c` | `-fplugin-arg-ternary_plugin-conv` | `__ternary_tb2t_t32`, `__ternary_tt2b_t32`, `__ternary_bt_str_t32` | Phase 1 (runtime conversions) | Binary ↔ ternary boundary is explicit |
| 3 | **Arithmetic chain** — add/mul/sub/muladd a few ternary values so the lowered helpers (`__ternary_add_t32`, `__ternary_mul_t32`, `__ternary_tmuladd_t32`) appear in sequence. | `examples/example3_arithmetic.c` | `-fplugin-arg-ternary_plugin-arith` | math helpers | Phase 3 (lowering coverage) | Arithmetic lowers 1:1 to helpers |
| 4 | **Logic/min/max** — compute `tmin`, `tmax`, `tequiv`, `txor` on packed inputs to illustrate ternary logic semantics. | `examples/example4_logic.c` | `-fplugin-arg-ternary_plugin-logic` | `__ternary_tmin_t32`, `__ternary_tequiv_t32`, `__ternary_txor_t32` | Phase 3 | Logic operators stay ternary-aware |
| 5 | **Conditional routing pipeline** — use `if / else if` ternary conditions to demonstrate how control intent is lowered into `__ternary_tbranch` / `__ternary_tsignjmp_*`, without yet emitting native branch instructions. | `examples/example5_branch.c` | `-fplugin-arg-ternary_plugin-lower -fplugin-arg-ternary_plugin-cmp -fplugin-arg-ternary_plugin-trace -fplugin-arg-ternary_plugin-dump-gimple` | `__ternary_cmp_t32`, `__ternary_tbranch`, `__ternary_tsignjmp_t32` | Phase 3/4 (control flow) | Control intent is reified w/out native branch |
| 6 | **Vector arithmetic** — operate on `tv32_t` (or `tv64_t`) values so `__ternary_add_tv32`/`__ternary_cmp_tv32` appear; highlight how the runtime folds vector helpers to scalar `t32_t` helpers for correctness while the ABI stays the same. | `examples/example6_vector.c` | `-fplugin-arg-ternary_plugin-vector` | `__ternary_add_tv32`, `__ternary_cmp_tv32` | Phase 7 (SIMD exploration) | Vector helpers canonicalize to scalars |
| 7 | **Shift/rotate pipeline** — combine `__ternary_shl_t32`, `__ternary_shr_t32`, `__ternary_rol_t32`, `__ternary_ror_t32` to demonstrate trit-level shifts. | `examples/example7_shift.c` | `-fplugin-arg-ternary_plugin-shift` | shift/rotate helpers | Phase 4 (validation) | Trit-level updates stay consistent |
| 8 | **Memory load/store** — show how `t32_t` values persist through `__ternary_load_t32`/`__ternary_store_t32`, so the ABI matches binary-compatible storage. | `examples/example8_memory.c` | `-fplugin-arg-ternary_plugin-mem` | `__ternary_load_t32`, `__ternary_store_t32` | Phase 3 | ABI matches binary storage |
| 9 | **TMUX/TNET routing** — an AI-relevant kernel that gathers three paths (neg/zero/pos) via `__ternary_tmux_t32` and summarizes with `__ternary_tnet_t32`. | `examples/example9_tmux.c` | `-fplugin-arg-ternary_plugin-logic -fplugin-arg-ternary_plugin-arith` | `__ternary_tmux_t32`, `__ternary_tnet_t32`, `__ternary_tequiv_t32` | Phase 4 (ternary-specific ISA) | Routing replaces boolean control |
| 10 | **Coverage-driven diagnostics** — run the quickstart script to build the above examples and capture the `tests/run_phase34_coverage.sh` trace log so AI/engineers know which helpers emit diagnostics. | `tools/quickstart.sh` | n/a (script) | coverage log, helper list audit (`tools/check_helper_docs.py`) | Phase 3/4 regression | Diagnostics log is canonical |

Each example should include a minimal C/C++ snippet (see `examples/`), the plugin command line, and the helpers it pushes into the runtime. Adding these to the documentation (and ideally as runnable snippets in `examples/`) helps both humans and agents understand the progression from simple selects to complex routing/diagnostics.

Every tier compiles with the same template so the emitted helpers can be compared deterministically:

```bash
cc -Iinclude -fplugin=./ternary_plugin.so \
  <plugin-flags> examples/<name>.c runtime/ternary_runtime.c \
  -o build/examples/<name> && ./build/examples/<name>
```

Add `-fplugin-arg-ternary_plugin-trace` and `-fplugin-arg-ternary_plugin-dump-gimple` when you want to capture traces or GIMPLE dumps that list the helper calls for inspection.
