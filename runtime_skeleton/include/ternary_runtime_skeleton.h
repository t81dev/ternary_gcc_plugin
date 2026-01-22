#ifndef TERNARY_RUNTIME_SKELETON_H
#define TERNARY_RUNTIME_SKELETON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint64_t t32_t;           /* 32 trits -> 64 bits */
typedef unsigned __int128 t64_t;  /* 64 trits -> 128 bits */
#endif

#ifndef TERNARY_COND_T
typedef int64_t ternary_cond_t;
#define TERNARY_COND_T ternary_cond_t
#endif

#define TERNARY_RUNTIME_NAME_IMPL(prefix, name) prefix##name
#define TERNARY_RUNTIME_NAME(prefix, name) TERNARY_RUNTIME_NAME_IMPL(prefix, name)

#ifndef TERNARY_RUNTIME_PREFIX
/* Prefix used for exported helper symbols. */
#define TERNARY_RUNTIME_PREFIX __t81_ternary_
#endif

#define TERNARY_RUNTIME_SYM(name) TERNARY_RUNTIME_NAME(TERNARY_RUNTIME_PREFIX, name)

#define TERNARY_RUNTIME_DECLARE_SCALAR(prefix) \
    int TERNARY_RUNTIME_NAME(prefix, add)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, sub)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, mul)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, div)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, mod)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, neg)(int a); \
    int TERNARY_RUNTIME_NAME(prefix, not)(int a); \
    int TERNARY_RUNTIME_NAME(prefix, and)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, or)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, xor)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, shl)(int a, int shift); \
    int TERNARY_RUNTIME_NAME(prefix, shr)(int a, int shift); \
    int TERNARY_RUNTIME_NAME(prefix, rol)(int a, int shift); \
    int TERNARY_RUNTIME_NAME(prefix, ror)(int a, int shift); \
    int TERNARY_RUNTIME_NAME(prefix, cmp)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, eq)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, ne)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, lt)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, le)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, gt)(int a, int b); \
    int TERNARY_RUNTIME_NAME(prefix, ge)(int a, int b);

#define TERNARY_RUNTIME_DECLARE_T32(prefix) \
    t32_t TERNARY_RUNTIME_NAME(prefix, add_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, sub_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, mul_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, div_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, mod_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, neg_t32)(t32_t a); \
    t32_t TERNARY_RUNTIME_NAME(prefix, and_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, or_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, xor_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tmin_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tmax_t32)(t32_t a, t32_t b); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tmaj_t32)(t32_t a, t32_t b, t32_t c); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tlimp_t32)(t32_t antecedent, t32_t consequent); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tquant_t32)(float value, float threshold); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tnot_t32)(t32_t a); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tinv_t32)(t32_t a); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tmuladd_t32)(t32_t a, t32_t b, t32_t c); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tround_t32)(t32_t a, unsigned drop); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tnormalize_t32)(t32_t a); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tbias_t32)(t32_t a, int64_t bias); \
    t32_t TERNARY_RUNTIME_NAME(prefix, shl_t32)(t32_t a, int shift); \
    t32_t TERNARY_RUNTIME_NAME(prefix, shr_t32)(t32_t a, int shift); \
    t32_t TERNARY_RUNTIME_NAME(prefix, rol_t32)(t32_t a, int shift); \
    t32_t TERNARY_RUNTIME_NAME(prefix, ror_t32)(t32_t a, int shift); \
    t32_t TERNARY_RUNTIME_NAME(prefix, select_t32)(TERNARY_COND_T cond, t32_t t, t32_t f); \
    t32_t TERNARY_RUNTIME_NAME(prefix, tb2t_t32)(int64_t v); \
    int64_t TERNARY_RUNTIME_NAME(prefix, tt2b_t32)(t32_t v); \
    int TERNARY_RUNTIME_NAME(prefix, cmp_t32)(t32_t a, t32_t b);

#define TERNARY_RUNTIME_DECLARE_T64(prefix) \
    t64_t TERNARY_RUNTIME_NAME(prefix, add_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, sub_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, mul_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, div_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, mod_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, neg_t64)(t64_t a); \
    t64_t TERNARY_RUNTIME_NAME(prefix, and_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, or_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, xor_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tmin_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tmax_t64)(t64_t a, t64_t b); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tmaj_t64)(t64_t a, t64_t b, t64_t c); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tlimp_t64)(t64_t antecedent, t64_t consequent); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tquant_t64)(double value, double threshold); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tnot_t64)(t64_t a); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tinv_t64)(t64_t a); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tmuladd_t64)(t64_t a, t64_t b, t64_t c); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tround_t64)(t64_t a, unsigned drop); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tnormalize_t64)(t64_t a); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tbias_t64)(t64_t a, int64_t bias); \
    t64_t TERNARY_RUNTIME_NAME(prefix, shl_t64)(t64_t a, int shift); \
    t64_t TERNARY_RUNTIME_NAME(prefix, shr_t64)(t64_t a, int shift); \
    t64_t TERNARY_RUNTIME_NAME(prefix, rol_t64)(t64_t a, int shift); \
    t64_t TERNARY_RUNTIME_NAME(prefix, ror_t64)(t64_t a, int shift); \
    t64_t TERNARY_RUNTIME_NAME(prefix, select_t64)(TERNARY_COND_T cond, t64_t t, t64_t f); \
    t64_t TERNARY_RUNTIME_NAME(prefix, tb2t_t64)(int64_t v); \
    int64_t TERNARY_RUNTIME_NAME(prefix, tt2b_t64)(t64_t v); \
    int TERNARY_RUNTIME_NAME(prefix, cmp_t64)(t64_t a, t64_t b);

/* Scalar helpers (non-packed). */
TERNARY_RUNTIME_DECLARE_SCALAR(TERNARY_RUNTIME_PREFIX)

TERNARY_RUNTIME_DECLARE_T32(TERNARY_RUNTIME_PREFIX)
TERNARY_RUNTIME_DECLARE_T64(TERNARY_RUNTIME_PREFIX)

#define TERNARY_RUNTIME_DECLARE_BRANCH(prefix) \
    int TERNARY_RUNTIME_NAME(prefix, tbranch)(TERNARY_COND_T cond, int neg_target, int zero_target, int pos_target); \
    int TERNARY_RUNTIME_NAME(prefix, tsignjmp_t32)(t32_t reg, int neg_target, int zero_target, int pos_target); \
    int TERNARY_RUNTIME_NAME(prefix, tsignjmp_t64)(t64_t reg, int neg_target, int zero_target, int pos_target);

TERNARY_RUNTIME_DECLARE_BRANCH(TERNARY_RUNTIME_PREFIX)

#ifndef TERNARY_RUNTIME_NO_COMPAT
TERNARY_RUNTIME_DECLARE_SCALAR(__ternary_)
TERNARY_RUNTIME_DECLARE_T32(__ternary_)
TERNARY_RUNTIME_DECLARE_T64(__ternary_)
TERNARY_RUNTIME_DECLARE_BRANCH(__ternary_)
#endif

#undef TERNARY_RUNTIME_DECLARE_SCALAR
#undef TERNARY_RUNTIME_DECLARE_T32
#undef TERNARY_RUNTIME_DECLARE_T64

/* Need to ensure branch macros cleanup */
#undef TERNARY_RUNTIME_DECLARE_BRANCH

#ifdef __cplusplus
}
#endif

#endif /* TERNARY_RUNTIME_SKELETON_H */
