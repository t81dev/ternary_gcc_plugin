#include "ternary_runtime_skeleton.h"

/*
 * Reference implementations for t32/t64 helpers.
 * This is intentionally small and easy to audit.
 */

#define __ternary_add TERNARY_RUNTIME_SYM(add)
#define __ternary_sub TERNARY_RUNTIME_SYM(sub)
#define __ternary_mul TERNARY_RUNTIME_SYM(mul)
#define __ternary_div TERNARY_RUNTIME_SYM(div)
#define __ternary_mod TERNARY_RUNTIME_SYM(mod)
#define __ternary_neg TERNARY_RUNTIME_SYM(neg)
#define __ternary_not TERNARY_RUNTIME_SYM(not)
#define __ternary_and TERNARY_RUNTIME_SYM(and)
#define __ternary_or TERNARY_RUNTIME_SYM(or)
#define __ternary_xor TERNARY_RUNTIME_SYM(xor)
#define __ternary_shl TERNARY_RUNTIME_SYM(shl)
#define __ternary_shr TERNARY_RUNTIME_SYM(shr)
#define __ternary_rol TERNARY_RUNTIME_SYM(rol)
#define __ternary_ror TERNARY_RUNTIME_SYM(ror)
#define __ternary_cmp TERNARY_RUNTIME_SYM(cmp)
#define __ternary_eq TERNARY_RUNTIME_SYM(eq)
#define __ternary_ne TERNARY_RUNTIME_SYM(ne)
#define __ternary_lt TERNARY_RUNTIME_SYM(lt)
#define __ternary_le TERNARY_RUNTIME_SYM(le)
#define __ternary_gt TERNARY_RUNTIME_SYM(gt)
#define __ternary_ge TERNARY_RUNTIME_SYM(ge)

#define __ternary_add_t32 TERNARY_RUNTIME_SYM(add_t32)
#define __ternary_sub_t32 TERNARY_RUNTIME_SYM(sub_t32)
#define __ternary_mul_t32 TERNARY_RUNTIME_SYM(mul_t32)
#define __ternary_div_t32 TERNARY_RUNTIME_SYM(div_t32)
#define __ternary_mod_t32 TERNARY_RUNTIME_SYM(mod_t32)
#define __ternary_neg_t32 TERNARY_RUNTIME_SYM(neg_t32)
#define __ternary_and_t32 TERNARY_RUNTIME_SYM(and_t32)
#define __ternary_or_t32 TERNARY_RUNTIME_SYM(or_t32)
#define __ternary_xor_t32 TERNARY_RUNTIME_SYM(xor_t32)
#define __ternary_shl_t32 TERNARY_RUNTIME_SYM(shl_t32)
#define __ternary_shr_t32 TERNARY_RUNTIME_SYM(shr_t32)
#define __ternary_rol_t32 TERNARY_RUNTIME_SYM(rol_t32)
#define __ternary_ror_t32 TERNARY_RUNTIME_SYM(ror_t32)
#define __ternary_select_t32 TERNARY_RUNTIME_SYM(select_t32)
#define __ternary_tb2t_t32 TERNARY_RUNTIME_SYM(tb2t_t32)
#define __ternary_tt2b_t32 TERNARY_RUNTIME_SYM(tt2b_t32)
#define __ternary_cmp_t32 TERNARY_RUNTIME_SYM(cmp_t32)

#define __ternary_add_t64 TERNARY_RUNTIME_SYM(add_t64)
#define __ternary_sub_t64 TERNARY_RUNTIME_SYM(sub_t64)
#define __ternary_mul_t64 TERNARY_RUNTIME_SYM(mul_t64)
#define __ternary_div_t64 TERNARY_RUNTIME_SYM(div_t64)
#define __ternary_mod_t64 TERNARY_RUNTIME_SYM(mod_t64)
#define __ternary_neg_t64 TERNARY_RUNTIME_SYM(neg_t64)
#define __ternary_and_t64 TERNARY_RUNTIME_SYM(and_t64)
#define __ternary_or_t64 TERNARY_RUNTIME_SYM(or_t64)
#define __ternary_xor_t64 TERNARY_RUNTIME_SYM(xor_t64)
#define __ternary_shl_t64 TERNARY_RUNTIME_SYM(shl_t64)
#define __ternary_shr_t64 TERNARY_RUNTIME_SYM(shr_t64)
#define __ternary_rol_t64 TERNARY_RUNTIME_SYM(rol_t64)
#define __ternary_ror_t64 TERNARY_RUNTIME_SYM(ror_t64)
#define __ternary_select_t64 TERNARY_RUNTIME_SYM(select_t64)
#define __ternary_tb2t_t64 TERNARY_RUNTIME_SYM(tb2t_t64)
#define __ternary_tt2b_t64 TERNARY_RUNTIME_SYM(tt2b_t64)
#define __ternary_cmp_t64 TERNARY_RUNTIME_SYM(cmp_t64)

static unsigned ternary_trit_to_bits(int trit)
{
    return (trit < 0) ? 0U : (trit == 0 ? 1U : 2U);
}

static int ternary_bits_to_trit(unsigned bits)
{
    if (bits == 0U)
        return -1;
    if (bits == 1U)
        return 0;
    return 1;
}

static int64_t ternary_decode_u64(uint64_t packed, unsigned trit_count)
{
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

static uint64_t ternary_encode_u64(int64_t value, unsigned trit_count)
{
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

static int ternary_get_trit_u64(uint64_t packed, unsigned idx)
{
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 0x3U);
    return ternary_bits_to_trit(bits);
}

static uint64_t ternary_set_trit_u64(uint64_t packed, unsigned idx, int trit)
{
    uint64_t mask = 0x3ULL << (2U * idx);
    uint64_t bits = (uint64_t)ternary_trit_to_bits(trit) << (2U * idx);
    return (packed & ~mask) | bits;
}

static uint64_t ternary_shift_left_u64(uint64_t packed, unsigned trit_count, unsigned shift)
{
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = 0;
        if (i >= shift)
            trit = ternary_get_trit_u64(packed, i - shift);
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static uint64_t ternary_shift_right_u64(uint64_t packed, unsigned trit_count, unsigned shift)
{
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    int sign_trit = ternary_get_trit_u64(packed, trit_count - 1);
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = sign_trit;
        if (i + shift < trit_count)
            trit = ternary_get_trit_u64(packed, i + shift);
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static uint64_t ternary_rotate_left_u64(uint64_t packed, unsigned trit_count, unsigned shift)
{
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + trit_count - shift) % trit_count;
        int trit = ternary_get_trit_u64(packed, src);
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static uint64_t ternary_rotate_right_u64(uint64_t packed, unsigned trit_count, unsigned shift)
{
    uint64_t out = 0;
    if (trit_count == 0)
        return packed;
    shift %= trit_count;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned src = (i + shift) % trit_count;
        int trit = ternary_get_trit_u64(packed, src);
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static int ternary_trit_min(int a, int b)
{
    return (a < b) ? a : b;
}

static int ternary_trit_max(int a, int b)
{
    return (a > b) ? a : b;
}

static int ternary_trit_xor(int a, int b)
{
    int sum = a + b;
    int mod = ((sum % 3) + 3) % 3;
    if (mod == 0) return 0;
    if (mod == 1) return 1;
    return -1;
}

static int ternary_trit_majority(int a, int b, int c)
{
    if (a == b || a == c)
        return a;
    if (b == c)
        return b;
    return 0;
}

static uint64_t ternary_majority_u64(uint64_t a, uint64_t b, uint64_t c, unsigned trit_count)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_majority(ternary_get_trit_u64(a, i),
                                          ternary_get_trit_u64(b, i),
                                          ternary_get_trit_u64(c, i));
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static unsigned __int128 ternary_majority_u128(unsigned __int128 a, unsigned __int128 b,
                                               unsigned __int128 c, unsigned trit_count)
{
    unsigned __int128 out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_majority(ternary_get_trit_u128(a, i),
                                          ternary_get_trit_u128(b, i),
                                          ternary_get_trit_u128(c, i));
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static uint64_t ternary_tritwise_op_u64(uint64_t a, uint64_t b, unsigned trit_count, int op)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int ta = ternary_get_trit_u64(a, i);
        int tb = ternary_get_trit_u64(b, i);
        int trit = 0;
        if (op == 0)
            trit = ternary_trit_min(ta, tb);
        else if (op == 1)
            trit = ternary_trit_max(ta, tb);
        else
            trit = ternary_trit_xor(ta, tb);
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static int ternary_trit_implication(int antecedent, int consequent)
{
    if (antecedent == -1)
        return 1;
    if (antecedent == 0) {
        if (consequent == -1)
            return -1;
        if (consequent == 0)
            return 0;
        return 1;
    }
    return consequent;
}

static uint64_t ternary_not_u64(uint64_t packed, unsigned trit_count)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_get_trit_u64(packed, i);
        int inverted = (trit == 0) ? 0 : -trit;
        out = ternary_set_trit_u64(out, i, inverted);
    }
    return out;
}

static unsigned __int128 ternary_not_u128(unsigned __int128 packed, unsigned trit_count)
{
    unsigned __int128 out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_get_trit_u128(packed, i);
        int inverted = (trit == 0) ? 0 : -trit;
        out = ternary_set_trit_u128(out, i, inverted);
    }
    return out;
}

static uint64_t ternary_muladd_u64(uint64_t a, uint64_t b, uint64_t c, unsigned trit_count)
{
    int64_t va = ternary_decode_u64(a, trit_count);
    int64_t vb = ternary_decode_u64(b, trit_count);
    int64_t vc = ternary_decode_u64(c, trit_count);
    return (uint64_t)ternary_encode_u64(va * vb + vc, trit_count);
}

static unsigned __int128 ternary_muladd_u128(unsigned __int128 a, unsigned __int128 b,
                                             unsigned __int128 c, unsigned trit_count)
{
    int64_t va = ternary_decode_u128(a, trit_count);
    int64_t vb = ternary_decode_u128(b, trit_count);
    int64_t vc = ternary_decode_u128(c, trit_count);
    return ternary_encode_u128(va * vb + vc, trit_count);
}

static uint64_t ternary_round_u64(uint64_t packed, unsigned trit_count, unsigned drop)
{
    if (drop >= trit_count)
        return 0;
    uint64_t out = 0;
    for (unsigned i = drop; i < trit_count; ++i) {
        int trit = ternary_get_trit_u64(packed, i);
        out = ternary_set_trit_u64(out, i - drop, trit);
    }
    return out;
}

static unsigned __int128 ternary_round_u128(unsigned __int128 packed, unsigned trit_count,
                                            unsigned drop)
{
    if (drop >= trit_count)
        return 0;
    unsigned __int128 out = 0;
    for (unsigned i = drop; i < trit_count; ++i) {
        int trit = ternary_get_trit_u128(packed, i);
        out = ternary_set_trit_u128(out, i - drop, trit);
    }
    return out;
}

static uint64_t ternary_normalize_u64(uint64_t packed, unsigned trit_count)
{
    uint64_t out = packed;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        if (bits == 0x3U) {
            out &= ~(0x3ULL << (2U * i));
            out |= (0x1ULL << (2U * i));
        }
    }
    return out;
}

static unsigned __int128 ternary_normalize_u128(unsigned __int128 packed, unsigned trit_count)
{
    unsigned __int128 out = packed;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        if (bits == 0x3U) {
            unsigned __int128 mask = (unsigned __int128)0x3U << (2U * i);
            out &= ~mask;
            out |= (unsigned __int128)0x1U << (2U * i);
        }
    }
    return out;
}

static uint64_t ternary_tbias_u64(uint64_t packed, unsigned trit_count, int64_t bias)
{
    int64_t value = ternary_decode_u64(packed, trit_count);
    return (uint64_t)ternary_encode_u64(value + bias, trit_count);
}

static unsigned __int128 ternary_tbias_u128(unsigned __int128 packed, unsigned trit_count, int64_t bias)
{
    int64_t value = ternary_decode_u128(packed, trit_count);
    return ternary_encode_u128(value + bias, trit_count);
}

static int ternary_branch_target(TERNARY_COND_T cond, int neg_target, int zero_target, int pos_target)
{
    if (cond < 0)
        return neg_target;
    if (cond > 0)
        return pos_target;
    return zero_target;
}

static int ternary_signjmp_u64(uint64_t packed, unsigned trit_count,
                               int neg_target, int zero_target, int pos_target)
{
    int64_t value = ternary_decode_u64(packed, trit_count);
    if (value < 0)
        return neg_target;
    if (value > 0)
        return pos_target;
    return zero_target;
}

static int ternary_signjmp_u128(unsigned __int128 packed, unsigned trit_count,
                                int neg_target, int zero_target, int pos_target)
{
    int64_t value = ternary_decode_u128(packed, trit_count);
    if (value < 0)
        return neg_target;
    if (value > 0)
        return pos_target;
    return zero_target;
}

static uint64_t ternary_implication_u64(uint64_t a, uint64_t b, unsigned trit_count)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_implication(ternary_get_trit_u64(a, i),
                                             ternary_get_trit_u64(b, i));
        out = ternary_set_trit_u64(out, i, trit);
    }
    return out;
}

static unsigned __int128 ternary_implication_u128(unsigned __int128 a, unsigned __int128 b,
                                                  unsigned trit_count)
{
    unsigned __int128 out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_implication(ternary_get_trit_u128(a, i),
                                             ternary_get_trit_u128(b, i));
        out = ternary_set_trit_u128(out, i, trit);
    }
    return out;
}

static int ternary_quantize_float(float value, float threshold)
{
    if (value > threshold)
        return 1;
    if (value < -threshold)
        return -1;
    return 0;
}

static uint64_t ternary_quantize_vector_float(float value, float threshold, unsigned trit_count)
{
    int trit = ternary_quantize_float(value, threshold);
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i)
        out = ternary_set_trit_u64(out, i, trit);
    return out;
}

static int ternary_quantize_double(double value, double threshold)
{
    if (value > threshold)
        return 1;
    if (value < -threshold)
        return -1;
    return 0;
}

static unsigned __int128 ternary_quantize_vector_double(double value, double threshold,
                                                        unsigned trit_count)
{
    int trit = ternary_quantize_double(value, threshold);
    unsigned __int128 out = 0;
    for (unsigned i = 0; i < trit_count; ++i)
        out = ternary_set_trit_u128(out, i, trit);
    return out;
}

static int64_t ternary_decode_u128(unsigned __int128 packed, unsigned trit_count)
{
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

static unsigned __int128 ternary_encode_u128(int64_t value, unsigned trit_count)
{
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

static int ternary_get_trit_u128(unsigned __int128 packed, unsigned idx)
{
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 0x3U);
    return ternary_bits_to_trit(bits);
}

static unsigned __int128 ternary_set_trit_u128(unsigned __int128 packed, unsigned idx, int trit)
{
    unsigned __int128 mask = (unsigned __int128)0x3U << (2U * idx);
    unsigned __int128 bits = (unsigned __int128)ternary_trit_to_bits(trit) << (2U * idx);
    return (packed & ~mask) | bits;
}

static unsigned __int128 ternary_shift_left_u128(unsigned __int128 packed, unsigned trit_count,
                                                 unsigned shift)
{
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

static unsigned __int128 ternary_shift_right_u128(unsigned __int128 packed, unsigned trit_count,
                                                  unsigned shift)
{
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

static unsigned __int128 ternary_rotate_left_u128(unsigned __int128 packed, unsigned trit_count,
                                                  unsigned shift)
{
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

static unsigned __int128 ternary_rotate_right_u128(unsigned __int128 packed, unsigned trit_count,
                                                   unsigned shift)
{
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

static unsigned __int128 ternary_tritwise_op_u128(unsigned __int128 a, unsigned __int128 b,
                                                  unsigned trit_count, int op)
{
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

/* Scalar helpers (non-packed). */

int __ternary_add(int a, int b)
{
    return a + b;
}

int __ternary_sub(int a, int b)
{
    return a - b;
}

int __ternary_mul(int a, int b)
{
    return a * b;
}

int __ternary_div(int a, int b)
{
    return (b == 0) ? 0 : (a / b);
}

int __ternary_mod(int a, int b)
{
    return (b == 0) ? 0 : (a % b);
}

int __ternary_neg(int a)
{
    return -a;
}

int __ternary_not(int a)
{
    return -a;
}

int __ternary_and(int a, int b)
{
    return ternary_trit_min(a, b);
}

int __ternary_or(int a, int b)
{
    return ternary_trit_max(a, b);
}

int __ternary_xor(int a, int b)
{
    return ternary_trit_xor(a, b);
}

int __ternary_shl(int a, int shift)
{
    int64_t value = a;
    int64_t pow3 = 1;
    for (int i = 0; i < shift; ++i)
        pow3 *= 3;
    return (int)(value * pow3);
}

int __ternary_shr(int a, int shift)
{
    int64_t value = a;
    int64_t pow3 = 1;
    for (int i = 0; i < shift; ++i)
        pow3 *= 3;
    return (int)(value / pow3);
}

int __ternary_rol(int a, int shift)
{
    (void)shift;
    return a;
}

int __ternary_ror(int a, int shift)
{
    (void)shift;
    return a;
}

int __ternary_cmp(int a, int b)
{
    if (a < b)
        return -1;
    if (a > b)
        return 1;
    return 0;
}

int __ternary_eq(int a, int b)
{
    return __ternary_cmp(a, b) == 0 ? 1 : 0;
}

int __ternary_ne(int a, int b)
{
    return __ternary_cmp(a, b) != 0 ? 1 : 0;
}

int __ternary_lt(int a, int b)
{
    return __ternary_cmp(a, b) == -1 ? 1 : 0;
}

int __ternary_le(int a, int b)
{
    int cmp = __ternary_cmp(a, b);
    return (cmp == -1 || cmp == 0) ? 1 : 0;
}

int __ternary_gt(int a, int b)
{
    return __ternary_cmp(a, b) == 1 ? 1 : 0;
}

int __ternary_ge(int a, int b)
{
    int cmp = __ternary_cmp(a, b);
    return (cmp == 1 || cmp == 0) ? 1 : 0;
}

/* t32 helpers */

t32_t __ternary_add_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    return (t32_t)ternary_encode_u64(va + vb, 32);
}

t32_t __ternary_sub_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    return (t32_t)ternary_encode_u64(va - vb, 32);
}

t32_t __ternary_mul_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    return (t32_t)ternary_encode_u64(va * vb, 32);
}

t32_t __ternary_div_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    if (vb == 0)
        return (t32_t)ternary_encode_u64(0, 32);
    return (t32_t)ternary_encode_u64(va / vb, 32);
}

t32_t __ternary_mod_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    if (vb == 0)
        return (t32_t)ternary_encode_u64(0, 32);
    return (t32_t)ternary_encode_u64(va % vb, 32);
}

t32_t __ternary_neg_t32(t32_t a)
{
    int64_t va = ternary_decode_u64(a, 32);
    return (t32_t)ternary_encode_u64(-va, 32);
}

t32_t __ternary_and_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op_u64(a, b, 32, 0);
}

t32_t __ternary_or_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op_u64(a, b, 32, 1);
}

t32_t __ternary_xor_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op_u64(a, b, 32, 2);
}

t32_t __ternary_tmin_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op_u64(a, b, 32, 0);
}

t32_t __ternary_tmax_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op_u64(a, b, 32, 1);
}

t32_t __ternary_tmaj_t32(t32_t a, t32_t b, t32_t c)
{
    return (t32_t)ternary_majority_u64(a, b, c, 32);
}

t32_t __ternary_tlimp_t32(t32_t antecedent, t32_t consequent)
{
    return (t32_t)ternary_implication_u64(antecedent, consequent, 32);
}

t32_t __ternary_tquant_t32(float value, float threshold)
{
    return (t32_t)ternary_quantize_vector_float(value, threshold, 32);
}

t32_t __ternary_tnot_t32(t32_t a)
{
    return (t32_t)ternary_not_u64(a, 32);
}

t32_t __ternary_tinv_t32(t32_t a)
{
    return __ternary_tnot_t32(a);
}

t32_t __ternary_tmuladd_t32(t32_t a, t32_t b, t32_t c)
{
    return (t32_t)ternary_muladd_u64(a, b, c, 32);
}

t32_t __ternary_tround_t32(t32_t a, unsigned drop)
{
    return (t32_t)ternary_round_u64(a, 32, drop);
}

t32_t __ternary_tnormalize_t32(t32_t a)
{
    return (t32_t)ternary_normalize_u64(a, 32);
}

t32_t __ternary_tbias_t32(t32_t a, int64_t bias)
{
    return (t32_t)ternary_tbias_u64(a, 32, bias);
}

t32_t __ternary_shl_t32(t32_t a, int shift)
{
    return (t32_t)ternary_shift_left_u64(a, 32, (unsigned)shift);
}

t32_t __ternary_shr_t32(t32_t a, int shift)
{
    return (t32_t)ternary_shift_right_u64(a, 32, (unsigned)shift);
}

t32_t __ternary_rol_t32(t32_t a, int shift)
{
    return (t32_t)ternary_rotate_left_u64(a, 32, (unsigned)shift);
}

t32_t __ternary_ror_t32(t32_t a, int shift)
{
    return (t32_t)ternary_rotate_right_u64(a, 32, (unsigned)shift);
}

t32_t __ternary_select_t32(TERNARY_COND_T cond, t32_t t, t32_t f)
{
    return cond ? t : f;
}

t32_t __ternary_tb2t_t32(int64_t v)
{
    return (t32_t)ternary_encode_u64(v, 32);
}

int64_t __ternary_tt2b_t32(t32_t v)
{
    return ternary_decode_u64(v, 32);
}

int __ternary_cmp_t32(t32_t a, t32_t b)
{
    int64_t va = ternary_decode_u64(a, 32);
    int64_t vb = ternary_decode_u64(b, 32);
    if (va < vb)
        return -1;
    if (va > vb)
        return 1;
    return 0;
}

/* t64 helpers */

t64_t __ternary_add_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va + vb, 64);
}

t64_t __ternary_sub_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va - vb, 64);
}

t64_t __ternary_mul_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    return (t64_t)ternary_encode_u128(va * vb, 64);
}

t64_t __ternary_div_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    if (vb == 0)
        return (t64_t)ternary_encode_u128(0, 64);
    return (t64_t)ternary_encode_u128(va / vb, 64);
}

t64_t __ternary_mod_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    if (vb == 0)
        return (t64_t)ternary_encode_u128(0, 64);
    return (t64_t)ternary_encode_u128(va % vb, 64);
}

t64_t __ternary_neg_t64(t64_t a)
{
    int64_t va = ternary_decode_u128(a, 64);
    return (t64_t)ternary_encode_u128(-va, 64);
}

t64_t __ternary_and_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 0);
}

t64_t __ternary_or_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 1);
}

t64_t __ternary_xor_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 2);
}

t64_t __ternary_tmin_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 0);
}

t64_t __ternary_tmax_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128(a, b, 64, 1);
}

t64_t __ternary_tmaj_t64(t64_t a, t64_t b, t64_t c)
{
    return (t64_t)ternary_majority_u128(a, b, c, 64);
}

t64_t __ternary_tlimp_t64(t64_t antecedent, t64_t consequent)
{
    return (t64_t)ternary_implication_u128(antecedent, consequent, 64);
}

t64_t __ternary_tquant_t64(double value, double threshold)
{
    return (t64_t)ternary_quantize_vector_double(value, threshold, 64);
}

t64_t __ternary_tnot_t64(t64_t a)
{
    return (t64_t)ternary_not_u128(a, 64);
}

t64_t __ternary_tinv_t64(t64_t a)
{
    return __ternary_tnot_t64(a);
}

t64_t __ternary_tmuladd_t64(t64_t a, t64_t b, t64_t c)
{
    return (t64_t)ternary_muladd_u128(a, b, c, 64);
}

t64_t __ternary_tround_t64(t64_t a, unsigned drop)
{
    return (t64_t)ternary_round_u128(a, 64, drop);
}

t64_t __ternary_tnormalize_t64(t64_t a)
{
    return (t64_t)ternary_normalize_u128(a, 64);
}

t64_t __ternary_tbias_t64(t64_t a, int64_t bias)
{
    return (t64_t)ternary_tbias_u128(a, 64, bias);
}

t64_t __ternary_shl_t64(t64_t a, int shift)
{
    return (t64_t)ternary_shift_left_u128(a, 64, (unsigned)shift);
}

t64_t __ternary_shr_t64(t64_t a, int shift)
{
    return (t64_t)ternary_shift_right_u128(a, 64, (unsigned)shift);
}

t64_t __ternary_rol_t64(t64_t a, int shift)
{
    return (t64_t)ternary_rotate_left_u128(a, 64, (unsigned)shift);
}

t64_t __ternary_ror_t64(t64_t a, int shift)
{
    return (t64_t)ternary_rotate_right_u128(a, 64, (unsigned)shift);
}

t64_t __ternary_select_t64(TERNARY_COND_T cond, t64_t t, t64_t f)
{
    return cond ? t : f;
}

t64_t __ternary_tb2t_t64(int64_t v)
{
    return (t64_t)ternary_encode_u128(v, 64);
}

int64_t __ternary_tt2b_t64(t64_t v)
{
    return ternary_decode_u128(v, 64);
}

int __ternary_cmp_t64(t64_t a, t64_t b)
{
    int64_t va = ternary_decode_u128(a, 64);
    int64_t vb = ternary_decode_u128(b, 64);
    if (va < vb)
        return -1;
    if (va > vb)
        return 1;
    return 0;
}

int __ternary_tbranch(TERNARY_COND_T cond, int neg_target, int zero_target, int pos_target)
{
    return ternary_branch_target(cond, neg_target, zero_target, pos_target);
}

int __ternary_tsignjmp_t32(t32_t reg, int neg_target, int zero_target, int pos_target)
{
    return ternary_signjmp_u64(reg, 32, neg_target, zero_target, pos_target);
}

int __ternary_tsignjmp_t64(t64_t reg, int neg_target, int zero_target, int pos_target)
{
    return ternary_signjmp_u128(reg, 64, neg_target, zero_target, pos_target);
}

#undef __ternary_add
#undef __ternary_sub
#undef __ternary_mul
#undef __ternary_div
#undef __ternary_mod
#undef __ternary_neg
#undef __ternary_not
#undef __ternary_and
#undef __ternary_or
#undef __ternary_xor
#undef __ternary_shl
#undef __ternary_shr
#undef __ternary_rol
#undef __ternary_ror
#undef __ternary_cmp
#undef __ternary_eq
#undef __ternary_ne
#undef __ternary_lt
#undef __ternary_le
#undef __ternary_gt
#undef __ternary_ge

#undef __ternary_add_t32
#undef __ternary_sub_t32
#undef __ternary_mul_t32
#undef __ternary_div_t32
#undef __ternary_mod_t32
#undef __ternary_neg_t32
#undef __ternary_and_t32
#undef __ternary_or_t32
#undef __ternary_xor_t32
#undef __ternary_shl_t32
#undef __ternary_shr_t32
#undef __ternary_rol_t32
#undef __ternary_ror_t32
#undef __ternary_select_t32
#undef __ternary_tb2t_t32
#undef __ternary_tt2b_t32
#undef __ternary_cmp_t32

#undef __ternary_add_t64
#undef __ternary_sub_t64
#undef __ternary_mul_t64
#undef __ternary_div_t64
#undef __ternary_mod_t64
#undef __ternary_neg_t64
#undef __ternary_and_t64
#undef __ternary_or_t64
#undef __ternary_xor_t64
#undef __ternary_shl_t64
#undef __ternary_shr_t64
#undef __ternary_rol_t64
#undef __ternary_ror_t64
#undef __ternary_select_t64
#undef __ternary_tb2t_t64
#undef __ternary_tt2b_t64
#undef __ternary_cmp_t64

#ifndef TERNARY_RUNTIME_NO_COMPAT
#define TERNARY_COMPAT_SCALAR_BIN(name) \
    int __ternary_##name(int a, int b) { return TERNARY_RUNTIME_SYM(name)(a, b); }
#define TERNARY_COMPAT_SCALAR_UNARY(name) \
    int __ternary_##name(int a) { return TERNARY_RUNTIME_SYM(name)(a); }
#define TERNARY_COMPAT_SCALAR_SHIFT(name) \
    int __ternary_##name(int a, int shift) { return TERNARY_RUNTIME_SYM(name)(a, shift); }

#define TERNARY_COMPAT_T32_BIN(name) \
    t32_t __ternary_##name##_t32(t32_t a, t32_t b) { return TERNARY_RUNTIME_SYM(name##_t32)(a, b); }
#define TERNARY_COMPAT_T32_UNARY(name) \
    t32_t __ternary_##name##_t32(t32_t a) { return TERNARY_RUNTIME_SYM(name##_t32)(a); }
#define TERNARY_COMPAT_T32_SHIFT(name) \
    t32_t __ternary_##name##_t32(t32_t a, int shift) { return TERNARY_RUNTIME_SYM(name##_t32)(a, shift); }
#define TERNARY_COMPAT_T32_SELECT() \
    t32_t __ternary_select_t32(TERNARY_COND_T cond, t32_t t, t32_t f) { \
        return TERNARY_RUNTIME_SYM(select_t32)(cond, t, f); \
    }
#define TERNARY_COMPAT_T32_TB2T() \
    t32_t __ternary_tb2t_t32(int64_t v) { return TERNARY_RUNTIME_SYM(tb2t_t32)(v); }
#define TERNARY_COMPAT_T32_TT2B() \
    int64_t __ternary_tt2b_t32(t32_t v) { return TERNARY_RUNTIME_SYM(tt2b_t32)(v); }
#define TERNARY_COMPAT_T32_IMP() \
    t32_t __ternary_tlimp_t32(t32_t antecedent, t32_t consequent) { \
        return TERNARY_RUNTIME_SYM(tlimp_t32)(antecedent, consequent); \
    }
#define TERNARY_COMPAT_T32_QUANT() \
    t32_t __ternary_tquant_t32(float value, float threshold) { \
        return TERNARY_RUNTIME_SYM(tquant_t32)(value, threshold); \
    }
#define TERNARY_COMPAT_T32_CMP() \
    int __ternary_cmp_t32(t32_t a, t32_t b) { return TERNARY_RUNTIME_SYM(cmp_t32)(a, b); }

#define TERNARY_COMPAT_T64_BIN(name) \
    t64_t __ternary_##name##_t64(t64_t a, t64_t b) { return TERNARY_RUNTIME_SYM(name##_t64)(a, b); }
#define TERNARY_COMPAT_T64_UNARY(name) \
    t64_t __ternary_##name##_t64(t64_t a) { return TERNARY_RUNTIME_SYM(name##_t64)(a); }
#define TERNARY_COMPAT_T64_SHIFT(name) \
    t64_t __ternary_##name##_t64(t64_t a, int shift) { return TERNARY_RUNTIME_SYM(name##_t64)(a, shift); }
#define TERNARY_COMPAT_T64_SELECT() \
    t64_t __ternary_select_t64(TERNARY_COND_T cond, t64_t t, t64_t f) { \
        return TERNARY_RUNTIME_SYM(select_t64)(cond, t, f); \
    }
#define TERNARY_COMPAT_T64_TB2T() \
    t64_t __ternary_tb2t_t64(int64_t v) { return TERNARY_RUNTIME_SYM(tb2t_t64)(v); }
#define TERNARY_COMPAT_T64_TT2B() \
    int64_t __ternary_tt2b_t64(t64_t v) { return TERNARY_RUNTIME_SYM(tt2b_t64)(v); }
#define TERNARY_COMPAT_T64_IMP() \
    t64_t __ternary_tlimp_t64(t64_t antecedent, t64_t consequent) { \
        return TERNARY_RUNTIME_SYM(tlimp_t64)(antecedent, consequent); \
    }
#define TERNARY_COMPAT_T64_QUANT() \
    t64_t __ternary_tquant_t64(double value, double threshold) { \
        return TERNARY_RUNTIME_SYM(tquant_t64)(value, threshold); \
    }
#define TERNARY_COMPAT_T64_CMP() \
    int __ternary_cmp_t64(t64_t a, t64_t b) { return TERNARY_RUNTIME_SYM(cmp_t64)(a, b); }

#define TERNARY_COMPAT_T32_TERNARY(name) \
    t32_t __ternary_##name##_t32(t32_t a, t32_t b, t32_t c) { \
        return TERNARY_RUNTIME_SYM(name##_t32)(a, b, c); \
    }

#define TERNARY_COMPAT_T64_TERNARY(name) \
    t64_t __ternary_##name##_t64(t64_t a, t64_t b, t64_t c) { \
        return TERNARY_RUNTIME_SYM(name##_t64)(a, b, c); \
    }

TERNARY_COMPAT_SCALAR_BIN(add)
TERNARY_COMPAT_SCALAR_BIN(sub)
TERNARY_COMPAT_SCALAR_BIN(mul)
TERNARY_COMPAT_SCALAR_BIN(div)
TERNARY_COMPAT_SCALAR_BIN(mod)
TERNARY_COMPAT_SCALAR_UNARY(neg)
TERNARY_COMPAT_SCALAR_UNARY(not)
TERNARY_COMPAT_SCALAR_BIN(and)
TERNARY_COMPAT_SCALAR_BIN(or)
TERNARY_COMPAT_SCALAR_BIN(xor)
TERNARY_COMPAT_SCALAR_SHIFT(shl)
TERNARY_COMPAT_SCALAR_SHIFT(shr)
TERNARY_COMPAT_SCALAR_SHIFT(rol)
TERNARY_COMPAT_SCALAR_SHIFT(ror)
TERNARY_COMPAT_SCALAR_BIN(cmp)
TERNARY_COMPAT_SCALAR_BIN(eq)
TERNARY_COMPAT_SCALAR_BIN(ne)
TERNARY_COMPAT_SCALAR_BIN(lt)
TERNARY_COMPAT_SCALAR_BIN(le)
TERNARY_COMPAT_SCALAR_BIN(gt)
TERNARY_COMPAT_SCALAR_BIN(ge)

TERNARY_COMPAT_T32_BIN(add)
TERNARY_COMPAT_T32_BIN(sub)
TERNARY_COMPAT_T32_BIN(mul)
TERNARY_COMPAT_T32_BIN(div)
TERNARY_COMPAT_T32_BIN(mod)
TERNARY_COMPAT_T32_UNARY(neg)
TERNARY_COMPAT_T32_BIN(and)
TERNARY_COMPAT_T32_BIN(or)
TERNARY_COMPAT_T32_BIN(xor)
TERNARY_COMPAT_T32_BIN(tmin)
TERNARY_COMPAT_T32_BIN(tmax)
TERNARY_COMPAT_T32_TERNARY(tmaj)
TERNARY_COMPAT_T32_IMP()
TERNARY_COMPAT_T32_QUANT()
TERNARY_COMPAT_T32_SHIFT(shl)
TERNARY_COMPAT_T32_SHIFT(shr)
TERNARY_COMPAT_T32_SHIFT(rol)
TERNARY_COMPAT_T32_SHIFT(ror)
TERNARY_COMPAT_T32_SELECT()
TERNARY_COMPAT_T32_TB2T()
TERNARY_COMPAT_T32_TT2B()
TERNARY_COMPAT_T32_CMP()

TERNARY_COMPAT_T64_BIN(add)
TERNARY_COMPAT_T64_BIN(sub)
TERNARY_COMPAT_T64_BIN(mul)
TERNARY_COMPAT_T64_BIN(div)
TERNARY_COMPAT_T64_BIN(mod)
TERNARY_COMPAT_T64_UNARY(neg)
TERNARY_COMPAT_T64_BIN(and)
TERNARY_COMPAT_T64_BIN(or)
TERNARY_COMPAT_T64_BIN(xor)
TERNARY_COMPAT_T64_BIN(tmin)
TERNARY_COMPAT_T64_BIN(tmax)
TERNARY_COMPAT_T64_TERNARY(tmaj)
TERNARY_COMPAT_T64_IMP()
TERNARY_COMPAT_T64_QUANT()
TERNARY_COMPAT_T64_SHIFT(shl)
TERNARY_COMPAT_T64_SHIFT(shr)
TERNARY_COMPAT_T64_SHIFT(rol)
TERNARY_COMPAT_T64_SHIFT(ror)
TERNARY_COMPAT_T64_SELECT()
TERNARY_COMPAT_T64_TB2T()
TERNARY_COMPAT_T64_TT2B()
TERNARY_COMPAT_T64_CMP()

#undef TERNARY_COMPAT_SCALAR_BIN
#undef TERNARY_COMPAT_SCALAR_UNARY
#undef TERNARY_COMPAT_SCALAR_SHIFT
#undef TERNARY_COMPAT_T32_BIN
#undef TERNARY_COMPAT_T32_UNARY
#undef TERNARY_COMPAT_T32_SHIFT
#undef TERNARY_COMPAT_T32_SELECT
#undef TERNARY_COMPAT_T32_TB2T
#undef TERNARY_COMPAT_T32_TT2B
#undef TERNARY_COMPAT_T32_CMP
#undef TERNARY_COMPAT_T32_IMP
#undef TERNARY_COMPAT_T32_QUANT
#undef TERNARY_COMPAT_T64_BIN
#undef TERNARY_COMPAT_T64_UNARY
#undef TERNARY_COMPAT_T64_SHIFT
#undef TERNARY_COMPAT_T64_SELECT
#undef TERNARY_COMPAT_T64_TB2T
#undef TERNARY_COMPAT_T64_TT2B
#undef TERNARY_COMPAT_T64_CMP
#undef TERNARY_COMPAT_T64_IMP
#undef TERNARY_COMPAT_T64_QUANT
#undef TERNARY_COMPAT_T32_TERNARY
#undef TERNARY_COMPAT_T64_TERNARY
#endif
