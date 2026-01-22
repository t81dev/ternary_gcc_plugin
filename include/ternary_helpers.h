#ifndef TERNARY_HELPERS_H
#define TERNARY_HELPERS_H

/* Helper functions for a balanced-ternary ISA.
 * These are example implementations using placeholder ISA instructions (tsel/tadd/tmul/tnot).
 * Adjust the assembly to match your actual ISA and calling convention.
 *
 * The Setun ISA and modern REBEL-6 work inspired the helpers below. When possible prefer
 * the ternary-only ops listed in MASTER_ISA.md (three-valued implication, majority voting,
 * TMIN/TMAX, TQUANT, TMULADD, and symmetric rounding) instead of emulating them with
 * binary sequences. These helpers stay compatible with the GCC plugin ABI and are intended
 * to survive into hardware-friendly implementations.
 */

#include <stdint.h>
#include <stdarg.h>
#define TERNARY_PLUGIN_SKIP_BT_STR
#include "ternary_plugin.h"
#undef TERNARY_PLUGIN_SKIP_BT_STR

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TERNARY_COND_T
typedef int64_t ternary_cond_t;
#define TERNARY_COND_T ternary_cond_t
#endif

/* TERNARY_COND_T defines the ABI condition type for select helpers.
 * The plugin lowers conditions to this type before helper calls.
 * Non-zero values are treated as true by the select helpers.
 */

/* Define TERNARY_USE_BUILTIN_TYPES to use plugin-provided ternary types (t32_t, t64_t, etc.)
 * and avoid typedef conflicts.
 */

/* Packed ternary encoding: 2 bits per trit.
 * Encoding: 00 = -1, 01 = 0, 10 = +1, 11 = reserved.
 */
static inline unsigned ternary_trit_to_bits(int trit) {
    return (trit < 0) ? 0U : (trit == 0 ? 1U : 2U);
}

static inline int ternary_bits_to_trit(unsigned bits) {
    if (bits == 0U) return -1;
    if (bits == 1U) return 0;
    return 1;
}

static inline int64_t ternary_decode(uint64_t packed, unsigned trit_count) {
    int64_t value = 0;
    int64_t pow3 = 1;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        int trit = ternary_bits_to_trit(bits);
        value += (int64_t)trit * pow3;
        pow3 *= 3;
    }
    return value;
}

static inline uint64_t ternary_encode(int64_t value, unsigned trit_count) {
    uint64_t packed = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int64_t rem = value % 3;
        value /= 3;

        if (rem == 2) {
            rem = -1;
            value += 1;
        } else if (rem == -2) {
            rem = 1;
            value -= 1;
        }

        unsigned bits = ternary_trit_to_bits((int)rem);
        packed |= ((uint64_t)bits) << (2U * i);
    }
    return packed;
}

static inline int ternary_trit_min(int a, int b) {
    return (a < b) ? a : b;
}

static inline int ternary_trit_max(int a, int b) {
    return (a > b) ? a : b;
}

static inline int ternary_trit_xor(int a, int b) {
    int mn = ternary_trit_min(a, b);
    int mx = ternary_trit_max(a, b);
    return a + b - 2 * mn - 2 * mx;
}

static inline int ternary_get_trit(uint64_t packed, unsigned idx) {
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 0x3U);
    return ternary_bits_to_trit(bits);
}

static inline uint64_t ternary_set_trit(uint64_t packed, unsigned idx, int trit) {
    uint64_t mask = 0x3ULL << (2U * idx);
    uint64_t bits = (uint64_t)ternary_trit_to_bits(trit) << (2U * idx);
    return (packed & ~mask) | bits;
}

static inline uint64_t ternary_tritwise_op(uint64_t a, uint64_t b, unsigned trit_count, int op) {
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int ta = ternary_get_trit(a, i);
        int tb = ternary_get_trit(b, i);
        int trit = 0;
        if (op == 0)
            trit = ternary_trit_min(ta, tb);
        else if (op == 1)
            trit = ternary_trit_max(ta, tb);
        else
            trit = ternary_trit_xor(ta, tb);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static inline uint64_t ternary_shift_left(uint64_t packed, unsigned trit_count, unsigned shift) {
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = 0;
        if (i >= shift)
            trit = ternary_get_trit(packed, i - shift);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static inline uint64_t ternary_shift_right(uint64_t packed, unsigned trit_count, unsigned shift) {
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    int sign_trit = ternary_get_trit(packed, trit_count - 1);
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = sign_trit;
        if (i + shift < trit_count)
            trit = ternary_get_trit(packed, i + shift);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static inline uint64_t ternary_rotate_left(uint64_t packed, unsigned trit_count, unsigned shift) {
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + trit_count - shift) % trit_count;
        int trit = ternary_get_trit(packed, src);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static inline uint64_t ternary_rotate_right(uint64_t packed, unsigned trit_count, unsigned shift) {
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + shift) % trit_count;
        int trit = ternary_get_trit(packed, src);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static inline int64_t ternary_decode_u128(unsigned __int128 packed, unsigned trit_count) {
    int64_t value = 0;
    int64_t pow3 = 1;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        int trit = ternary_bits_to_trit(bits);
        value += (int64_t)trit * pow3;
        pow3 *= 3;
    }
    return value;
}

static inline unsigned __int128 ternary_encode_u128(int64_t value, unsigned trit_count) {
    unsigned __int128 packed = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int64_t rem = value % 3;
        value /= 3;

        if (rem == 2) {
            rem = -1;
            value += 1;
        } else if (rem == -2) {
            rem = 1;
            value -= 1;
        }

        unsigned bits = ternary_trit_to_bits((int)rem);
        packed |= ((unsigned __int128)bits) << (2U * i);
    }
    return packed;
}

static inline int ternary_parse_bt_str(const char *s, int64_t *out) {
    int64_t acc = 0;
    int saw = 0;

    while (s && *s) {
        char c = *s;
        int trit = 0;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f') {
            ++s;
            continue;
        }
        if (c == ',') {
            ++s;
            continue;
        }
        if (c == '0') {
            trit = 0;
            ++s;
        } else if (c == '1') {
            trit = 1;
            ++s;
        } else if (c == '+' && s[1] == '1') {
            trit = 1;
            s += 2;
        } else if (c == '-' && s[1] == '1') {
            trit = -1;
            s += 2;
        } else {
            return 0;
        }

        saw = 1;
        if (trit >= 0) {
            if (acc > (INT64_MAX - trit) / 3)
                return 0;
        } else {
            if (acc < (INT64_MIN - trit) / 3)
                return 0;
        }
        acc = acc * 3 + trit;
    }

    if (!saw)
        return 0;
    *out = acc;
    return 1;
}

static inline int ternary_get_trit_u128(unsigned __int128 packed, unsigned idx) {
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 0x3U);
    return ternary_bits_to_trit(bits);
}

static inline unsigned __int128 ternary_set_trit_u128(unsigned __int128 packed, unsigned idx, int trit) {
    unsigned __int128 mask = (unsigned __int128)0x3U << (2U * idx);
    unsigned __int128 bits = (unsigned __int128)ternary_trit_to_bits(trit) << (2U * idx);
    return (packed & ~mask) | bits;
}

static inline unsigned __int128 ternary_tritwise_op_u128(unsigned __int128 a, unsigned __int128 b,
                                                         unsigned trit_count, int op) {
    unsigned __int128 out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int ta = ternary_get_trit_u128(a, i);
        int tb = ternary_get_trit_u128(b, i);
        int trit = 0;
        if (op == 0)
            trit = ternary_trit_min(ta, tb);
        else if (op == 1)
            trit = ternary_trit_max(ta, tb);
        else
            trit = ternary_trit_xor(ta, tb);
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static inline unsigned __int128 ternary_shift_left_u128(unsigned __int128 packed, unsigned trit_count,
                                                        unsigned shift) {
    unsigned __int128 out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = 0;
        if (i >= shift)
            trit = ternary_get_trit_u128(packed, i - shift);
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static inline unsigned __int128 ternary_shift_right_u128(unsigned __int128 packed, unsigned trit_count,
                                                         unsigned shift) {
    unsigned __int128 out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    int sign_trit = ternary_get_trit_u128(packed, trit_count - 1);
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = sign_trit;
        if (i + shift < trit_count)
            trit = ternary_get_trit_u128(packed, i + shift);
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static inline unsigned __int128 ternary_rotate_left_u128(unsigned __int128 packed, unsigned trit_count,
                                                         unsigned shift) {
    unsigned __int128 out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + trit_count - shift) % trit_count;
        int trit = ternary_get_trit_u128(packed, src);
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static inline unsigned __int128 ternary_rotate_right_u128(unsigned __int128 packed, unsigned trit_count,
                                                          unsigned shift) {
    unsigned __int128 out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + shift) % trit_count;
        int trit = ternary_get_trit_u128(packed, src);
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

/* Signed integer selects */
static inline int __ternary_select_i8(TERNARY_COND_T cond, int true_val, int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline int __ternary_select_i16(TERNARY_COND_T cond, int true_val, int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline int __ternary_select_i32(TERNARY_COND_T cond, int true_val, int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline long long __ternary_select_i64(TERNARY_COND_T cond, long long true_val, long long false_val) {
#ifdef TERNARY_USE_ISA_ASM
    long long result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

/* Unsigned integer selects */
static inline unsigned int __ternary_select_u8(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    unsigned int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline unsigned int __ternary_select_u16(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    unsigned int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline unsigned int __ternary_select_u32(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val) {
#ifdef TERNARY_USE_ISA_ASM
    unsigned int result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline unsigned long long __ternary_select_u64(TERNARY_COND_T cond, unsigned long long true_val, unsigned long long false_val) {
#ifdef TERNARY_USE_ISA_ASM
    unsigned long long result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

/* Floating point selects */
static inline float __ternary_select_f32(TERNARY_COND_T cond, float true_val, float false_val) {
#ifdef TERNARY_USE_ISA_ASM
    float result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

static inline double __ternary_select_f64(TERNARY_COND_T cond, double true_val, double false_val) {
#ifdef TERNARY_USE_ISA_ASM
    double result;
    __asm__ volatile (
        "tsel %0, %1, %2, %3"
        : "=r" (result)
        : "r" (cond), "r" (true_val), "r" (false_val)
    );
    return result;
#else
    return cond ? true_val : false_val;
#endif
}

/* Ternary type definitions using packed 2-bit trits. */
#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint64_t t32_t;           /* 32 trits -> 64 bits */
typedef unsigned __int128 t64_t;  /* 64 trits -> 128 bits */
#endif

/* Varargs helpers for ternary packed types. */
#define TERNARY_VA_ARG_T32(ap) ((t32_t)va_arg(ap, uint64_t))
#define TERNARY_VA_ARG_T64(ap) ((t64_t)va_arg(ap, unsigned __int128))

/* Note: t128 helpers are not provided in this header. */

/* Ternary type select operations */

/* t32 selects */
static inline t32_t __ternary_select_t32(TERNARY_COND_T cond, t32_t true_val, t32_t false_val) {
    return cond ? true_val : false_val;
}

/* t64 selects */
static inline t64_t __ternary_select_t64(TERNARY_COND_T cond, t64_t true_val, t64_t false_val) {
    return cond ? true_val : false_val;
}

/* Ternary arithmetic operations (ISA implementation or C fallback). */
static inline int __ternary_add(int a, int b) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tadd %0, %1, %2"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    return result;
#else
    return a + b;
#endif
}

static inline int __ternary_mul(int a, int b) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tmul %0, %1, %2"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    return result;
#else
    return a * b;
#endif
}

static inline int __ternary_not(int a) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tnot %0, %1"
        : "=r" (result)
        : "r" (a)
    );
    return result;
#else
    return -a;
#endif
}

static inline int __ternary_and(int a, int b) {
    return (a < b) ? a : b;
}

static inline int __ternary_or(int a, int b) {
    return (a > b) ? a : b;
}

static inline int __ternary_xor(int a, int b) {
    return ternary_trit_xor(a, b);
}

/* Extended ternary arithmetic operations */
static inline int __ternary_sub(int a, int b) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tsub %0, %1, %2"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    return result;
#else
    return a - b;
#endif
}

static inline int __ternary_div(int a, int b) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tdiv %0, %1, %2"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    return result;
#else
    return (b == 0) ? 0 : (a / b);
#endif
}

static inline int __ternary_mod(int a, int b) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tmod %0, %1, %2"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    return result;
#else
    return (b == 0) ? 0 : (a % b);
#endif
}

static inline int __ternary_neg(int a) {
#ifdef TERNARY_USE_ISA_ASM
    int result;
    __asm__ volatile (
        "tneg %0, %1"
        : "=r" (result)
        : "r" (a)
    );
    return result;
#else
    return -a;
#endif
}

static inline int __ternary_shl(int a, int shift) {
    int64_t value = a;
    int64_t pow3 = 1;
    for (int i = 0; i < shift; ++i)
        pow3 *= 3;
    return (int)(value * pow3);
}

static inline int __ternary_shr(int a, int shift) {
    int64_t value = a;
    int64_t pow3 = 1;
    for (int i = 0; i < shift; ++i)
        pow3 *= 3;
    return (int)(value / pow3);
}

static inline int __ternary_rol(int a, int shift) {
    (void)shift;
    return a;
}

static inline int __ternary_ror(int a, int shift) {
    (void)shift;
    return a;
}

/* Ternary type arithmetic operations (t32, t64, etc.) */
/* Note: These use the same underlying representation but with ternary semantics */

static inline t32_t __ternary_add_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    return (t32_t)ternary_encode(va + vb, 32);
}

static inline t32_t __ternary_mul_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    return (t32_t)ternary_encode(va * vb, 32);
}

static inline t32_t __ternary_not_t32(t32_t a) {
    int64_t va = ternary_decode(a, 32);
    return (t32_t)ternary_encode(-va, 32);
}

static inline t32_t __ternary_sub_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    return (t32_t)ternary_encode(va - vb, 32);
}

static inline t32_t __ternary_div_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    return (t32_t)ternary_encode(vb == 0 ? 0 : va / vb, 32);
}

static inline t32_t __ternary_mod_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    return (t32_t)ternary_encode(vb == 0 ? 0 : va % vb, 32);
}

static inline t32_t __ternary_neg_t32(t32_t a) {
    int64_t va = ternary_decode(a, 32);
    return (t32_t)ternary_encode(-va, 32);
}

static inline t32_t __ternary_and_t32(t32_t a, t32_t b) {
    return (t32_t)ternary_tritwise_op(a, b, 32, 0);
}

static inline t32_t __ternary_or_t32(t32_t a, t32_t b) {
    return (t32_t)ternary_tritwise_op(a, b, 32, 1);
}

static inline t32_t __ternary_xor_t32(t32_t a, t32_t b) {
    return (t32_t)ternary_tritwise_op(a, b, 32, 2);
}

static inline t32_t __ternary_shl_t32(t32_t a, int shift) {
    return (t32_t)ternary_shift_left(a, 32, (unsigned)shift);
}

static inline t32_t __ternary_shr_t32(t32_t a, int shift) {
    return (t32_t)ternary_shift_right(a, 32, (unsigned)shift);
}

static inline t32_t __ternary_rol_t32(t32_t a, int shift) {
    return (t32_t)ternary_rotate_left(a, 32, (unsigned)shift);
}

static inline t32_t __ternary_ror_t32(t32_t a, int shift) {
    return (t32_t)ternary_rotate_right(a, 32, (unsigned)shift);
}

static inline t32_t __ternary_tb2t_t32(int64_t v) {
    return (t32_t)ternary_encode(v, 32);
}

static inline int64_t __ternary_tt2b_t32(t32_t v) {
    return ternary_decode(v, 32);
}

static inline float __ternary_t2f32_t32(t32_t v) {
    return (float)ternary_decode(v, 32);
}

static inline double __ternary_t2f64_t32(t32_t v) {
    return (double)ternary_decode(v, 32);
}

static inline t32_t __ternary_f2t32_t32(float v) {
    return (t32_t)ternary_encode((int64_t)v, 32);
}

static inline t32_t __ternary_f2t64_t32(double v) {
    return (t32_t)ternary_encode((int64_t)v, 32);
}

static inline t32_t __ternary_bt_str_t32(const char *s) {
    int64_t value = 0;
    if (!ternary_parse_bt_str(s, &value))
        return 0;
    return (t32_t)ternary_encode(value, 32);
}

static inline t64_t __ternary_add_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va + vb, 64);
}

static inline t64_t __ternary_mul_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va * vb, 64);
}

static inline t64_t __ternary_not_t64(t64_t a) {
    int64_t va = ternary_decode_u128(a, 64);
    return (t64_t)ternary_encode_u128(-va, 64);
}

static inline t64_t __ternary_sub_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va - vb, 64);
}

static inline t64_t __ternary_div_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(vb == 0 ? 0 : va / vb, 64);
}

static inline t64_t __ternary_mod_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(vb == 0 ? 0 : va % vb, 64);
}

static inline t64_t __ternary_neg_t64(t64_t a) {
    int64_t va = ternary_decode_u128(a, 64);
    return (t64_t)ternary_encode_u128(-va, 64);
}

static inline t64_t __ternary_and_t64(t64_t a, t64_t b) {
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 0);
}

static inline t64_t __ternary_or_t64(t64_t a, t64_t b) {
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 1);
}

static inline t64_t __ternary_xor_t64(t64_t a, t64_t b) {
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 2);
}

static inline t64_t __ternary_shl_t64(t64_t a, int shift) {
    return (t64_t)ternary_shift_left_u128(a, 64, (unsigned)shift);
}

static inline t64_t __ternary_shr_t64(t64_t a, int shift) {
    return (t64_t)ternary_shift_right_u128(a, 64, (unsigned)shift);
}

static inline t64_t __ternary_rol_t64(t64_t a, int shift) {
    return (t64_t)ternary_rotate_left_u128(a, 64, (unsigned)shift);
}

static inline t64_t __ternary_ror_t64(t64_t a, int shift) {
    return (t64_t)ternary_rotate_right_u128(a, 64, (unsigned)shift);
}

static inline t64_t __ternary_tb2t_t64(int64_t v) {
    return (t64_t)ternary_encode_u128(v, 64);
}

static inline int64_t __ternary_tt2b_t64(t64_t v) {
    return ternary_decode_u128(v, 64);
}

static inline float __ternary_t2f32_t64(t64_t v) {
    return (float)ternary_decode_u128(v, 64);
}

static inline double __ternary_t2f64_t64(t64_t v) {
    return (double)ternary_decode_u128(v, 64);
}

static inline t64_t __ternary_f2t32_t64(float v) {
    return (t64_t)ternary_encode_u128((int64_t)v, 64);
}

static inline t64_t __ternary_f2t64_t64(double v) {
    return (t64_t)ternary_encode_u128((int64_t)v, 64);
}

static inline t64_t __ternary_bt_str_t64(const char *s) {
    int64_t value = 0;
    if (!ternary_parse_bt_str(s, &value))
        return 0;
    return (t64_t)ternary_encode_u128(value, 64);
}

/* Ternary comparison operations (return -1, 0, +1) */
static inline int __ternary_cmp(int a, int b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static inline int __ternary_cmp_t32(t32_t a, t32_t b) {
    int64_t va = ternary_decode(a, 32);
    int64_t vb = ternary_decode(b, 32);
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

static inline int __ternary_cmp_t64(t64_t a, t64_t b) {
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

/* Extended helper declarations that mirror the runtime implementations. */
extern t32_t __ternary_tmin_t32(t32_t a, t32_t b);
extern t32_t __ternary_tmax_t32(t32_t a, t32_t b);
extern t32_t __ternary_tmaj_t32(t32_t a, t32_t b, t32_t c);
extern t32_t __ternary_tlimp_t32(t32_t antecedent, t32_t consequent);
extern t32_t __ternary_tquant_t32(float value, float threshold);
extern t32_t __ternary_tnot_t32(t32_t a);
extern t32_t __ternary_tinv_t32(t32_t a);
extern t32_t __ternary_tmuladd_t32(t32_t a, t32_t b, t32_t c);
extern t32_t __ternary_tround_t32(t32_t a, unsigned drop);
extern t32_t __ternary_tnormalize_t32(t32_t a);
extern t32_t __ternary_tbias_t32(t32_t a, int64_t bias);
extern t32_t __ternary_tmux_t32(t32_t sel, t32_t neg, t32_t zero, t32_t pos);
extern t32_t __ternary_tequiv_t32(t32_t a, t32_t b);
extern t32_t __ternary_txor_t32(t32_t a, t32_t b);
extern int __ternary_tnet_t32(t32_t a);

extern t64_t __ternary_tmin_t64(t64_t a, t64_t b);
extern t64_t __ternary_tmax_t64(t64_t a, t64_t b);
extern t64_t __ternary_tmaj_t64(t64_t a, t64_t b, t64_t c);
extern t64_t __ternary_tlimp_t64(t64_t antecedent, t64_t consequent);
extern t64_t __ternary_tquant_t64(double value, double threshold);
extern t64_t __ternary_tnot_t64(t64_t a);
extern t64_t __ternary_tinv_t64(t64_t a);
extern t64_t __ternary_tmuladd_t64(t64_t a, t64_t b, t64_t c);
extern t64_t __ternary_tround_t64(t64_t a, unsigned drop);
extern t64_t __ternary_tnormalize_t64(t64_t a);
extern t64_t __ternary_tbias_t64(t64_t a, int64_t bias);
extern t64_t __ternary_tmux_t64(t64_t sel, t64_t neg, t64_t zero, t64_t pos);
extern t64_t __ternary_tequiv_t64(t64_t a, t64_t b);
extern t64_t __ternary_txor_t64(t64_t a, t64_t b);
extern int __ternary_tnet_t64(t64_t a);

#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
extern t128_t __ternary_tmin_t128(t128_t a, t128_t b);
extern t128_t __ternary_tmax_t128(t128_t a, t128_t b);
extern t128_t __ternary_tmaj_t128(t128_t a, t128_t b, t128_t c);
extern t128_t __ternary_tlimp_t128(t128_t antecedent, t128_t consequent);
extern t128_t __ternary_tquant_t128(double value, double threshold);
extern t128_t __ternary_tnot_t128(t128_t a);
extern t128_t __ternary_tinv_t128(t128_t a);
extern t128_t __ternary_tmuladd_t128(t128_t a, t128_t b, t128_t c);
extern t128_t __ternary_tround_t128(t128_t a, unsigned drop);
extern t128_t __ternary_tnormalize_t128(t128_t a);
extern t128_t __ternary_tbias_t128(t128_t a, int64_t bias);
extern t128_t __ternary_tmux_t128(t128_t sel, t128_t neg, t128_t zero, t128_t pos);
extern t128_t __ternary_tequiv_t128(t128_t a, t128_t b);
extern t128_t __ternary_txor_t128(t128_t a, t128_t b);
extern int __ternary_tnet_t128(t128_t a);
#endif
#ifdef __cplusplus
}
#endif

#endif /* TERNARY_HELPERS_H */
