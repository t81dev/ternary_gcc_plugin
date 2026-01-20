// Edge case tests for ternary operations
#include <stdio.h>
#include "ternary_runtime.h"

int main() {
    // Test overflow/underflow (balanced ternary wraps)
    int a = 1000000; // Large
    int b = -1000000;
    int add_result = __ternary_add(a, b);
    printf("Large add: %d + %d = %d\n", a, b, add_result);

    // Zero cases
    int zero_add = __ternary_add(0, 5);
    printf("Zero add: 0 + 5 = %d\n", zero_add);

    // Comparison
    int cmp_result = __ternary_cmp(5, 3);
    printf("Cmp 5 vs 3: %d\n", cmp_result);

    // Select
    int select_result = __ternary_select_i32(1, 10, 20);
    printf("Select true: %d\n", select_result);

    select_result = __ternary_select_i32(0, 10, 20);
    printf("Select false: %d\n", select_result);

    return 0;
}