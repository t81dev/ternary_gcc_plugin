# Phase 3/4 Regression Coverage

Phase 3 (Lowering and GIMPLE coverage) and Phase 4 (diagnostics and codegen validation)
share a tight feedback loop. To keep the roadmap milestones explicit, we track both the
regression targets and any emitted diagnostics in a single place.

## Scripted Coverage

Run `tests/run_phase34_coverage.sh` from the `tests/` directory to rebuild the plugin,
exercise representative lowering flag combinations, and capture the outputs/diagnostics for
future compare runs. The script writes timestamps + command outputs to `tests/phase34_coverage.log`.

## Targeted Regression Entries

Each command encodes the features we care about:

1. **Phase 3 – Lowering (GIMPLE dump)**: compile `test_ternary.c` with `-fplugin-arg-ternary_plugin-lower`
   and `-dump-gimple` so we repeatedly repro the GIMPLE paths the plugin rewrites.
2. **Phase 3 – Literal/Promotion helpers**: compile `test_literals.c` with `-fplugin-arg-ternary_plugin-types`
   to confirm the promotion, literal, and conversion builtins are still covered by the plugin.
3. **Phase 4 – Diagnostics (trace + dump)**: compile `test_ternary.c` with trace/dump flags so any new
   diagnostic output (warnings, traces) lands in the shared log file.

## Logging Expectations

- `tests/phase34_coverage.log` acts as a living diagnostics artifact for these runs.
  Each invocation of `run_phase34_coverage.sh` overwrites the log so stale diagnostics do not persist.
- The log records when each sub-step starts, what command is executed, and any stderr emitted (e.g., plugin
  warnings/diagnostics). Review it when Phase 3/4 changes are introduced to make regressions visible.

## Roadmap Tie-In

Use this document and the generated log to close the loop for Phase 3 and Phase 4 deliverables,
and update this file when new regression entries are required for upcoming releases.
