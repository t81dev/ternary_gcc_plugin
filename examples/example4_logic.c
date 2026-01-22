#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t mid = __ternary_tb2t_t32(0);
    t32_t up = __ternary_tb2t_t32(3);
    t32_t down = __ternary_tb2t_t32(-3);

    t32_t tmin = __ternary_tmin_t32(up, down);
    t32_t tmax = __ternary_tmax_t32(up, mid);
    t32_t tequiv = __ternary_tequiv_t32(up, up);
    t32_t txor = __ternary_txor_t32(up, mid);

    printf("tmin=%" PRId64 " tmax=%" PRId64 "\n", __ternary_tt2b_t32(tmin), __ternary_tt2b_t32(tmax));
    printf("tequiv=%" PRId64 " txor=%" PRId64 "\n", __ternary_tt2b_t32(tequiv), __ternary_tt2b_t32(txor));
    return 0;
}
