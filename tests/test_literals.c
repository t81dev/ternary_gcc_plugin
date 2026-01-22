#include "ternary.h"

int main(void)
{
    t32_t a = T32_BT_STR("1 0 -1 1");
    t64_t b = T64_BT_STR("1,0,+1,-1");

    __ternary(32) alias = a;
    __ternary(64) alias64 = b;

    int cond = __ternary_tt2b_t32(alias);
    float as_float = __ternary_t2f32_t32(alias);
    t32_t roundtrip = __ternary_f2t32_t32(as_float);

    return cond + (int)__ternary_tt2b_t32(roundtrip) + (int)alias + (int)alias64;
}
