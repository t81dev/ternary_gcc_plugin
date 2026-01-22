#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "ternary_runtime.h"

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
        fprintf(stderr, "FAIL %s: got %" PRId64 " expect %" PRId64 "\n", name, got, expect);
        fail_count++;
    }
}

static void expect_tv64_lane(const char *name, tv64_t vec, int lane, int64_t expect)
{
    t64_t lane_val = (lane == 0) ? vec.lo : vec.hi;
    int64_t got = __ternary_tt2b_t64(lane_val);
    if (got != expect) {
        fprintf(stderr, "FAIL %s lane %d: got %" PRId64 " expect %" PRId64 "\n",
                name, lane, got, expect);
        fail_count++;
    }
}

int main(void)
{
    t32_t neg = __ternary_tb2t_t32(-1);
    t32_t zero = __ternary_tb2t_t32(0);
    t32_t pos = __ternary_tb2t_t32(1);

    expect_int("tmin_neg_zero", __ternary_tt2b_t32(__ternary_tmin_t32(neg, zero)), -1);
    expect_int("tmin_zero_pos", __ternary_tt2b_t32(__ternary_tmin_t32(zero, pos)), 0);
    expect_int("tmax_zero_pos", __ternary_tt2b_t32(__ternary_tmax_t32(zero, pos)), 1);

    expect_int("tequiv_same", __ternary_tt2b_t32(__ternary_tequiv_t32(pos, pos)), 1);
    expect_int("tequiv_unknown", __ternary_tt2b_t32(__ternary_tequiv_t32(pos, zero)), 0);
    expect_int("tnet_simple", __ternary_tnet_t32(__ternary_tb2t_t32(2)), 2);
    expect_int("tmux_select", __ternary_tt2b_t32(__ternary_tmux_t32(__ternary_tb2t_t32(-1),
                                                                  __ternary_tb2t_t32(-4),
                                                                  __ternary_tb2t_t32(0),
                                                                  __ternary_tb2t_t32(5))),
               -4);

    expect_int("tmaj_pos", __ternary_tt2b_t32(__ternary_tmaj_t32(pos, pos, neg)), 1);
    expect_int("tmaj_all_diff", __ternary_tt2b_t32(__ternary_tmaj_t32(pos, zero, neg)), 0);

    expect_int("tlimp_false", __ternary_tt2b_t32(__ternary_tlimp_t32(neg, pos)), 1);
    expect_int("tlimp_unknown", __ternary_tt2b_t32(__ternary_tlimp_t32(zero, neg)), -1);
    expect_int("tlimp_true", __ternary_tt2b_t32(__ternary_tlimp_t32(pos, zero)), 0);

    expect_int("tquant_pos", __ternary_tt2b_t32(__ternary_tquant_t32(0.6f, 0.5f)), 1);
    expect_int("tquant_zero", __ternary_tt2b_t32(__ternary_tquant_t32(0.2f, 0.5f)), 0);

    expect_int("tmuladd", __ternary_tt2b_t32(__ternary_tmuladd_t32(__ternary_tb2t_t32(2),
                                                                 __ternary_tb2t_t32(3),
                                                                 __ternary_tb2t_t32(1))),
               7);

    expect_int("tnot", __ternary_tt2b_t32(__ternary_tnot_t32(__ternary_tb2t_t32(4))), -4);
    expect_int("tinv alias", __ternary_tt2b_t32(__ternary_tinv_t32(__ternary_tb2t_t32(-5))), 5);

    expect_int("tround", __ternary_tt2b_t32(__ternary_tround_t32(__ternary_tb2t_t32(8), 1)), 2);
    expect_int("tnormalize", __ternary_tt2b_t32(__ternary_tnormalize_t32(0xFFFFFFFFFFFFFFFFULL)), 0);
    expect_int("tbias", __ternary_tt2b_t32(__ternary_tbias_t32(__ternary_tb2t_t32(1), 3)), 4);

    expect_int("tbranch_neg", __ternary_tbranch(-1, -10, 0, 10), -10);
    expect_int("tbranch_zero", __ternary_tbranch(0, -10, 0, 10), 0);
    expect_int("tbranch_pos", __ternary_tbranch(1, -10, 0, 10), 10);

    expect_int("tsignjmp_neg", __ternary_tsignjmp_t32(__ternary_tb2t_t32(-2), -1, 0, 1), -1);
    expect_int("tsignjmp_zero", __ternary_tsignjmp_t64(__ternary_tb2t_t64(0), -1, 0, 1), 0);
    expect_int("tsignjmp_pos", __ternary_tsignjmp_t64(__ternary_tb2t_t64(3), -1, 0, 1), 1);

    expect_int("tequiv_t64_same", __ternary_tt2b_t64(__ternary_tequiv_t64(__ternary_tb2t_t64(1),
                                                                      __ternary_tb2t_t64(1))), 1);
    expect_int("txor_t64_diff", __ternary_tt2b_t64(__ternary_txor_t64(__ternary_tb2t_t64(1),
                                                                    __ternary_tb2t_t64(-1))), 0);
    expect_int("tnet_t64", __ternary_tnet_t64(__ternary_tb2t_t64(2)), 2);
    expect_int("tmux_t64_select", __ternary_tt2b_t64(__ternary_tmux_t64(__ternary_tb2t_t64(0),
                                                                      __ternary_tb2t_t64(-3),
                                                                      __ternary_tb2t_t64(0),
                                                                      __ternary_tb2t_t64(7))), 0);

#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
    expect_int("t128_tequiv_same", __ternary_tt2b_t128(__ternary_tequiv_t128(__ternary_tb2t_t128(2),
                                                                           __ternary_tb2t_t128(2))), 1);
    expect_int("t128_tequiv_unknown", __ternary_tt2b_t128(__ternary_tequiv_t128(__ternary_tb2t_t128(1),
                                                                              __ternary_tb2t_t128(0))), 0);
    expect_int("t128_txor_diff", __ternary_tt2b_t128(__ternary_txor_t128(__ternary_tb2t_t128(1),
                                                                       __ternary_tb2t_t128(0))), 1);
    expect_int("t128_txor_same", __ternary_tt2b_t128(__ternary_txor_t128(__ternary_tb2t_t128(1),
                                                                       __ternary_tb2t_t128(1))), -1);
    expect_int("t128_tnet", __ternary_tnet_t128(__ternary_tb2t_t128(3)), 3);
    expect_int("t128_tmux_pos", __ternary_tt2b_t128(__ternary_tmux_t128(__ternary_tb2t_t128(1),
                                                                       __ternary_tb2t_t128(-2),
                                                                       __ternary_tb2t_t128(0),
                                                                       __ternary_tb2t_t128(4))), 4);
#endif

    expect_int("tequiv_true", __ternary_tt2b_t32(__ternary_tequiv_t32(pos, pos)), 1);
    expect_int("tequiv_unknown", __ternary_tt2b_t32(__ternary_tequiv_t32(zero, pos)), 0);
    expect_int("tequiv_false", __ternary_tt2b_t32(__ternary_tequiv_t32(pos, neg)), -1);

    expect_int("txor_zero", __ternary_tt2b_t32(__ternary_txor_t32(pos, neg)), 0);
    expect_int("txor_diff", __ternary_tt2b_t32(__ternary_txor_t32(pos, zero)), 1);
    expect_int("txor_same", __ternary_tt2b_t32(__ternary_txor_t32(pos, pos)), -1);

    t32_t mux_lo = __ternary_tb2t_t32(1);
    t32_t mux_mid = __ternary_tb2t_t32(0);
    t32_t mux_hi = __ternary_tb2t_t32(-1);
    expect_int("tmux_neg", __ternary_tt2b_t32(__ternary_tmux_t32(__ternary_tb2t_t32(-1), mux_lo, mux_mid, mux_hi)), 1);
    expect_int("tmux_zero", __ternary_tt2b_t32(__ternary_tmux_t32(__ternary_tb2t_t32(0), mux_lo, mux_mid, mux_hi)), 0);
    expect_int("tmux_pos", __ternary_tt2b_t32(__ternary_tmux_t32(__ternary_tb2t_t32(1), mux_lo, mux_mid, mux_hi)), -1);

    expect_int("tnet_one", __ternary_tnet_t32(__ternary_tb2t_t32(1)), 1);
    expect_int("tnet_minus", __ternary_tnet_t32(__ternary_tb2t_t32(-1)), -1);

    tv64_t v_lo = { __ternary_tb2t_t64(1), __ternary_tb2t_t64(2) };
    tv64_t v_hi = { __ternary_tb2t_t64(3), __ternary_tb2t_t64(-1) };
    tv64_t added = __ternary_add_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_add_lo", added, 0,
                     __ternary_tt2b_t64(__ternary_add_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_add_hi", added, 1,
                     __ternary_tt2b_t64(__ternary_add_t64(v_lo.hi, v_hi.hi)));

    tv64_t subbed = __ternary_sub_tv64(v_hi, v_lo);
    expect_tv64_lane("tv64_sub_lo", subbed, 0,
                     __ternary_tt2b_t64(__ternary_sub_t64(v_hi.lo, v_lo.lo)));
    expect_tv64_lane("tv64_sub_hi", subbed, 1,
                     __ternary_tt2b_t64(__ternary_sub_t64(v_hi.hi, v_lo.hi)));

    tv64_t mul = __ternary_mul_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_mul_lo", mul, 0,
                     __ternary_tt2b_t64(__ternary_mul_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_mul_hi", mul, 1,
                     __ternary_tt2b_t64(__ternary_mul_t64(v_lo.hi, v_hi.hi)));

    tv64_t anded = __ternary_and_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_and_lo", anded, 0,
                     __ternary_tt2b_t64(__ternary_and_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_and_hi", anded, 1,
                     __ternary_tt2b_t64(__ternary_and_t64(v_lo.hi, v_hi.hi)));

    tv64_t ored = __ternary_or_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_or_lo", ored, 0,
                     __ternary_tt2b_t64(__ternary_or_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_or_hi", ored, 1,
                     __ternary_tt2b_t64(__ternary_or_t64(v_lo.hi, v_hi.hi)));

    tv64_t xored = __ternary_xor_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_xor_lo", xored, 0,
                     __ternary_tt2b_t64(__ternary_xor_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_xor_hi", xored, 1,
                     __ternary_tt2b_t64(__ternary_xor_t64(v_lo.hi, v_hi.hi)));

    tv64_t notted = __ternary_not_tv64(v_lo);
    expect_tv64_lane("tv64_not_lo", notted, 0,
                     __ternary_tt2b_t64(__ternary_not_t64(v_lo.lo)));
    expect_tv64_lane("tv64_not_hi", notted, 1,
                     __ternary_tt2b_t64(__ternary_not_t64(v_lo.hi)));

    tv64_t cmp = __ternary_cmp_tv64(v_lo, v_hi);
    expect_tv64_lane("tv64_cmp_lo", cmp, 0,
                     __ternary_tt2b_t64(__ternary_cmplt_t64(v_lo.lo, v_hi.lo)));
    expect_tv64_lane("tv64_cmp_hi", cmp, 1,
                     __ternary_tt2b_t64(__ternary_cmplt_t64(v_lo.hi, v_hi.hi)));


    if (fail_count == 0) {
        printf("tests/test_logic_helpers: ok\n");
        return 0;
    }
    fprintf(stderr, "tests/test_logic_helpers: %d failures\n", fail_count);
    return 1;
}
