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
    int mn = ternary_trit_min(a, b);
    int mx = ternary_trit_max(a, b);
    return a + b - 2 * mn - 2 * mx;
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
#define TERNARY_COMPAT_T64_CMP() \
    int __ternary_cmp_t64(t64_t a, t64_t b) { return TERNARY_RUNTIME_SYM(cmp_t64)(a, b); }

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
#undef TERNARY_COMPAT_T64_BIN
#undef TERNARY_COMPAT_T64_UNARY
#undef TERNARY_COMPAT_T64_SHIFT
#undef TERNARY_COMPAT_T64_SELECT
#undef TERNARY_COMPAT_T64_TB2T
#undef TERNARY_COMPAT_T64_TT2B
#undef TERNARY_COMPAT_T64_CMP
#endif
