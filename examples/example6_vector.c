#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

static inline tv32_t pack_tv32(t32_t lo, t32_t hi)
{
    return (tv32_t)lo | ((tv32_t)hi << 64);
}

static inline t32_t unpack_lo(tv32_t vec)
{
    return (t32_t)(vec & 0xFFFFFFFFFFFFFFFFULL);
}

static inline t32_t unpack_hi(tv32_t vec)
{
    return (t32_t)(vec >> 64);
}

int main(void)
{
    t32_t lo = __ternary_tb2t_t32(2);
    t32_t hi = __ternary_tb2t_t32(-3);
    tv32_t vec_a = pack_tv32(lo, hi);
    tv32_t vec_b = pack_tv32(hi, lo);

    tv32_t added = __ternary_add_tv32(vec_a, vec_b);
    tv32_t cmp = __ternary_cmp_tv32(vec_a, vec_b);

    printf("add_lo=%" PRId64 " add_hi=%" PRId64 "\n",
           __ternary_tt2b_t32(unpack_lo(added)),
           __ternary_tt2b_t32(unpack_hi(added)));
    printf("cmp_lo=%" PRId64 " cmp_hi=%" PRId64 "\n",
           __ternary_tt2b_t32(unpack_lo(cmp)),
           __ternary_tt2b_t32(unpack_hi(cmp)));
    return 0;
}
