#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "ternary_runtime.h"

static int64_t sum_t32(int count, ...)
{
    va_list ap;
    int64_t sum = 0;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) {
        t32_t v = TERNARY_VA_ARG_T32(ap);
        sum += __ternary_tt2b_t32(v);
    }
    va_end(ap);
    return sum;
}

static int64_t sum_t64(int count, ...)
{
    va_list ap;
    int64_t sum = 0;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) {
        t64_t v = TERNARY_VA_ARG_T64(ap);
        sum += __ternary_tt2b_t64(v);
    }
    va_end(ap);
    return sum;
}

static t32_t select_t32_varargs(int count, ...)
{
    (void)count;
    va_list ap;
    va_start(ap, count);
    t32_t cond = TERNARY_VA_ARG_T32(ap);
    t32_t a = TERNARY_VA_ARG_T32(ap);
    t32_t b = TERNARY_VA_ARG_T32(ap);
    va_end(ap);

    int64_t cond_val = __ternary_tt2b_t32(cond);
    return __ternary_select_t32((TERNARY_COND_T)cond_val, a, b);
}

int main(void)
{
    t32_t t32_a = __ternary_tb2t_t32(2);
    t32_t t32_b = __ternary_tb2t_t32(-1);
    t32_t t32_c = __ternary_tb2t_t32(3);
    int64_t t32_sum = sum_t32(3, t32_a, t32_b, t32_c);
    printf("t32 varargs sum: %lld\n", (long long)t32_sum);
    if (t32_sum != 4)
        return 1;

    t64_t t64_a = __ternary_tb2t_t64(5);
    t64_t t64_b = __ternary_tb2t_t64(-2);
    int64_t t64_sum = sum_t64(2, t64_a, t64_b);
    printf("t64 varargs sum: %lld\n", (long long)t64_sum);
    if (t64_sum != 3)
        return 1;

    t32_t cond_true = __ternary_tb2t_t32(1);
    t32_t cond_false = __ternary_tb2t_t32(0);
    t32_t sel_true = select_t32_varargs(3, cond_true, t32_a, t32_b);
    t32_t sel_false = select_t32_varargs(3, cond_false, t32_a, t32_b);
    printf("t32 varargs select: %lld %lld\n",
           (long long)__ternary_tt2b_t32(sel_true),
           (long long)__ternary_tt2b_t32(sel_false));
    if (__ternary_tt2b_t32(sel_true) != 2 || __ternary_tt2b_t32(sel_false) != -1)
        return 1;

    return 0;
}
