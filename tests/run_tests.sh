#!/bin/bash

# Run plugin tests

GCC=/opt/homebrew/bin/gcc-15
PLUGIN=../ternary_plugin.so

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