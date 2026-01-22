#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t a = __ternary_tb2t_t32(2);
    t32_t b = __ternary_tb2t_t32(3);
    t32_t c = __ternary_tb2t_t32(-1);

    t32_t sum = __ternary_add_t32(a, b);
    t32_t prod = __ternary_mul_t32(sum, b);
    t32_t fused = __ternary_tmuladd_t32(a, b, c);
    t32_t rounded = __ternary_tround_t32(prod, 1);

    printf("sum=%" PRId64 "\n", __ternary_tt2b_t32(sum));
    printf("product=%" PRId64 "\n", __ternary_tt2b_t32(prod));
    printf("muladd=%" PRId64 "\n", __ternary_tt2b_t32(fused));
    printf("rounded(product)=%" PRId64 "\n", __ternary_tt2b_t32(rounded));
    return 0;
}
