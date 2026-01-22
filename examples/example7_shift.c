#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t base = __ternary_tb2t_t32(5);
    t32_t left = __ternary_shl_t32(base, 1);
    t32_t right = __ternary_shr_t32(base, 1);
    t32_t rol = __ternary_rol_t32(base, 2);
    t32_t ror = __ternary_ror_t32(base, 2);

    printf("shl=%" PRId64 " shr=%" PRId64 " rol=%" PRId64 " ror=%" PRId64 "\n",
           __ternary_tt2b_t32(left),
           __ternary_tt2b_t32(right),
           __ternary_tt2b_t32(rol),
           __ternary_tt2b_t32(ror));
    return 0;
}
