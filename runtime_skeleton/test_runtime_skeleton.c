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
    t32_t t32_sum = TERNARY_RUNTIME_SYM(add_t32)(t32_a, t32_b);
    expect_i64("t32_sum", TERNARY_RUNTIME_SYM(tt2b_t32)(t32_sum), 3);
    expect_i64("t32_neg", TERNARY_RUNTIME_SYM(tt2b_t32)(TERNARY_RUNTIME_SYM(neg_t32)(t32_a)), -5);
    expect_int("t32_cmp", TERNARY_RUNTIME_SYM(cmp_t32)(t32_a, t32_b), 1);

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
