#include <stdio.h>
#include "ternary_runtime.h"

int main(void)
{
    ternary_cond_t cond = __ternary_tt2b_t32(__ternary_tb2t_t32(1));
    int result_true = __ternary_select_i32(cond, 42, -42);

    cond = __ternary_tt2b_t32(__ternary_tb2t_t32(0));
    int result_false = __ternary_select_i32(cond, 42, -42);

    printf("select(true) -> %d\n", result_true);
    printf("select(false) -> %d\n", result_false);
    return 0;
}
