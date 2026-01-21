#!/bin/bash

# Run plugin tests

# Auto-detect GCC
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    if command -v gcc-15 >/dev/null 2>&1; then
        GCC=gcc-15
    elif command -v gcc-14 >/dev/null 2>&1; then
        GCC=gcc-14
    elif command -v gcc-13 >/dev/null 2>&1; then
        GCC=gcc-13
    else
        echo "GCC not found on macOS. Install with 'brew install gcc' and use gcc-14 or gcc-15."
        exit 1
    fi
else
    # Linux or other
    GCC=gcc
fi

PLUGIN=../ternary_plugin.so

echo "Using GCC: $GCC"
echo "Building plugin..."
cd .. && make && cd tests

echo "Testing lowering..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-lower -I../include -c ../test_ternary.c -o test_lower.o

echo "Testing arithmetic..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-arith -I../include -c ../test_ternary.c -o test_arith.o

echo "Testing logic..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-logic -I../include -c ../test_ternary.c -o test_logic.o

echo "Testing comparisons..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-cmp -I../include -c ../test_ternary.c -o test_cmp.o

echo "Testing shifts..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-shift -I../include -c ../test_ternary.c -o test_shift.o

echo "Testing conversions..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-conv -I../include -c ../test_ternary.c -o test_conv.o

echo "Testing types..."
$GCC -fplugin=$PLUGIN -fplugin-arg-ternary_plugin-types -I../include -c ../test_ternary.c -o test_types.o

echo "All plugin tests compiled successfully."