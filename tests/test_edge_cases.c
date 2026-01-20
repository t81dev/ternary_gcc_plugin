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
    // Note: t48/t96/t192 helpers are not implemented in the reference runtime.
    t6_t tval = __ternary_tb2t_t6(2);
    t6_t tval_shl = __ternary_shl_t6(tval, 1);
    t6_t tval_shr = __ternary_shr_t6(tval_shl, 1);
    printf("t6 shl/shr: %lld\n", (long long)__ternary_tt2b_t6(tval_shr));

    t6_t tval_top = __ternary_tb2t_t6(243); // 3^5
    t6_t tval_rol = __ternary_rol_t6(tval_top, 1);
    t6_t tval_ror = __ternary_ror_t6(tval_rol, 1);
    printf("t6 rol/ror: %lld\n", (long long)__ternary_tt2b_t6(tval_ror));

    t6_t tval_mask = __ternary_shl_t6(tval, 6);
    printf("t6 shl mask: %lld\n", (long long)__ternary_tt2b_t6(tval_mask));

    t12_t t12_a = __ternary_tb2t_t12(7);
    t12_t t12_div0 = __ternary_div_t12(t12_a, __ternary_tb2t_t12(0));
    printf("t12 div0: %lld\n", (long long)__ternary_tt2b_t12(t12_div0));
    printf("t12 shl mask: %lld\n", (long long)__ternary_tt2b_t12(__ternary_shl_t12(t12_a, 12)));

    t24_t t24_a = __ternary_tb2t_t24(-9);
    t24_t t24_mod0 = __ternary_mod_t24(t24_a, __ternary_tb2t_t24(0));
    printf("t24 mod0: %lld\n", (long long)__ternary_tt2b_t24(t24_mod0));
    printf("t24 shr mask: %lld\n", (long long)__ternary_tt2b_t24(__ternary_shr_t24(t24_a, 24)));

    printf("t6 cmp -1 vs +1: %d\n",
           __ternary_cmp_t6(__ternary_tb2t_t6(-1), __ternary_tb2t_t6(1)));

    // Conversion rounding toward zero.
    t6_t t_from_f = __ternary_f2t32_t6(-2.9f);
    printf("f2t trunc: %lld\n", (long long)__ternary_tt2b_t6(t_from_f));

    return 0;
}
