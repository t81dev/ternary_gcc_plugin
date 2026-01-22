# Runtime Skeleton

This directory provides a minimal, auditable runtime helper set for the GCC
ternary plugin. It is intended as a starting point for a standalone runtime
library.

## Contents

- `include/ternary_runtime_skeleton.h`: minimal helper declarations for t32/t64 (and t128 when `_BitInt(256)` is available).
- `src/ternary_runtime_skeleton.c`: tiny reference implementations.

## Build

```bash
cc -Iruntime_skeleton/include -c runtime_skeleton/src/ternary_runtime_skeleton.c -o build/ternary_runtime_skeleton.o
```

## Provided helpers

- Scalar helpers: `__t81_ternary_add`, `__t81_ternary_sub`, `__t81_ternary_mul`, `__t81_ternary_div`, `__t81_ternary_mod`,
  `__t81_ternary_neg`, `__t81_ternary_not`, `__t81_ternary_and`, `__t81_ternary_or`, `__t81_ternary_xor`,
  `__t81_ternary_shl`, `__t81_ternary_shr`, `__t81_ternary_rol`, `__t81_ternary_ror`, `__t81_ternary_cmp`,
  `__t81_ternary_eq`, `__t81_ternary_ne`, `__t81_ternary_lt`, `__t81_ternary_le`, `__t81_ternary_gt`, `__t81_ternary_ge`
- `__t81_ternary_add_t32`, `__t81_ternary_neg_t32`, `__t81_ternary_select_t32`
- `__t81_ternary_sub_t32`, `__t81_ternary_mul_t32`, `__t81_ternary_cmp_t32`
- `__t81_ternary_div_t32`, `__t81_ternary_mod_t32`
- `__t81_ternary_and_t32`, `__t81_ternary_or_t32`, `__t81_ternary_xor_t32`
- `__t81_ternary_shl_t32`, `__t81_ternary_shr_t32`, `__t81_ternary_rol_t32`, `__t81_ternary_ror_t32`
- `__t81_ternary_tb2t_t32`, `__t81_ternary_tt2b_t32`
- `__t81_ternary_add_t64`, `__t81_ternary_neg_t64`, `__t81_ternary_select_t64`
- `__t81_ternary_sub_t64`, `__t81_ternary_mul_t64`, `__t81_ternary_cmp_t64`
- `__t81_ternary_div_t64`, `__t81_ternary_mod_t64`
- `__t81_ternary_and_t64`, `__t81_ternary_or_t64`, `__t81_ternary_xor_t64`
- `__t81_ternary_shl_t64`, `__t81_ternary_shr_t64`, `__t81_ternary_rol_t64`, `__t81_ternary_ror_t64`
- `__t81_ternary_tb2t_t64`, `__t81_ternary_tt2b_t64`
- `__t81_ternary_tequiv_t32`, `__t81_ternary_txor_t32`, `__t81_ternary_tmux_t32`, `__t81_ternary_tnet_t32`
- `__t81_ternary_tequiv_t64`, `__t81_ternary_txor_t64`, `__t81_ternary_tmux_t64`, `__t81_ternary_tnet_t64`
- `__t81_ternary_tequiv_t128`, `__t81_ternary_txor_t128`, `__t81_ternary_tmux_t128`, `__t81_ternary_tnet_t128`

## Notes

- These implementations use simple encode/decode routines and are not optimized.
- The integer conversion helpers are limited by `int64_t` range in this skeleton.
- The exported symbol prefix is configurable via `TERNARY_RUNTIME_PREFIX` (default: `__t81_ternary_`).
- Compatibility wrappers for `__ternary_*` names are built unless you define `TERNARY_RUNTIME_NO_COMPAT`.
- Extended t128 helper coverage is now implemented (see `../docs/MASTER_ISA.md` step 5); the skeleton exposes TMUX/TNET/TEQUIV/TXOR for `_BitInt(256)` builds so the same ternary-only helpers can be validated on wider registers.
- Build/run the TMUX-based demo and CI benchmark:
  ```bash
  ./runtime_skeleton/run_tnn_demo.sh
  ```
  The script compiles `ternary_tnn_demo.c`, prints the TNET sum, shows TMUX routing, and exercises
  TEQUIV/TXOR semantics; a CI job can rerun the script to keep the async-control paths covered.
- ### TMUX-based Ternary Neural Network Toy
- Use `__t81_ternary_tmux_t32` to route activations based on ternary condition trits without chaining selects. The TMUX helper can pick one of three ternary kernel outputs (`neg`, `zero`, `pos`) in a single cycle, mirroring the async arbiter described in `../docs/MASTER_ISA.md` and `../docs/ENCODING.md`. This is handy for ternary inference paths where the sign of an accumulated dot product should choose the next stage.
- Run the TMUX/TNET/TNN demo via `make runtime-skeleton-test` (root Makefile target) or `./run_tnn_demo.sh` to validate the ternary-only helpers on t32, t64, and—when `_BitInt(256)` is available—t128. This CI-friendly target builds the skeleton helpers plus the demo, exercising TEQUIV/TXOR/TNET/TMUX everywhere the ISA roadmap requires.
