#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
mkdir -p "${BUILD_DIR}"

cc -std=c11 -DTERMINARY_RUNTIME_NO_COMPAT -I runtime_skeleton/include \
    -c runtime_skeleton/src/ternary_runtime_skeleton.c \
    -o "${BUILD_DIR}/ternary_runtime_skeleton.o"

cc -std=c11 -DTERMINARY_RUNTIME_NO_COMPAT -I runtime_skeleton/include \
    runtime_skeleton/src/ternary_tnn_demo.c "${BUILD_DIR}/ternary_runtime_skeleton.o" \
    -o "${BUILD_DIR}/ternary_tnn_demo"

printf 'Running TMUX/TNET/TNN demo (%s)...\n' "${BUILD_DIR}/ternary_tnn_demo"
"${BUILD_DIR}/ternary_tnn_demo"
