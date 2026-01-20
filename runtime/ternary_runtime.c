#include "ternary_runtime.h"

/* Reference runtime for packed 2-bit-per-trit values.
 * This is a portable skeleton; replace with ISA-specific code as needed.
 */

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

static int64_t ternary_decode(uint64_t packed, unsigned trit_count)
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

static uint64_t ternary_encode(int64_t value, unsigned trit_count)
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

static int ternary_get_trit(uint64_t packed, unsigned idx)
{
    unsigned bits = (unsigned)((packed >> (2U * idx)) & 0x3U);
    return ternary_bits_to_trit(bits);
}

static uint64_t ternary_set_trit(uint64_t packed, unsigned idx, int trit)
{
    uint64_t mask = 0x3ULL << (2U * idx);
    uint64_t bits = (uint64_t)ternary_trit_to_bits(trit) << (2U * idx);
    return (packed & ~mask) | bits;
}

static uint64_t ternary_tritwise_op(uint64_t a, uint64_t b, unsigned trit_count, int op)
{
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

static uint64_t ternary_shift_left(uint64_t packed, unsigned trit_count, unsigned shift)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = 0;
        if (i >= shift)
            trit = ternary_get_trit(packed, i - shift);
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static uint64_t ternary_shift_right(uint64_t packed, unsigned trit_count, unsigned shift)
{
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

static uint64_t ternary_rotate_left(uint64_t packed, unsigned trit_count, unsigned shift)
{
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

static uint64_t ternary_rotate_right(uint64_t packed, unsigned trit_count, unsigned shift)
{
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

int __ternary_select_i8(TERNARY_COND_T cond, int true_val, int false_val)
{
    return cond ? true_val : false_val;
}

int __ternary_select_i16(TERNARY_COND_T cond, int true_val, int false_val)
{
    return cond ? true_val : false_val;
}

int __ternary_select_i32(TERNARY_COND_T cond, int true_val, int false_val)
{
    return cond ? true_val : false_val;
}

long long __ternary_select_i64(TERNARY_COND_T cond, long long true_val, long long false_val)
{
    return cond ? true_val : false_val;
}

unsigned int __ternary_select_u8(TERNARY_COND_T cond, unsigned int true_val,
                                 unsigned int false_val)
{
    return cond ? true_val : false_val;
}

unsigned int __ternary_select_u16(TERNARY_COND_T cond, unsigned int true_val,
                                  unsigned int false_val)
{
    return cond ? true_val : false_val;
}

unsigned int __ternary_select_u32(TERNARY_COND_T cond, unsigned int true_val,
                                  unsigned int false_val)
{
    return cond ? true_val : false_val;
}

unsigned long long __ternary_select_u64(TERNARY_COND_T cond, unsigned long long true_val,
                                        unsigned long long false_val)
{
    return cond ? true_val : false_val;
}

float __ternary_select_f32(TERNARY_COND_T cond, float true_val, float false_val)
{
    return cond ? true_val : false_val;
}

double __ternary_select_f64(TERNARY_COND_T cond, double true_val, double false_val)
{
    return cond ? true_val : false_val;
}

int __ternary_add(int a, int b)
{
    return a + b;
}

int __ternary_mul(int a, int b)
{
    return a * b;
}

int __ternary_not(int a)
{
    return -a;
}

int __ternary_and(int a, int b)
{
    return (a < b) ? a : b;
}

int __ternary_or(int a, int b)
{
    return (a > b) ? a : b;
}

int __ternary_xor(int a, int b)
{
    return ternary_trit_xor(a, b);
}

int __ternary_sub(int a, int b)
{
    return a - b;
}

int __ternary_div(int a, int b)
{
    return a / b;
}

int __ternary_mod(int a, int b)
{
    return a % b;
}

int __ternary_neg(int a)
{
    return -a;
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

#define DEFINE_TERNARY_TYPE_OPS(TRITS, TYPE, SUFFIX) \
    TYPE __ternary_select_t##SUFFIX(TERNARY_COND_T cond, TYPE true_val, TYPE false_val) \
    { \
        return cond ? true_val : false_val; \
    } \
    TYPE __ternary_add_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        return (TYPE)ternary_encode(va + vb, TRITS); \
    } \
    TYPE __ternary_mul_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        return (TYPE)ternary_encode(va * vb, TRITS); \
    } \
    TYPE __ternary_not_t##SUFFIX(TYPE a) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        return (TYPE)ternary_encode(-va, TRITS); \
    } \
    TYPE __ternary_sub_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        return (TYPE)ternary_encode(va - vb, TRITS); \
    } \
    TYPE __ternary_div_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        return (TYPE)ternary_encode(vb == 0 ? 0 : va / vb, TRITS); \
    } \
    TYPE __ternary_mod_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        return (TYPE)ternary_encode(vb == 0 ? 0 : va % vb, TRITS); \
    } \
    TYPE __ternary_neg_t##SUFFIX(TYPE a) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        return (TYPE)ternary_encode(-va, TRITS); \
    } \
    TYPE __ternary_and_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)ternary_tritwise_op((uint64_t)a, (uint64_t)b, TRITS, 0); \
    } \
    TYPE __ternary_or_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)ternary_tritwise_op((uint64_t)a, (uint64_t)b, TRITS, 1); \
    } \
    TYPE __ternary_xor_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)ternary_tritwise_op((uint64_t)a, (uint64_t)b, TRITS, 2); \
    } \
    TYPE __ternary_shl_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ternary_shift_left((uint64_t)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_shr_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ternary_shift_right((uint64_t)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_rol_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ternary_rotate_left((uint64_t)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_ror_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ternary_rotate_right((uint64_t)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_tb2t_t##SUFFIX(int64_t v) \
    { \
        return (TYPE)ternary_encode(v, TRITS); \
    } \
    int64_t __ternary_tt2b_t##SUFFIX(TYPE v) \
    { \
        return ternary_decode((uint64_t)v, TRITS); \
    } \
    float __ternary_t2f32_t##SUFFIX(TYPE v) \
    { \
        return (float)ternary_decode((uint64_t)v, TRITS); \
    } \
    double __ternary_t2f64_t##SUFFIX(TYPE v) \
    { \
        return (double)ternary_decode((uint64_t)v, TRITS); \
    } \
    TYPE __ternary_f2t32_t##SUFFIX(float v) \
    { \
        return (TYPE)ternary_encode((int64_t)v, TRITS); \
    } \
    TYPE __ternary_f2t64_t##SUFFIX(double v) \
    { \
        return (TYPE)ternary_encode((int64_t)v, TRITS); \
    } \
    int __ternary_cmp_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = ternary_decode((uint64_t)a, TRITS); \
        int64_t vb = ternary_decode((uint64_t)b, TRITS); \
        if (va < vb) \
            return -1; \
        if (va > vb) \
            return 1; \
        return 0; \
    }

DEFINE_TERNARY_TYPE_OPS(6, t6_t, 6)
DEFINE_TERNARY_TYPE_OPS(12, t12_t, 12)
DEFINE_TERNARY_TYPE_OPS(24, t24_t, 24)

#undef DEFINE_TERNARY_TYPE_OPS
