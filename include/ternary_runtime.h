#ifndef TERNARY_RUNTIME_H
#define TERNARY_RUNTIME_H

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TERNARY_COND_T
typedef int64_t ternary_cond_t;
#define TERNARY_COND_T ternary_cond_t
#endif

#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint64_t t32_t;           /* 32 trits -> 64 bits */
typedef unsigned __int128 t64_t;  /* 64 trits -> 128 bits */
typedef unsigned __int128 tv32_t; /* vector of 2 x t32_t (128 bits) */
typedef struct { unsigned __int128 lo, hi; } tv64_t; /* vector of 2 x t64_t (256 bits) */
typedef struct { unsigned __int128 lo, hi; } tv128_t; /* vector of 2 x t128_t (512 bits) */
#endif

/* Varargs helpers for ternary packed types. */
#define TERNARY_VA_ARG_T32(ap) ((t32_t)va_arg(ap, uint64_t))
#define TERNARY_VA_ARG_T64(ap) ((t64_t)va_arg(ap, unsigned __int128))

/* Note: t128 helpers are not provided in this header. */

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
t32_t __ternary_select_t32(TERNARY_COND_T cond, t32_t true_val, t32_t false_val);
t64_t __ternary_select_t64(TERNARY_COND_T cond, t64_t true_val, t64_t false_val);

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

/* Packed ternary ops for t32/t64. */
t32_t __ternary_add_t32(t32_t a, t32_t b);
t32_t __ternary_mul_t32(t32_t a, t32_t b);
t32_t __ternary_not_t32(t32_t a);
t32_t __ternary_sub_t32(t32_t a, t32_t b);
t32_t __ternary_div_t32(t32_t a, t32_t b);
t32_t __ternary_mod_t32(t32_t a, t32_t b);
t32_t __ternary_neg_t32(t32_t a);
t32_t __ternary_and_t32(t32_t a, t32_t b);
t32_t __ternary_or_t32(t32_t a, t32_t b);
t32_t __ternary_xor_t32(t32_t a, t32_t b);
t32_t __ternary_shl_t32(t32_t a, int shift);
t32_t __ternary_shr_t32(t32_t a, int shift);
t32_t __ternary_rol_t32(t32_t a, int shift);
t32_t __ternary_ror_t32(t32_t a, int shift);
t32_t __ternary_tb2t_t32(int64_t v);
int64_t __ternary_tt2b_t32(t32_t v);
float __ternary_t2f32_t32(t32_t v);
double __ternary_t2f64_t32(t32_t v);
t32_t __ternary_f2t32_t32(float v);
t32_t __ternary_f2t64_t32(double v);
int __ternary_cmp_t32(t32_t a, t32_t b);
t32_t __ternary_bt_str_t32(const char *s);

/* Ternary-specific comparison operations (return ternary results) */
t32_t __ternary_cmplt_t32(t32_t a, t32_t b);
t32_t __ternary_cmpeq_t32(t32_t a, t32_t b);
t32_t __ternary_cmpgt_t32(t32_t a, t32_t b);
t32_t __ternary_cmpneq_t32(t32_t a, t32_t b);

t64_t __ternary_add_t64(t64_t a, t64_t b);
t64_t __ternary_mul_t64(t64_t a, t64_t b);
t64_t __ternary_not_t64(t64_t a);
t64_t __ternary_sub_t64(t64_t a, t64_t b);
t64_t __ternary_div_t64(t64_t a, t64_t b);
t64_t __ternary_mod_t64(t64_t a, t64_t b);
t64_t __ternary_neg_t64(t64_t a);
t64_t __ternary_and_t64(t64_t a, t64_t b);
t64_t __ternary_or_t64(t64_t a, t64_t b);
t64_t __ternary_xor_t64(t64_t a, t64_t b);
t64_t __ternary_shl_t64(t64_t a, int shift);
t64_t __ternary_shr_t64(t64_t a, int shift);
t64_t __ternary_rol_t64(t64_t a, int shift);
t64_t __ternary_ror_t64(t64_t a, int shift);
t64_t __ternary_tb2t_t64(int64_t v);
int64_t __ternary_tt2b_t64(t64_t v);
float __ternary_t2f32_t64(t64_t v);
double __ternary_t2f64_t64(t64_t v);
t64_t __ternary_f2t32_t64(float v);
t64_t __ternary_f2t64_t64(double v);
int __ternary_cmp_t64(t64_t a, t64_t b);
t64_t __ternary_bt_str_t64(const char *s);

/* Ternary-specific comparison operations for t64 (return ternary results) */
t64_t __ternary_cmplt_t64(t64_t a, t64_t b);
t64_t __ternary_cmpeq_t64(t64_t a, t64_t b);
t64_t __ternary_cmpgt_t64(t64_t a, t64_t b);
t64_t __ternary_cmpneq_t64(t64_t a, t64_t b);

/* Memory operations (tld/tst) */
t32_t __ternary_load_t32(const void *addr);
void __ternary_store_t32(void *addr, t32_t value);
t64_t __ternary_load_t64(const void *addr);
void __ternary_store_t64(void *addr, t64_t value);

/* Vector operations - SIMD accelerated ternary computations */
tv32_t __ternary_add_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_sub_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_mul_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_and_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_or_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_xor_tv32(tv32_t a, tv32_t b);
tv32_t __ternary_not_tv32(tv32_t a);
tv32_t __ternary_cmp_tv32(tv32_t a, tv32_t b);

tv64_t __ternary_add_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_sub_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_mul_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_and_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_or_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_xor_tv64(tv64_t a, tv64_t b);
tv64_t __ternary_not_tv64(tv64_t a);
tv64_t __ternary_cmp_tv64(tv64_t a, tv64_t b);

#ifdef __cplusplus
}
#endif

#endif /* TERNARY_RUNTIME_H */
