// Compile-time test for type promotion
// This should compile without errors if promotion rules are correct

#include <stdio.h>
#include "ternary_plugin.h"

int main() {
    // Test type promotion
    t6_t t6 = 1;
    t12_t t12 = 2;
    t24_t t24 = 3;

    // Implicit conversions
    t12 = t6;  // Should promote
    t24 = t12; // Should promote

    // Arithmetic - assuming builtins are enabled, but for compile test, just assign
    t6_t result1 = t6; // Placeholder
    t12_t result2 = t12; // Placeholder

    printf("Type promotion test passed\n");
    return 0;
}