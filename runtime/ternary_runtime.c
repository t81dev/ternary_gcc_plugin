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

static int ternary_parse_bt_str(const char *s, int64_t *out)
{
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

static int ternary_get_trit_u128(unsigned __int128 packed, unsigned idx);
static unsigned __int128 ternary_set_trit_u128(unsigned __int128 packed, unsigned idx, int trit);
static int64_t ternary_decode_u128(unsigned __int128 packed, unsigned trit_count);
static unsigned __int128 ternary_encode_u128(int64_t value, unsigned trit_count);
static int ternary_signjmp_u128(unsigned __int128 packed, unsigned trit_count,
                                int neg_target, int zero_target, int pos_target);

static int ternary_trit_majority(int a, int b, int c)
{
    if (a == b || a == c)
        return a;
    if (b == c)
        return b;
    return 0;
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
        int trit = ternary_get_trit(packed, i);
        int inverted = (trit == 0) ? 0 : -trit;
        out = ternary_set_trit(out, i, inverted);
    }
    return out;
}

static int64_t ternary_muladd_scalar(int64_t a, int64_t b, int64_t c)
{
    return a * b + c;
}

static uint64_t ternary_muladd_u64(uint64_t a, uint64_t b, uint64_t c, unsigned trit_count)
{
    int64_t va = ternary_decode(a, trit_count);
    int64_t vb = ternary_decode(b, trit_count);
    int64_t vc = ternary_decode(c, trit_count);
    return ternary_encode(ternary_muladd_scalar(va, vb, vc), trit_count);
}

static uint64_t ternary_round_u64(uint64_t packed, unsigned trit_count, unsigned drop)
{
    if (drop >= trit_count)
        return 0;
    int64_t value = ternary_decode(packed, trit_count);
    int64_t divisor = 1;
    for (unsigned i = 0; i < drop; ++i)
        divisor *= 3;
    return ternary_encode(value / divisor, trit_count);
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

static unsigned __int128 ternary_muladd_u128(unsigned __int128 a, unsigned __int128 b,
                                             unsigned __int128 c, unsigned trit_count)
{
    int64_t va = ternary_decode_u128(a, trit_count);
    int64_t vb = ternary_decode_u128(b, trit_count);
    int64_t vc = ternary_decode_u128(c, trit_count);
    return ternary_encode_u128(ternary_muladd_scalar(va, vb, vc), trit_count);
}

static unsigned __int128 ternary_round_u128(unsigned __int128 packed, unsigned trit_count,
                                            unsigned drop)
{
    if (drop >= trit_count)
        return 0;
    int64_t value = ternary_decode_u128(packed, trit_count);
    int64_t divisor = 1;
    for (unsigned i = 0; i < drop; ++i)
        divisor *= 3;
    return ternary_encode_u128(value / divisor, trit_count);
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

static unsigned __int128 ternary_tbias_u128(unsigned __int128 packed, unsigned trit_count, int64_t bias)
{
    int64_t value = ternary_decode_u128(packed, trit_count);
    return ternary_encode_u128(value + bias, trit_count);
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

static uint64_t ternary_tbias_u64(uint64_t packed, unsigned trit_count, int64_t bias)
{
    int64_t value = ternary_decode(packed, trit_count);
    return ternary_encode(value + bias, trit_count);
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
    int64_t value = ternary_decode(packed, trit_count);
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

static int ternary_quantize_scalar(float value, float threshold)
{
    if (value > threshold)
        return 1;
    if (value < -threshold)
        return -1;
    return 0;
}

static int ternary_quantize_scalar_d(double value, double threshold)
{
    if (value > threshold)
        return 1;
    if (value < -threshold)
        return -1;
    return 0;
}

static uint64_t ternary_majority_u64(uint64_t a, uint64_t b, uint64_t c, unsigned trit_count)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_majority(ternary_get_trit(a, i),
                                          ternary_get_trit(b, i),
                                          ternary_get_trit(c, i));
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static uint64_t ternary_implication_u64(uint64_t a, uint64_t b, unsigned trit_count)
{
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int trit = ternary_trit_implication(ternary_get_trit(a, i),
                                             ternary_get_trit(b, i));
        out = ternary_set_trit(out, i, trit);
    }
    return out;
}

static uint64_t ternary_quantize_vector(float value, float threshold, unsigned trit_count)
{
    int trit = ternary_quantize_scalar(value, threshold);
    uint64_t result = 0;
    for (unsigned i = 0; i < trit_count; ++i)
        result = ternary_set_trit(result, i, 0);
    if (trit_count > 0)
        result = ternary_set_trit(result, 0, trit);
    return result;
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

static unsigned __int128 ternary_quantize_vector_d(double value, double threshold,
                                                   unsigned trit_count)
{
    int trit = ternary_quantize_scalar_d(value, threshold);
    unsigned __int128 result = 0;
    for (unsigned i = 0; i < trit_count; ++i)
        result = ternary_set_trit_u128(result, i, 0);
    if (trit_count > 0)
        result = ternary_set_trit_u128(result, 0, trit);
    return result;
}

static uint64_t ternary_shift_left(uint64_t packed, unsigned trit_count, unsigned shift)
{
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

static uint64_t ternary_shift_right(uint64_t packed, unsigned trit_count, unsigned shift)
{
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

t32_t __ternary_bt_str_t32(const char *s)
{
    int64_t value = 0;
    if (!ternary_parse_bt_str(s, &value))
        return 0;
    return (t32_t)ternary_encode(value, 32);
}

t64_t __ternary_bt_str_t64(const char *s)
{
    int64_t value = 0;
    if (!ternary_parse_bt_str(s, &value))
        return 0;
    return (t64_t)ternary_encode_u128(value, 64);
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
    return cmp == -1 || cmp == 0 ? 1 : 0;
}

int __ternary_gt(int a, int b)
{
    return __ternary_cmp(a, b) == 1 ? 1 : 0;
}

int __ternary_ge(int a, int b)
{
    int cmp = __ternary_cmp(a, b);
    return cmp == 1 || cmp == 0 ? 1 : 0;
}

#define DEFINE_TERNARY_TYPE_OPS(TRITS, TYPE, SUFFIX, PACK_T, DECODE, ENCODE, TRIT_OP, SHL, SHR, ROL, ROR) \
    TYPE __ternary_select_t##SUFFIX(TERNARY_COND_T cond, TYPE true_val, TYPE false_val) \
    { \
        return cond ? true_val : false_val; \
    } \
    TYPE __ternary_add_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        return (TYPE)ENCODE(va + vb, TRITS); \
    } \
    TYPE __ternary_mul_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        return (TYPE)ENCODE(va * vb, TRITS); \
    } \
    TYPE __ternary_not_t##SUFFIX(TYPE a) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        return (TYPE)ENCODE(-va, TRITS); \
    } \
    TYPE __ternary_sub_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        return (TYPE)ENCODE(va - vb, TRITS); \
    } \
    TYPE __ternary_div_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        return (TYPE)ENCODE(vb == 0 ? 0 : va / vb, TRITS); \
    } \
    TYPE __ternary_mod_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        return (TYPE)ENCODE(vb == 0 ? 0 : va % vb, TRITS); \
    } \
    TYPE __ternary_neg_t##SUFFIX(TYPE a) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        return (TYPE)ENCODE(-va, TRITS); \
    } \
    TYPE __ternary_and_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)TRIT_OP((PACK_T)a, (PACK_T)b, TRITS, 0); \
    } \
    TYPE __ternary_or_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)TRIT_OP((PACK_T)a, (PACK_T)b, TRITS, 1); \
    } \
    TYPE __ternary_xor_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return (TYPE)TRIT_OP((PACK_T)a, (PACK_T)b, TRITS, 2); \
    } \
    TYPE __ternary_shl_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)SHL((PACK_T)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_shr_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)SHR((PACK_T)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_rol_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ROL((PACK_T)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_ror_t##SUFFIX(TYPE a, int shift) \
    { \
        return (TYPE)ROR((PACK_T)a, TRITS, (unsigned)shift); \
    } \
    TYPE __ternary_tb2t_t##SUFFIX(int64_t v) \
    { \
        return (TYPE)ENCODE(v, TRITS); \
    } \
    int64_t __ternary_tt2b_t##SUFFIX(TYPE v) \
    { \
        return DECODE((PACK_T)v, TRITS); \
    } \
    float __ternary_t2f32_t##SUFFIX(TYPE v) \
    { \
        return (float)DECODE((PACK_T)v, TRITS); \
    } \
    double __ternary_t2f64_t##SUFFIX(TYPE v) \
    { \
        return (double)DECODE((PACK_T)v, TRITS); \
    } \
    TYPE __ternary_f2t32_t##SUFFIX(float v) \
    { \
        return (TYPE)ENCODE((int64_t)v, TRITS); \
    } \
    TYPE __ternary_f2t64_t##SUFFIX(double v) \
    { \
        return (TYPE)ENCODE((int64_t)v, TRITS); \
    } \
    int __ternary_cmp_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int64_t va = DECODE((PACK_T)a, TRITS); \
        int64_t vb = DECODE((PACK_T)b, TRITS); \
        if (va < vb) \
            return -1; \
        if (va > vb) \
            return 1; \
        return 0; \
    } \
    int __ternary_eq_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return __ternary_cmp_t##SUFFIX(a, b) == 0 ? 1 : 0; \
    } \
    int __ternary_ne_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return __ternary_cmp_t##SUFFIX(a, b) != 0 ? 1 : 0; \
    } \
    int __ternary_lt_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return __ternary_cmp_t##SUFFIX(a, b) == -1 ? 1 : 0; \
    } \
    int __ternary_le_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return cmp == -1 || cmp == 0 ? 1 : 0; \
    } \
    int __ternary_gt_t##SUFFIX(TYPE a, TYPE b) \
    { \
        return __ternary_cmp_t##SUFFIX(a, b) == 1 ? 1 : 0; \
    } \
    int __ternary_ge_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return cmp == 1 || cmp == 0 ? 1 : 0; \
    } \
    /* Ternary-specific comparison operations returning ternary results */ \
    TYPE __ternary_cmplt_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return (TYPE)ENCODE(cmp == -1 ? -1 : 0, TRITS); \
    } \
    TYPE __ternary_cmpeq_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return (TYPE)ENCODE(cmp == 0 ? 1 : 0, TRITS); \
    } \
    TYPE __ternary_cmpgt_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return (TYPE)ENCODE(cmp == 1 ? 1 : 0, TRITS); \
    } \
    TYPE __ternary_cmpneq_t##SUFFIX(TYPE a, TYPE b) \
    { \
        int cmp = __ternary_cmp_t##SUFFIX(a, b); \
        return (TYPE)ENCODE(cmp != 0 ? 1 : 0, TRITS); \
    }

DEFINE_TERNARY_TYPE_OPS(32, t32_t, 32, uint64_t, ternary_decode, ternary_encode,
                        ternary_tritwise_op, ternary_shift_left, ternary_shift_right,
                        ternary_rotate_left, ternary_rotate_right)
DEFINE_TERNARY_TYPE_OPS(64, t64_t, 64, unsigned __int128, ternary_decode_u128, ternary_encode_u128,
                        ternary_tritwise_op_u128, ternary_shift_left_u128, ternary_shift_right_u128,
                        ternary_rotate_left_u128, ternary_rotate_right_u128)

t32_t __ternary_tmin_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op((uint64_t)a, (uint64_t)b, 32, 0);
}

t32_t __ternary_tmax_t32(t32_t a, t32_t b)
{
    return (t32_t)ternary_tritwise_op((uint64_t)a, (uint64_t)b, 32, 1);
}

t32_t __ternary_tmaj_t32(t32_t a, t32_t b, t32_t c)
{
    return (t32_t)ternary_majority_u64((uint64_t)a, (uint64_t)b, (uint64_t)c, 32);
}

t32_t __ternary_tlimp_t32(t32_t antecedent, t32_t consequent)
{
    return (t32_t)ternary_implication_u64((uint64_t)antecedent, (uint64_t)consequent, 32);
}

t32_t __ternary_tquant_t32(float value, float threshold)
{
    return (t32_t)ternary_quantize_vector(value, threshold, 32);
}

t32_t __ternary_tnot_t32(t32_t a)
{
    return (t32_t)ternary_not_u64((uint64_t)a, 32);
}

t32_t __ternary_tinv_t32(t32_t a)
{
    return __ternary_tnot_t32(a);
}

t32_t __ternary_tmuladd_t32(t32_t a, t32_t b, t32_t c)
{
    return (t32_t)ternary_muladd_u64((uint64_t)a, (uint64_t)b, (uint64_t)c, 32);
}

t32_t __ternary_tround_t32(t32_t a, unsigned drop)
{
    return (t32_t)ternary_round_u64((uint64_t)a, 32, drop);
}

t32_t __ternary_tnormalize_t32(t32_t a)
{
    return (t32_t)ternary_normalize_u64((uint64_t)a, 32);
}

t32_t __ternary_tbias_t32(t32_t a, int64_t bias)
{
    return (t32_t)ternary_tbias_u64((uint64_t)a, 32, bias);
}

t64_t __ternary_tmin_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128((unsigned __int128)a, (unsigned __int128)b, 64, 0);
}

t64_t __ternary_tmax_t64(t64_t a, t64_t b)
{
    return (t64_t)ternary_tritwise_op_u128((unsigned __int128)a, (unsigned __int128)b, 64, 1);
}

t64_t __ternary_tmaj_t64(t64_t a, t64_t b, t64_t c)
{
    return (t64_t)ternary_majority_u128((unsigned __int128)a, (unsigned __int128)b,
                                         (unsigned __int128)c, 64);
}

t64_t __ternary_tlimp_t64(t64_t antecedent, t64_t consequent)
{
    return (t64_t)ternary_implication_u128((unsigned __int128)antecedent,
                                            (unsigned __int128)consequent, 64);
}

t64_t __ternary_tquant_t64(double value, double threshold)
{
    return (t64_t)ternary_quantize_vector_d(value, threshold, 64);
}

t64_t __ternary_tnot_t64(t64_t a)
{
    return (t64_t)ternary_not_u128((unsigned __int128)a, 64);
}

t64_t __ternary_tinv_t64(t64_t a)
{
    return __ternary_tnot_t64(a);
}

t64_t __ternary_tmuladd_t64(t64_t a, t64_t b, t64_t c)
{
    return (t64_t)ternary_muladd_u128((unsigned __int128)a, (unsigned __int128)b,
                                       (unsigned __int128)c, 64);
}

t64_t __ternary_tround_t64(t64_t a, unsigned drop)
{
    return (t64_t)ternary_round_u128((unsigned __int128)a, 64, drop);
}

t64_t __ternary_tnormalize_t64(t64_t a)
{
    return (t64_t)ternary_normalize_u128((unsigned __int128)a, 64);
}

t64_t __ternary_tbias_t64(t64_t a, int64_t bias)
{
    return (t64_t)ternary_tbias_u128((unsigned __int128)a, 64, bias);
}

int __ternary_tbranch(TERNARY_COND_T cond, int neg_target, int zero_target, int pos_target)
{
    return ternary_branch_target(cond, neg_target, zero_target, pos_target);
}

int __ternary_tsignjmp_t32(t32_t reg, int neg_target, int zero_target, int pos_target)
{
    return ternary_signjmp_u64((uint64_t)reg, 32, neg_target, zero_target, pos_target);
}

int __ternary_tsignjmp_t64(t64_t reg, int neg_target, int zero_target, int pos_target)
{
    return ternary_signjmp_u128((unsigned __int128)reg, 64, neg_target, zero_target, pos_target);
}

t32_t __ternary_load_t32(const void *addr)
{
    return *(const t32_t *)addr;
}

void __ternary_store_t32(void *addr, t32_t value)
{
    *(t32_t *)addr = value;
}

t64_t __ternary_load_t64(const void *addr)
{
    return *(const t64_t *)addr;
}

void __ternary_store_t64(void *addr, t64_t value)
{
    *(t64_t *)addr = value;
}

/* Vector operations - SIMD accelerated ternary computations */

/* tv32_t operations (vector of 2 x t32_t) */
tv32_t __ternary_add_tv32(tv32_t a, tv32_t b)
{
    // Extract two t32_t values from the 128-bit vector
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    // Perform scalar operations
    t32_t r0 = __ternary_add_t32(a0, b0);
    t32_t r1 = __ternary_add_t32(a1, b1);
    
    // Pack back into 128-bit vector
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_sub_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_sub_t32(a0, b0);
    t32_t r1 = __ternary_sub_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_mul_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_mul_t32(a0, b0);
    t32_t r1 = __ternary_mul_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_and_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_and_t32(a0, b0);
    t32_t r1 = __ternary_and_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_or_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_or_t32(a0, b0);
    t32_t r1 = __ternary_or_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_xor_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_xor_t32(a0, b0);
    t32_t r1 = __ternary_xor_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_not_tv32(tv32_t a)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    
    t32_t r0 = __ternary_not_t32(a0);
    t32_t r1 = __ternary_not_t32(a1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_cmp_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    
    t32_t r0 = __ternary_cmplt_t32(a0, b0);
    t32_t r1 = __ternary_cmplt_t32(a1, b1);
    
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_tmin_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);

    t32_t r0 = __ternary_tmin_t32(a0, b0);
    t32_t r1 = __ternary_tmin_t32(a1, b1);
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_tmax_tv32(tv32_t a, tv32_t b)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);

    t32_t r0 = __ternary_tmax_t32(a0, b0);
    t32_t r1 = __ternary_tmax_t32(a1, b1);
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_tmaj_tv32(tv32_t a, tv32_t b, tv32_t c)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);
    t32_t b0 = (t32_t)(uint64_t)b;
    t32_t b1 = (t32_t)(uint64_t)(b >> 64);
    t32_t c0 = (t32_t)(uint64_t)c;
    t32_t c1 = (t32_t)(uint64_t)(c >> 64);

    t32_t r0 = __ternary_tmaj_t32(a0, b0, c0);
    t32_t r1 = __ternary_tmaj_t32(a1, b1, c1);
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_tlimp_tv32(tv32_t antecedent, tv32_t consequent)
{
    t32_t a0 = (t32_t)(uint64_t)antecedent;
    t32_t a1 = (t32_t)(uint64_t)(antecedent >> 64);
    t32_t b0 = (t32_t)(uint64_t)consequent;
    t32_t b1 = (t32_t)(uint64_t)(consequent >> 64);

    t32_t r0 = __ternary_tlimp_t32(a0, b0);
    t32_t r1 = __ternary_tlimp_t32(a1, b1);
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

tv32_t __ternary_tquant_tv32(float value, float threshold)
{
    t32_t lane = __ternary_tquant_t32(value, threshold);
    return ((tv32_t)(uint64_t)lane << 64) | (tv32_t)(uint64_t)lane;
}

tv32_t __ternary_tround_tv32(tv32_t a, int drop)
{
    t32_t a0 = (t32_t)(uint64_t)a;
    t32_t a1 = (t32_t)(uint64_t)(a >> 64);

    t32_t r0 = __ternary_tround_t32(a0, drop);
    t32_t r1 = __ternary_tround_t32(a1, drop);
    return ((tv32_t)(uint64_t)r1 << 64) | (tv32_t)(uint64_t)r0;
}

/* tv64_t operations (vector of 2 x t64_t) - TODO: Implement for struct type */
tv64_t __ternary_add_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_add_t64(a.lo, b.lo);
    result.hi = __ternary_add_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_sub_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_sub_t64(a.lo, b.lo);
    result.hi = __ternary_sub_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_mul_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_mul_t64(a.lo, b.lo);
    result.hi = __ternary_mul_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_and_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_and_t64(a.lo, b.lo);
    result.hi = __ternary_and_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_or_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_or_t64(a.lo, b.lo);
    result.hi = __ternary_or_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_xor_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_xor_t64(a.lo, b.lo);
    result.hi = __ternary_xor_t64(a.hi, b.hi);
    return result;
}

tv64_t __ternary_not_tv64(tv64_t a)
{
    tv64_t result;
    result.lo = __ternary_not_t64(a.lo);
    result.hi = __ternary_not_t64(a.hi);
    return result;
}

tv64_t __ternary_cmp_tv64(tv64_t a, tv64_t b)
{
    tv64_t result;
    result.lo = __ternary_cmplt_t64(a.lo, b.lo);
    result.hi = __ternary_cmplt_t64(a.hi, b.hi);
    return result;
}

#undef DEFINE_TERNARY_TYPE_OPS
