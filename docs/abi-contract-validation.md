# ABI Contract Validation

Roadmap linkage:
- `t81-roadmap#2`
- `t81-roadmap/PHASE1_STABILIZATION_MATRIX.md` (`P1-S3`)

This document defines the ABI contract validation path for `ternary_gcc_plugin`.

## Contract Scope

The ABI contract is the union of:

1. exported helper declarations in `include/ternary_runtime.h`,
2. helper implementation coverage in `runtime/ternary_runtime.c`,
3. helper symbol documentation in `README.md` and `docs/SPECIFICATION.md`.

## Drift Policy

Any change to helper declarations (add/remove/rename/signature change) must include:

1. header updates (`include/ternary_runtime.h` and related helper headers),
2. runtime implementation updates,
3. documentation updates (`README.md`, `docs/SPECIFICATION.md`, and this page if process changes),
4. successful ABI validation run via `scripts/validate_abi_contract.sh`.

## Reproducible Validation

Run:

```bash
scripts/validate_abi_contract.sh
```

This command performs:

1. helper-doc parity check (`tools/check_helper_docs.py`),
2. compile/link smoke checks against runtime ABI (`tests/test_abi.c`, `tests/test_varargs.c`),
3. execution of those ABI smoke binaries.

## Escalation Rule

If ABI drift impacts downstream ecosystem contracts, link follow-up work in:

1. `t81-roadmap` Phase 1 tracker,
2. downstream consumers (`t81-lang`, `t81-python`) when relevant.
