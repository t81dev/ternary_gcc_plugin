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

    // Division/mod by zero should return 0.
    printf("Div by zero: %d\n", __ternary_div(5, 0));
    printf("Mod by zero: %d\n", __ternary_mod(5, 0));

    // Select
    int select_result = __ternary_select_i32(1, 10, 20);
    printf("Select true: %d\n", select_result);

    select_result = __ternary_select_i32(0, 10, 20);
    printf("Select false: %d\n", select_result);

    // Shift/rotate semantics on packed ternary types.
    // Note: t128 helpers are not implemented in the reference runtime.
    t32_t tval = __ternary_tb2t_t32(2);
    t32_t tval_shl = __ternary_shl_t32(tval, 1);
    t32_t tval_shr = __ternary_shr_t32(tval_shl, 1);
    printf("t32 shl/shr: %lld\n", (long long)__ternary_tt2b_t32(tval_shr));

    t32_t tval_top = __ternary_tb2t_t32(243); // 3^5
    t32_t tval_rol = __ternary_rol_t32(tval_top, 1);
    t32_t tval_ror = __ternary_ror_t32(tval_rol, 1);
    printf("t32 rol/ror: %lld\n", (long long)__ternary_tt2b_t32(tval_ror));

    t32_t tval_mask = __ternary_shl_t32(tval, 32);
    printf("t32 shl mask: %lld\n", (long long)__ternary_tt2b_t32(tval_mask));

    t64_t t64_a = __ternary_tb2t_t64(7);
    t64_t t64_div0 = __ternary_div_t64(t64_a, __ternary_tb2t_t64(0));
    printf("t64 div0: %lld\n", (long long)__ternary_tt2b_t64(t64_div0));
    printf("t64 shl mask: %lld\n", (long long)__ternary_tt2b_t64(__ternary_shl_t64(t64_a, 64)));

    printf("t32 cmp -1 vs +1: %d\n",
           __ternary_cmp_t32(__ternary_tb2t_t32(-1), __ternary_tb2t_t32(1)));

    // Conversion rounding toward zero.
    t32_t t_from_f = __ternary_f2t32_t32(-2.9f);
    printf("f2t trunc: %lld\n", (long long)__ternary_tt2b_t32(t_from_f));

    return 0;
}
