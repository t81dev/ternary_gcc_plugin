/**
 * TMUX/TNET/TNN demo that exercises the extended helper ABI via the runtime skeleton exports.
 * Uses the `__t81_ternary_*` helpers that the skeleton exposes after macro expansion.
 */

#include <stdio.h>
#define TERNARY_RUNTIME_NO_COMPAT 1
#include "ternary_runtime_skeleton.h"

extern t32_t __t81_ternary_tb2t_t32(int64_t v);
extern int64_t __t81_ternary_tt2b_t32(t32_t v);
extern t32_t __t81_ternary_tequiv_t32(t32_t a, t32_t b);
extern t32_t __t81_ternary_txor_t32(t32_t a, t32_t b);
extern int __t81_ternary_tnet_t32(t32_t a);
extern t32_t __t81_ternary_tmux_t32(t32_t sel, t32_t neg, t32_t zero, t32_t pos);

extern t64_t __t81_ternary_tb2t_t64(int64_t v);
extern int64_t __t81_ternary_tt2b_t64(t64_t v);
extern t64_t __t81_ternary_tequiv_t64(t64_t a, t64_t b);
extern t64_t __t81_ternary_txor_t64(t64_t a, t64_t b);
extern int __t81_ternary_tnet_t64(t64_t a);
extern t64_t __t81_ternary_tmux_t64(t64_t sel, t64_t neg, t64_t zero, t64_t pos);

#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
extern t128_t __t81_ternary_tb2t_t128(int64_t v);
extern int64_t __t81_ternary_tt2b_t128(t128_t v);
extern t128_t __t81_ternary_tequiv_t128(t128_t a, t128_t b);
extern t128_t __t81_ternary_txor_t128(t128_t a, t128_t b);
extern int __t81_ternary_tnet_t128(t128_t a);
extern t128_t __t81_ternary_tmux_t128(t128_t sel, t128_t neg, t128_t zero, t128_t pos);
#endif

static int run_tnn32(void)
{
    t32_t features[3] = {
        __t81_ternary_tb2t_t32(1),
        __t81_ternary_tb2t_t32(-1),
        __t81_ternary_tb2t_t32(0),
    };
    t32_t weights[3] = {
        __t81_ternary_tb2t_t32(1),
        __t81_ternary_tb2t_t32(0),
        __t81_ternary_tb2t_t32(-1),
    };

    int net = 0;
    for (unsigned i = 0; i < 3; ++i) {
        t32_t similarity = __t81_ternary_tequiv_t32(features[i], weights[i]);
        t32_t delta = __t81_ternary_txor_t32(features[i], weights[i]);
        net += __t81_ternary_tnet_t32(similarity);
        net += __t81_ternary_tnet_t32(delta);
    }

    t32_t cond = __t81_ternary_tb2t_t32(net > 0 ? 1 : (net < 0 ? -1 : 0));
    t32_t chosen = __t81_ternary_tmux_t32(
        cond,
        __t81_ternary_tb2t_t32(-5),
        __t81_ternary_tb2t_t32(0),
        __t81_ternary_tb2t_t32(7));

    return __t81_ternary_tt2b_t32(chosen);
}

static int run_tnn64(void)
{
    t64_t features[3] = {
        __t81_ternary_tb2t_t64(2),
        __t81_ternary_tb2t_t64(-1),
        __t81_ternary_tb2t_t64(1),
    };
    t64_t weights[3] = {
        __t81_ternary_tb2t_t64(1),
        __t81_ternary_tb2t_t64(-1),
        __t81_ternary_tb2t_t64(0),
    };

    int net = 0;
    for (unsigned i = 0; i < 3; ++i) {
        t64_t similarity = __t81_ternary_tequiv_t64(features[i], weights[i]);
        t64_t delta = __t81_ternary_txor_t64(features[i], weights[i]);
        net += __t81_ternary_tnet_t64(similarity);
        net += __t81_ternary_tnet_t64(delta);
    }

    t64_t cond = __t81_ternary_tb2t_t64(net > 0 ? 1 : (net < 0 ? -1 : 0));
    t64_t chosen = __t81_ternary_tmux_t64(
        cond,
        __t81_ternary_tb2t_t64(-4),
        __t81_ternary_tb2t_t64(0),
        __t81_ternary_tb2t_t64(6));

    return __t81_ternary_tt2b_t64(chosen);
}

#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
static int run_tnn128(void)
{
    t128_t feature = __t81_ternary_tb2t_t128(1);
    t128_t weight = __t81_ternary_tb2t_t128(-1);

    t128_t similarity = __t81_ternary_tequiv_t128(feature, weight);
    t128_t diff = __t81_ternary_txor_t128(feature, weight);
    int net = __t81_ternary_tnet_t128(similarity) + __t81_ternary_tnet_t128(diff);

    t128_t cond = __t81_ternary_tb2t_t128(net > 0 ? 1 : (net < 0 ? -1 : 0));
    t128_t chosen = __t81_ternary_tmux_t128(
        cond,
        __t81_ternary_tb2t_t128(-3),
        __t81_ternary_tb2t_t128(0),
        __t81_ternary_tb2t_t128(5));

    return (int)__t81_ternary_tt2b_t128(chosen);
}
#endif

int main(void)
{
    printf("TNN demo t32 output: %d\n", run_tnn32());
    printf("TNN demo t64 output: %d\n", run_tnn64());
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
    printf("TNN demo t128 output: %d\n", run_tnn128());
#else
    puts("TNN demo t128 output: (skipped; _BitInt(256) unavailable)");
#endif
    return 0;
}
