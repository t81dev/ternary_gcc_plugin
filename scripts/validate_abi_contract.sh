#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/abi-contract}"
CC_BIN="${CC:-cc}"

mkdir -p "$BUILD_DIR"

echo "[abi] helper documentation parity"
python3 "$ROOT_DIR/tools/check_helper_docs.py"

echo "[abi] compiling runtime and smoke tests with ${CC_BIN}"
"$CC_BIN" -std=c11 -I"$ROOT_DIR/include" -c "$ROOT_DIR/runtime/ternary_runtime.c" -o "$BUILD_DIR/ternary_runtime.o"
"$CC_BIN" -std=c11 -I"$ROOT_DIR/include" "$ROOT_DIR/tests/test_abi.c" "$BUILD_DIR/ternary_runtime.o" -o "$BUILD_DIR/test_abi"
"$CC_BIN" -std=c11 -I"$ROOT_DIR/include" "$ROOT_DIR/tests/test_varargs.c" "$BUILD_DIR/ternary_runtime.o" -o "$BUILD_DIR/test_varargs"

echo "[abi] running smoke binaries"
"$BUILD_DIR/test_abi"
"$BUILD_DIR/test_varargs"

echo "[abi] validation passed"
