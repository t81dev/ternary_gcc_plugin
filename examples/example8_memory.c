#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t slot = __ternary_tb2t_t32(0);
    __ternary_store_t32(&slot, __ternary_tb2t_t32(7));
    t32_t loaded = __ternary_load_t32(&slot);

    printf("stored=%" PRId64 " loaded=%" PRId64 "\n",
           __ternary_tt2b_t32(slot),
           __ternary_tt2b_t32(loaded));
    return 0;
}
