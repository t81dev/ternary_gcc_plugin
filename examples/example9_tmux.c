#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t neg = __ternary_tb2t_t32(-1);
    t32_t zero = __ternary_tb2t_t32(0);
    t32_t pos = __ternary_tb2t_t32(1);

    t32_t selection = __ternary_tmux_t32(__ternary_tb2t_t32(1), neg, zero, pos);
    int64_t net = __ternary_tnet_t32(selection);

    printf("tmux selection=%" PRId64 "\n", __ternary_tt2b_t32(selection));
    printf("tnet(selection)=%" PRId64 "\n", net);
    return 0;
}
