/*
 * Minimal ternary example using t32_t and __ternary(32).
 *
 * Build:
 *   mkdir -p build
 *   cc -Iinclude -c runtime/ternary_runtime.c -o build/ternary_runtime.o
 *   cc -fplugin=./ternary_plugin.so \
 *     -fplugin-arg-ternary_plugin-types \
 *     -fplugin-arg-ternary_plugin-lower \
 *     -Iinclude -c examples/ternary_basic.c -o build/ternary_basic.o
 *   cc build/ternary_basic.o build/ternary_runtime.o -o build/ternary_basic
 *
 * Run:
 *   ./build/ternary_basic
 *
 * Expected lowered symbols in build/ternary_basic.o:
 *   __ternary_add_t32
 *   __ternary_select_t32
 *   __ternary_tt2b_t32
 */

#include <stdio.h>
#define TERNARY_USE_BUILTIN_TYPES
#include "ternary_plugin.h"

typedef __ternary(32) ternary32;

ternary32 ternary_add(ternary32 a, ternary32 b)
{
    return a + b;
}

int to_int(ternary32 v)
{
    return v;
}

ternary32 select_example(int cond, ternary32 t, ternary32 f)
{
    return cond ? t : f;
}

int main(void)
{
    ternary32 a = 1;
    ternary32 b = 2;
    ternary32 c = ternary_add(a, b);
    ternary32 d = select_example(1, c, b);
    int out = to_int(d);
    printf("value=%d\n", out);
    return 0;
}
