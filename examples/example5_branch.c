#include <stdio.h>
#include "ternary_runtime.h"

int main(void)
{
    ternary_cond_t cond_pos = __ternary_tt2b_t32(__ternary_tb2t_t32(1));
    ternary_cond_t cond_zero = __ternary_tt2b_t32(__ternary_tb2t_t32(0));
    t32_t shifted = __ternary_shl_t32(__ternary_tb2t_t32(2), 1);

    int target_pos = __ternary_tbranch(cond_pos, -1, 0, 1);
    int target_zero = __ternary_tbranch(cond_zero, -1, 0, 1);
    int sjmp = __ternary_tsignjmp_t32(shifted, -5, 0, 5);

    printf("branch(pos) target=%d\n", target_pos);
    printf("branch(zero) target=%d\n", target_zero);
    printf("signjmp(shifted) target=%d\n", sjmp);
    return 0;
}
