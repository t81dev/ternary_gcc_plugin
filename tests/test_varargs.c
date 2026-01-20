#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "ternary_runtime.h"

static int64_t sum_t6(int count, ...)
{
    va_list ap;
    int64_t sum = 0;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) {
        t6_t v = TERNARY_VA_ARG_T6(ap);
        sum += __ternary_tt2b_t6(v);
    }
    va_end(ap);
    return sum;
}

static int64_t sum_t12(int count, ...)
{
    va_list ap;
    int64_t sum = 0;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) {
        t12_t v = TERNARY_VA_ARG_T12(ap);
        sum += __ternary_tt2b_t12(v);
    }
    va_end(ap);
    return sum;
}

static int64_t sum_t24(int count, ...)
{
    va_list ap;
    int64_t sum = 0;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) {
        t24_t v = TERNARY_VA_ARG_T24(ap);
        sum += __ternary_tt2b_t24(v);
    }
    va_end(ap);
    return sum;
}

static t6_t select_t6_varargs(int count, ...)
{
    (void)count;
    va_list ap;
    va_start(ap, count);
    t6_t cond = TERNARY_VA_ARG_T6(ap);
    t6_t a = TERNARY_VA_ARG_T6(ap);
    t6_t b = TERNARY_VA_ARG_T6(ap);
    va_end(ap);

    int64_t cond_val = __ternary_tt2b_t6(cond);
    return __ternary_select_t6((TERNARY_COND_T)cond_val, a, b);
}

int main(void)
{
    t6_t t6_a = __ternary_tb2t_t6(2);
    t6_t t6_b = __ternary_tb2t_t6(-1);
    t6_t t6_c = __ternary_tb2t_t6(3);
    int64_t t6_sum = sum_t6(3, t6_a, t6_b, t6_c);
    printf("t6 varargs sum: %lld\n", (long long)t6_sum);
    if (t6_sum != 4)
        return 1;

    t12_t t12_a = __ternary_tb2t_t12(5);
    t12_t t12_b = __ternary_tb2t_t12(-2);
    int64_t t12_sum = sum_t12(2, t12_a, t12_b);
    printf("t12 varargs sum: %lld\n", (long long)t12_sum);
    if (t12_sum != 3)
        return 1;

    t24_t t24_a = __ternary_tb2t_t24(7);
    t24_t t24_b = __ternary_tb2t_t24(0);
    t24_t t24_c = __ternary_tb2t_t24(-4);
    int64_t t24_sum = sum_t24(3, t24_a, t24_b, t24_c);
    printf("t24 varargs sum: %lld\n", (long long)t24_sum);
    if (t24_sum != 3)
        return 1;

    t6_t cond_true = __ternary_tb2t_t6(1);
    t6_t cond_false = __ternary_tb2t_t6(0);
    t6_t sel_true = select_t6_varargs(3, cond_true, t6_a, t6_b);
    t6_t sel_false = select_t6_varargs(3, cond_false, t6_a, t6_b);
    printf("t6 varargs select: %lld %lld\n",
           (long long)__ternary_tt2b_t6(sel_true),
           (long long)__ternary_tt2b_t6(sel_false));
    if (__ternary_tt2b_t6(sel_true) != 2 || __ternary_tt2b_t6(sel_false) != -1)
        return 1;

    return 0;
}
