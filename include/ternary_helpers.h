#ifndef TERNARY_HELPERS_H
#define TERNARY_HELPERS_H

/* Helper functions for a balanced-ternary ISA.
 * These are example implementations using placeholder ISA instructions (tsel/tadd/tmul/tnot).
 * Adjust the assembly to match your actual ISA and calling convention.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TERNARY_COND_T
#ifdef __cplusplus
#define TERNARY_COND_T bool
#else
#include <stdbool.h>
#define TERNARY_COND_T bool
#endif
#endif

/* TERNARY_COND_T should match the exact condition type used at the call site.
 * Non-zero values are treated as true by the select helpers.
 */

/* Define TERNARY_USE_BUILTIN_TYPES to use plugin-provided ternary types (t6_t, t12_t, etc.)
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
typedef uint16_t t6_t;   /* 6 trits -> 12 bits */
typedef uint32_t t12_t;  /* 12 trits -> 24 bits */
typedef uint64_t t24_t;  /* 24 trits -> 48 bits */
#endif

/* Ternary type select operations */

/* t6 selects */
static inline t6_t __ternary_select_t6(TERNARY_COND_T cond, t6_t true_val, t6_t false_val) {
    return cond ? true_val : false_val;
}

/* t12 selects */
static inline t12_t __ternary_select_t12(TERNARY_COND_T cond, t12_t true_val, t12_t false_val) {
    return cond ? true_val : false_val;
}

/* t24 selects */
static inline t24_t __ternary_select_t24(TERNARY_COND_T cond, t24_t true_val, t24_t false_val) {
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
    return a / b;
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
    return a % b;
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

/* Ternary type arithmetic operations (t6, t12, t24, etc.) */
/* Note: These use the same underlying representation but with ternary semantics */

static inline t6_t __ternary_add_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    return (t6_t)ternary_encode(va + vb, 6);
}

static inline t6_t __ternary_mul_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    return (t6_t)ternary_encode(va * vb, 6);
}

static inline t6_t __ternary_not_t6(t6_t a) {
    int64_t va = ternary_decode(a, 6);
    return (t6_t)ternary_encode(-va, 6);
}

static inline t6_t __ternary_sub_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    return (t6_t)ternary_encode(va - vb, 6);
}

static inline t6_t __ternary_div_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    return (t6_t)ternary_encode(vb == 0 ? 0 : va / vb, 6);
}

static inline t6_t __ternary_mod_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    return (t6_t)ternary_encode(vb == 0 ? 0 : va % vb, 6);
}

static inline t6_t __ternary_neg_t6(t6_t a) {
    int64_t va = ternary_decode(a, 6);
    return (t6_t)ternary_encode(-va, 6);
}

static inline t6_t __ternary_and_t6(t6_t a, t6_t b) {
    return (t6_t)ternary_tritwise_op(a, b, 6, 0);
}

static inline t6_t __ternary_or_t6(t6_t a, t6_t b) {
    return (t6_t)ternary_tritwise_op(a, b, 6, 1);
}

static inline t6_t __ternary_xor_t6(t6_t a, t6_t b) {
    return (t6_t)ternary_tritwise_op(a, b, 6, 2);
}

static inline t6_t __ternary_shl_t6(t6_t a, int shift) {
    return (t6_t)ternary_shift_left(a, 6, (unsigned)shift);
}

static inline t6_t __ternary_shr_t6(t6_t a, int shift) {
    return (t6_t)ternary_shift_right(a, 6, (unsigned)shift);
}

static inline t6_t __ternary_rol_t6(t6_t a, int shift) {
    return (t6_t)ternary_rotate_left(a, 6, (unsigned)shift);
}

static inline t6_t __ternary_ror_t6(t6_t a, int shift) {
    return (t6_t)ternary_rotate_right(a, 6, (unsigned)shift);
}

static inline t6_t __ternary_tb2t_t6(int64_t v) {
    return (t6_t)ternary_encode(v, 6);
}

static inline int64_t __ternary_tt2b_t6(t6_t v) {
    return ternary_decode(v, 6);
}

static inline float __ternary_t2f32_t6(t6_t v) {
    return (float)ternary_decode(v, 6);
}

static inline double __ternary_t2f64_t6(t6_t v) {
    return (double)ternary_decode(v, 6);
}

static inline t6_t __ternary_f2t32_t6(float v) {
    return (t6_t)ternary_encode((int64_t)v, 6);
}

static inline t6_t __ternary_f2t64_t6(double v) {
    return (t6_t)ternary_encode((int64_t)v, 6);
}

/* t12 operations (12 trits) */

static inline t12_t __ternary_add_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    return (t12_t)ternary_encode(va + vb, 12);
}

static inline t12_t __ternary_mul_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    return (t12_t)ternary_encode(va * vb, 12);
}

static inline t12_t __ternary_not_t12(t12_t a) {
    int64_t va = ternary_decode(a, 12);
    return (t12_t)ternary_encode(-va, 12);
}

static inline t12_t __ternary_sub_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    return (t12_t)ternary_encode(va - vb, 12);
}

static inline t12_t __ternary_div_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    return (t12_t)ternary_encode(vb == 0 ? 0 : va / vb, 12);
}

static inline t12_t __ternary_mod_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    return (t12_t)ternary_encode(vb == 0 ? 0 : va % vb, 12);
}

static inline t12_t __ternary_neg_t12(t12_t a) {
    int64_t va = ternary_decode(a, 12);
    return (t12_t)ternary_encode(-va, 12);
}

static inline t12_t __ternary_and_t12(t12_t a, t12_t b) {
    return (t12_t)ternary_tritwise_op(a, b, 12, 0);
}

static inline t12_t __ternary_or_t12(t12_t a, t12_t b) {
    return (t12_t)ternary_tritwise_op(a, b, 12, 1);
}

static inline t12_t __ternary_xor_t12(t12_t a, t12_t b) {
    return (t12_t)ternary_tritwise_op(a, b, 12, 2);
}

static inline t12_t __ternary_shl_t12(t12_t a, int shift) {
    return (t12_t)ternary_shift_left(a, 12, (unsigned)shift);
}

static inline t12_t __ternary_shr_t12(t12_t a, int shift) {
    return (t12_t)ternary_shift_right(a, 12, (unsigned)shift);
}

static inline t12_t __ternary_rol_t12(t12_t a, int shift) {
    return (t12_t)ternary_rotate_left(a, 12, (unsigned)shift);
}

static inline t12_t __ternary_ror_t12(t12_t a, int shift) {
    return (t12_t)ternary_rotate_right(a, 12, (unsigned)shift);
}

static inline t12_t __ternary_tb2t_t12(int64_t v) {
    return (t12_t)ternary_encode(v, 12);
}

static inline int64_t __ternary_tt2b_t12(t12_t v) {
    return ternary_decode(v, 12);
}

static inline float __ternary_t2f32_t12(t12_t v) {
    return (float)ternary_decode(v, 12);
}

static inline double __ternary_t2f64_t12(t12_t v) {
    return (double)ternary_decode(v, 12);
}

static inline t12_t __ternary_f2t32_t12(float v) {
    return (t12_t)ternary_encode((int64_t)v, 12);
}

static inline t12_t __ternary_f2t64_t12(double v) {
    return (t12_t)ternary_encode((int64_t)v, 12);
}

/* t24 operations (24 trits) */

static inline t24_t __ternary_add_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    return (t24_t)ternary_encode(va + vb, 24);
}

static inline t24_t __ternary_mul_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    return (t24_t)ternary_encode(va * vb, 24);
}

static inline t24_t __ternary_not_t24(t24_t a) {
    int64_t va = ternary_decode(a, 24);
    return (t24_t)ternary_encode(-va, 24);
}

static inline t24_t __ternary_sub_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    return (t24_t)ternary_encode(va - vb, 24);
}

static inline t24_t __ternary_div_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    return (t24_t)ternary_encode(vb == 0 ? 0 : va / vb, 24);
}

static inline t24_t __ternary_mod_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    return (t24_t)ternary_encode(vb == 0 ? 0 : va % vb, 24);
}

static inline t24_t __ternary_neg_t24(t24_t a) {
    int64_t va = ternary_decode(a, 24);
    return (t24_t)ternary_encode(-va, 24);
}

static inline t24_t __ternary_and_t24(t24_t a, t24_t b) {
    return (t24_t)ternary_tritwise_op(a, b, 24, 0);
}

static inline t24_t __ternary_or_t24(t24_t a, t24_t b) {
    return (t24_t)ternary_tritwise_op(a, b, 24, 1);
}

static inline t24_t __ternary_xor_t24(t24_t a, t24_t b) {
    return (t24_t)ternary_tritwise_op(a, b, 24, 2);
}

static inline t24_t __ternary_shl_t24(t24_t a, int shift) {
    return (t24_t)ternary_shift_left(a, 24, (unsigned)shift);
}

static inline t24_t __ternary_shr_t24(t24_t a, int shift) {
    return (t24_t)ternary_shift_right(a, 24, (unsigned)shift);
}

static inline t24_t __ternary_rol_t24(t24_t a, int shift) {
    return (t24_t)ternary_rotate_left(a, 24, (unsigned)shift);
}

static inline t24_t __ternary_ror_t24(t24_t a, int shift) {
    return (t24_t)ternary_rotate_right(a, 24, (unsigned)shift);
}

static inline t24_t __ternary_tb2t_t24(int64_t v) {
    return (t24_t)ternary_encode(v, 24);
}

static inline int64_t __ternary_tt2b_t24(t24_t v) {
    return ternary_decode(v, 24);
}

static inline float __ternary_t2f32_t24(t24_t v) {
    return (float)ternary_decode(v, 24);
}

static inline double __ternary_t2f64_t24(t24_t v) {
    return (double)ternary_decode(v, 24);
}

static inline t24_t __ternary_f2t32_t24(float v) {
    return (t24_t)ternary_encode((int64_t)v, 24);
}

static inline t24_t __ternary_f2t64_t24(double v) {
    return (t24_t)ternary_encode((int64_t)v, 24);
}
/* Ternary comparison operations (return -1, 0, +1) */
static inline int __ternary_cmp(int a, int b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static inline int __ternary_cmp_t6(t6_t a, t6_t b) {
    int64_t va = ternary_decode(a, 6);
    int64_t vb = ternary_decode(b, 6);
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

static inline int __ternary_cmp_t12(t12_t a, t12_t b) {
    int64_t va = ternary_decode(a, 12);
    int64_t vb = ternary_decode(b, 12);
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

static inline int __ternary_cmp_t24(t24_t a, t24_t b) {
    int64_t va = ternary_decode(a, 24);
    int64_t vb = ternary_decode(b, 24);
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}
#ifdef __cplusplus
}
#endif

#endif /* TERNARY_HELPERS_H */
