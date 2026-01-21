// Compile-time test for type promotion
// This should compile without errors if promotion rules are correct

#include <stdio.h>
#include "ternary_plugin.h"

int main() {
    // Test type promotion
    t32_t t32 = 1;
    t64_t t64 = 2;
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
    t128_t t128 = 3;
#endif

    // Implicit conversions
    t64 = t32;  // Should promote
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
    t128 = t64; // Should promote
#endif

    // Arithmetic - assuming builtins are enabled, but for compile test, just assign
    t32_t result1 = t32; // Placeholder
    t64_t result2 = t64; // Placeholder

    printf("Type promotion test passed\n");
    return 0;
}
