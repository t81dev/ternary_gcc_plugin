#!/usr/bin/env bash
set -euo pipefail

detect_gcc() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        for candidate in gcc-15 gcc-14 gcc-13; do
            if command -v "$candidate" >/dev/null 2>&1; then
                echo "$candidate"
                return
            fi
        done
        echo "GCC not found on macOS. Install Homebrew gcc-14/15." >&2
        exit 1
    fi
    echo gcc
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

GCC=$(detect_gcc)
PLUGIN=./ternary_plugin.so
BUILD_DIR=build/examples
mkdir -p "$BUILD_DIR"

echo "Using GCC: $GCC"
echo "Building plugin..."
make

RUNTIME_OBJ="$BUILD_DIR/ternary_runtime.o"
"$GCC" -Iinclude -c runtime/ternary_runtime.c -o "$RUNTIME_OBJ"

declare -a EXAMPLES=(
    "example1_select:examples/example1_select.c:-fplugin-arg-ternary_plugin-lower"
    "example2_conversion:examples/example2_conversion.c:-fplugin-arg-ternary_plugin-conv"
    "example3_arithmetic:examples/example3_arithmetic.c:-fplugin-arg-ternary_plugin-arith"
    "example4_logic:examples/example4_logic.c:-fplugin-arg-ternary_plugin-logic"
    "example5_branch:examples/example5_branch.c:-fplugin-arg-ternary_plugin-lower -fplugin-arg-ternary_plugin-shift"
    "example6_vector:examples/example6_vector.c:-fplugin-arg-ternary_plugin-vector"
    "example7_shift:examples/example7_shift.c:-fplugin-arg-ternary_plugin-shift"
    "example8_memory:examples/example8_memory.c:-fplugin-arg-ternary_plugin-mem"
    "example9_tmux:examples/example9_tmux.c:-fplugin-arg-ternary_plugin-logic -fplugin-arg-ternary_plugin-arith"
)

for entry in "${EXAMPLES[@]}"; do
    IFS=":" read -r name source flags <<< "$entry"
    binary="$BUILD_DIR/$name"

    echo "Building $name..."
    $GCC -fplugin="$PLUGIN" $flags -Iinclude "$source" "$RUNTIME_OBJ" -o "$binary"

    echo "Running $name..."
    "$binary"
done

echo "Phase 10: capturing Phase 3/4 diagnostics..."
./tests/run_phase34_coverage.sh
