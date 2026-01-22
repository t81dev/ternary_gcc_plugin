#include <stdio.h>
#include "include/ternary_runtime_skeleton.h"

static int fail_count = 0;

static void expect_int(const char *name, int got, int expect)
{
    if (got != expect) {
        fprintf(stderr, "FAIL %s: got %d expect %d\n", name, got, expect);
        fail_count++;
    }
}

static void expect_i64(const char *name, int64_t got, int64_t expect)
{
    if (got != expect) {
        fprintf(stderr, "FAIL %s: got %lld expect %lld\n", name,
                (long long)got, (long long)expect);
        fail_count++;
    }
}

static int manual_trit_min(int a, int b)
{
    return (a < b) ? a : b;
}

static int manual_trit_max(int a, int b)
{
    return (a > b) ? a : b;
}

static int manual_trit_majority(int a, int b, int c)
{
    if (a == b || a == c)
        return a;
    if (b == c)
        return b;
    return 0;
}

static int manual_decode_trit(uint64_t packed, unsigned idx)
{
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 3U);
    if (bits == 0U)
        return -1;
    if (bits == 1U)
        return 0;
    return 1;
}

static uint64_t manual_set_trit(uint64_t packed, unsigned idx, int trit)
{
    uint64_t mask = 0x3ULL << (2U * idx);
    unsigned bits = (trit < 0) ? 0U : (trit == 0 ? 1U : 2U);
    uint64_t new_bits = (uint64_t)bits << (2U * idx);
    return (packed & ~mask) | new_bits;
}

static t32_t manual_tmin_t32(t32_t a, t32_t b)
{
    t32_t out = 0;
    for (unsigned i = 0; i < 32; ++i) {
        int trit = manual_trit_min(manual_decode_trit(a, i), manual_decode_trit(b, i));
        out = manual_set_trit(out, i, trit);
    }
    return out;
}

static t32_t manual_tmax_t32(t32_t a, t32_t b)
{
    t32_t out = 0;
    for (unsigned i = 0; i < 32; ++i) {
        int trit = manual_trit_max(manual_decode_trit(a, i), manual_decode_trit(b, i));
        out = manual_set_trit(out, i, trit);
    }
    return out;
}

static t32_t manual_tmaj_t32(t32_t a, t32_t b, t32_t c)
{
    t32_t out = 0;
    for (unsigned i = 0; i < 32; ++i) {
        int trit = manual_trit_majority(manual_decode_trit(a, i),
                                        manual_decode_trit(b, i),
                                        manual_decode_trit(c, i));
        out = manual_set_trit(out, i, trit);
    }
    return out;
}

static t32_t manual_tlimp_t32(t32_t antecedent, t32_t consequent)
{
    t32_t out = 0;
    for (unsigned i = 0; i < 32; ++i) {
        int trit_a = manual_decode_trit(antecedent, i);
        int trit_b = manual_decode_trit(consequent, i);
        int result;
        if (trit_a == -1)
            result = 1;
        else if (trit_a == 0) {
            if (trit_b == -1)
                result = -1;
            else if (trit_b == 0)
                result = 0;
            else
                result = 1;
        } else {
            result = trit_b;
        }
        out = manual_set_trit(out, i, result);
    }
    return out;
}

static t32_t manual_tquant_t32(float value, float threshold)
{
    int trit = 0;
    if (value > threshold)
        trit = 1;
    else if (value < -threshold)
        trit = -1;

    t32_t out = 0;
    for (unsigned i = 0; i < 32; ++i)
        out = manual_set_trit(out, i, trit);
    return out;
}

int main(void)
{
    expect_int("add", TERNARY_RUNTIME_SYM(add)(2, -1), 1);
    expect_int("sub", TERNARY_RUNTIME_SYM(sub)(2, -1), 3);
    expect_int("mul", TERNARY_RUNTIME_SYM(mul)(2, -1), -2);
    expect_int("div", TERNARY_RUNTIME_SYM(div)(6, 3), 2);
    expect_int("mod", TERNARY_RUNTIME_SYM(mod)(7, 3), 1);
    expect_int("neg", TERNARY_RUNTIME_SYM(neg)(2), -2);
    expect_int("not", TERNARY_RUNTIME_SYM(not)(2), -2);
    expect_int("and", TERNARY_RUNTIME_SYM(and)(1, -1), -1);
    expect_int("or", TERNARY_RUNTIME_SYM(or)(1, -1), 1);
    expect_int("xor", TERNARY_RUNTIME_SYM(xor)(1, -1), 0);
    expect_int("cmp_lt", TERNARY_RUNTIME_SYM(cmp)(-1, 1), -1);
    expect_int("cmp_eq", TERNARY_RUNTIME_SYM(cmp)(1, 1), 0);
    expect_int("cmp_gt", TERNARY_RUNTIME_SYM(cmp)(2, 1), 1);
    expect_int("eq", __ternary_eq(1, 1), 1);
    expect_int("ne", __ternary_ne(1, 1), 0);
    expect_int("lt", __ternary_lt(0, 1), 1);
    expect_int("le", __ternary_le(1, 1), 1);
    expect_int("gt", __ternary_gt(2, 1), 1);
    expect_int("ge", __ternary_ge(2, 2), 1);

    t32_t t32_a = TERNARY_RUNTIME_SYM(tb2t_t32)(5);
    t32_t t32_b = TERNARY_RUNTIME_SYM(tb2t_t32)(-2);
    t32_t t32_c = TERNARY_RUNTIME_SYM(tb2t_t32)(0);
    t32_t t32_sum = TERNARY_RUNTIME_SYM(add_t32)(t32_a, t32_b);
    expect_i64("t32_sum", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_sum), 3);
    expect_i64("t32_neg", TERNARY_RUNTIME_SYM(tt2b_t32)(TERNARY_RUNTIME_SYM(neg_t32)(t32_a)), -5);
    expect_int("t32_cmp", TERNARY_RUNTIME_SYM(cmp_t32)(t32_a, t32_b), 1);

    t32_t t32_min = TERNARY_RUNTIME_SYM(tmin_t32)(t32_a, t32_b);
    t32_t expected_min = manual_tmin_t32(t32_a, t32_b);
    expect_i64("t32_tmin", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_min),
                TERNARY_RUNTIME_SYM(tt2b_t32)(expected_min));

    t32_t t32_max = TERNARY_RUNTIME_SYM(tmax_t32)(t32_a, t32_b);
    t32_t expected_max = manual_tmax_t32(t32_a, t32_b);
    expect_i64("t32_tmax", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_max),
                TERNARY_RUNTIME_SYM(tt2b_t32)(expected_max));

    t32_t t32_maj = TERNARY_RUNTIME_SYM(tmaj_t32)(t32_a, t32_b, t32_c);
    t32_t expected_maj = manual_tmaj_t32(t32_a, t32_b, t32_c);
    expect_i64("t32_tmaj", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_maj),
                TERNARY_RUNTIME_SYM(tt2b_t32)(expected_maj));

    t32_t t32_limp = TERNARY_RUNTIME_SYM(tlimp_t32)(t32_a, t32_b);
    t32_t expected_limp = manual_tlimp_t32(t32_a, t32_b);
    expect_i64("t32_tlimp", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_limp),
                TERNARY_RUNTIME_SYM(tt2b_t32)(expected_limp));

    t32_t t32_quant = TERNARY_RUNTIME_SYM(tquant_t32)(0.4f, 0.25f);
    t32_t expected_quant = manual_tquant_t32(0.4f, 0.25f);
    expect_i64("t32_tquant", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_quant),
                TERNARY_RUNTIME_SYM(tt2b_t32)(expected_quant));

    t64_t t64_a = TERNARY_RUNTIME_SYM(tb2t_t64)(7);
    t64_t t64_b = TERNARY_RUNTIME_SYM(tb2t_t64)(4);
    t64_t t64_mod = TERNARY_RUNTIME_SYM(mod_t64)(t64_a, t64_b);
    expect_i64("t64_mod", TERNARY_RUNTIME_SYM(tt2b_t64)(t64_mod), 3);
    expect_int("t64_cmp", TERNARY_RUNTIME_SYM(cmp_t64)(t64_b, t64_a), -1);

    if (fail_count == 0) {
        printf("runtime_skeleton: ok\n");
        return 0;
    }

    fprintf(stderr, "runtime_skeleton: %d failures\n", fail_count);
    return 1;
}
