#!/usr/bin/env bash
set -euo pipefail

# Helper script to repeatedly exercise Phase 3/4 lowering coverage and capture any diagnostics.
# It compiles the familiar helper test source with different plugin flag combinations and writes
# the command outputs (including stdout/stderr) to a log so regression status can be reviewed.

# Detect GCC similar to run_tests.sh so that macOS still prefers brewed gcc-14/15.
detect_gcc() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        for candidate in gcc-15 gcc-14 gcc-13; do
            if command -v "$candidate" >/dev/null 2>&1; then
                echo "$candidate"
                return
            fi
        done
        echo "GCC not found on macOS. Install one of gcc-14/gcc-15 via Homebrew." >&2
        exit 1
    fi
    echo gcc
}

GCC="$(detect_gcc)"
PLUGIN=../ternary_plugin.so
LOG_FILE="phase34_coverage.log"

# Fresh log file per run so old diagnostics do not linger.
: > "$LOG_FILE"

log_phase() {
    printf "[%s] %s\n" "$(date -Iseconds)" "$1" | tee -a "$LOG_FILE"
}

run_target() {
    log_phase "Running: $1"
    shift
    "$@" 2>&1 | tee -a "$LOG_FILE"
}

log_phase "Phase 3/4 regression coverage started (GCC=$GCC)."

log_phase "Building plugin before coverage regression."
(
    cd ..
    make
)
cd tests

run_target "Phase 3 lowering (GIMPLE dump)" \
    "$GCC" -fplugin="$PLUGIN" -fplugin-arg-ternary_plugin-lower \
    -fplugin-arg-ternary_plugin-dump-gimple -I../include \
    -c test_ternary.c -o test_phase34_lower.o

run_target "Phase 3 literal/promotion coverage" \
    "$GCC" -fplugin="$PLUGIN" -fplugin-arg-ternary_plugin-types \
    -I../include -c test_literals.c -o test_phase34_literals.o

run_target "Phase 4 diagnostics (trace + dump)" \
    "$GCC" -fplugin="$PLUGIN" -fplugin-arg-ternary_plugin-lower \
    -fplugin-arg-ternary_plugin-trace -fplugin-arg-ternary_plugin-dump-gimple \
    -I../include -c test_ternary.c -o test_phase34_diag.o

log_phase "Phase 3/4 regression coverage completed."
