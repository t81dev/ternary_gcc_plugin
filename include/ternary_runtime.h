#ifndef TERNARY_RUNTIME_H
#define TERNARY_RUNTIME_H

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

#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint16_t t6_t;   /* 6 trits -> 12 bits */
typedef uint32_t t12_t;  /* 12 trits -> 24 bits */
typedef uint64_t t24_t;  /* 24 trits -> 48 bits */
#endif

/* Select helpers for standard types. */
int __ternary_select_i8(TERNARY_COND_T cond, int true_val, int false_val);
int __ternary_select_i16(TERNARY_COND_T cond, int true_val, int false_val);
int __ternary_select_i32(TERNARY_COND_T cond, int true_val, int false_val);
long long __ternary_select_i64(TERNARY_COND_T cond, long long true_val, long long false_val);
unsigned int __ternary_select_u8(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val);
unsigned int __ternary_select_u16(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val);
unsigned int __ternary_select_u32(TERNARY_COND_T cond, unsigned int true_val, unsigned int false_val);
unsigned long long __ternary_select_u64(TERNARY_COND_T cond, unsigned long long true_val,
                                        unsigned long long false_val);
float __ternary_select_f32(TERNARY_COND_T cond, float true_val, float false_val);
double __ternary_select_f64(TERNARY_COND_T cond, double true_val, double false_val);

/* Select helpers for ternary packed types. */
t6_t __ternary_select_t6(TERNARY_COND_T cond, t6_t true_val, t6_t false_val);
t12_t __ternary_select_t12(TERNARY_COND_T cond, t12_t true_val, t12_t false_val);
t24_t __ternary_select_t24(TERNARY_COND_T cond, t24_t true_val, t24_t false_val);

/* Integer ternary ops (balanced ternary semantics). */
int __ternary_add(int a, int b);
int __ternary_mul(int a, int b);
int __ternary_not(int a);
int __ternary_and(int a, int b);
int __ternary_or(int a, int b);
int __ternary_xor(int a, int b);
int __ternary_sub(int a, int b);
int __ternary_div(int a, int b);
int __ternary_mod(int a, int b);
int __ternary_neg(int a);
int __ternary_shl(int a, int shift);
int __ternary_shr(int a, int shift);
int __ternary_rol(int a, int shift);
int __ternary_ror(int a, int shift);
int __ternary_cmp(int a, int b);

/* Packed ternary ops for t6/t12/t24. */
t6_t __ternary_add_t6(t6_t a, t6_t b);
t6_t __ternary_mul_t6(t6_t a, t6_t b);
t6_t __ternary_not_t6(t6_t a);
t6_t __ternary_sub_t6(t6_t a, t6_t b);
t6_t __ternary_div_t6(t6_t a, t6_t b);
t6_t __ternary_mod_t6(t6_t a, t6_t b);
t6_t __ternary_neg_t6(t6_t a);
t6_t __ternary_and_t6(t6_t a, t6_t b);
t6_t __ternary_or_t6(t6_t a, t6_t b);
t6_t __ternary_xor_t6(t6_t a, t6_t b);
t6_t __ternary_shl_t6(t6_t a, int shift);
t6_t __ternary_shr_t6(t6_t a, int shift);
t6_t __ternary_rol_t6(t6_t a, int shift);
t6_t __ternary_ror_t6(t6_t a, int shift);
t6_t __ternary_tb2t_t6(int64_t v);
int64_t __ternary_tt2b_t6(t6_t v);
float __ternary_t2f32_t6(t6_t v);
double __ternary_t2f64_t6(t6_t v);
t6_t __ternary_f2t32_t6(float v);
t6_t __ternary_f2t64_t6(double v);
int __ternary_cmp_t6(t6_t a, t6_t b);

t12_t __ternary_add_t12(t12_t a, t12_t b);
t12_t __ternary_mul_t12(t12_t a, t12_t b);
t12_t __ternary_not_t12(t12_t a);
t12_t __ternary_sub_t12(t12_t a, t12_t b);
t12_t __ternary_div_t12(t12_t a, t12_t b);
t12_t __ternary_mod_t12(t12_t a, t12_t b);
t12_t __ternary_neg_t12(t12_t a);
t12_t __ternary_and_t12(t12_t a, t12_t b);
t12_t __ternary_or_t12(t12_t a, t12_t b);
t12_t __ternary_xor_t12(t12_t a, t12_t b);
t12_t __ternary_shl_t12(t12_t a, int shift);
t12_t __ternary_shr_t12(t12_t a, int shift);
t12_t __ternary_rol_t12(t12_t a, int shift);
t12_t __ternary_ror_t12(t12_t a, int shift);
t12_t __ternary_tb2t_t12(int64_t v);
int64_t __ternary_tt2b_t12(t12_t v);
float __ternary_t2f32_t12(t12_t v);
double __ternary_t2f64_t12(t12_t v);
t12_t __ternary_f2t32_t12(float v);
t12_t __ternary_f2t64_t12(double v);
int __ternary_cmp_t12(t12_t a, t12_t b);

t24_t __ternary_add_t24(t24_t a, t24_t b);
t24_t __ternary_mul_t24(t24_t a, t24_t b);
t24_t __ternary_not_t24(t24_t a);
t24_t __ternary_sub_t24(t24_t a, t24_t b);
t24_t __ternary_div_t24(t24_t a, t24_t b);
t24_t __ternary_mod_t24(t24_t a, t24_t b);
t24_t __ternary_neg_t24(t24_t a);
t24_t __ternary_and_t24(t24_t a, t24_t b);
t24_t __ternary_or_t24(t24_t a, t24_t b);
t24_t __ternary_xor_t24(t24_t a, t24_t b);
t24_t __ternary_shl_t24(t24_t a, int shift);
t24_t __ternary_shr_t24(t24_t a, int shift);
t24_t __ternary_rol_t24(t24_t a, int shift);
t24_t __ternary_ror_t24(t24_t a, int shift);
t24_t __ternary_tb2t_t24(int64_t v);
int64_t __ternary_tt2b_t24(t24_t v);
float __ternary_t2f32_t24(t24_t v);
double __ternary_t2f64_t24(t24_t v);
t24_t __ternary_f2t32_t24(float v);
t24_t __ternary_f2t64_t24(double v);
int __ternary_cmp_t24(t24_t a, t24_t b);

#ifdef __cplusplus
}
#endif

#endif /* TERNARY_RUNTIME_H */
