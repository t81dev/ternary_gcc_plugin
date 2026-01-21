#ifndef TERNARY_H
#define TERNARY_H

#include <stdint.h>
#include "ternary_plugin.h"
#include "ternary_runtime.h"

// Ternary constants
#define TERNARY_TRIT_NEG -1
#define TERNARY_TRIT_ZERO 0
#define TERNARY_TRIT_POS 1

// Ternary types (from runtime)
#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint64_t t32_t;           /* 32 trits -> 64 bits */
typedef unsigned __int128 t64_t;  /* 64 trits -> 128 bits */
#endif

// Vector types - packed ternary vectors for SIMD operations
#ifndef TERNARY_USE_BUILTIN_TYPES
typedef unsigned __int128 tv32_t; /* vector of 2 x t32_t (128 bits) */
#endif

// Builtin function declarations (for plugin lowering)
extern int __builtin_ternary_add(int a, int b);
extern int __builtin_ternary_mul(int a, int b);
extern int __builtin_ternary_sub(int a, int b);
extern int __builtin_ternary_div(int a, int b);
extern int __builtin_ternary_mod(int a, int b);
extern int __builtin_ternary_neg(int a);
extern int __builtin_ternary_not(int a);
extern int __builtin_ternary_and(int a, int b);
extern int __builtin_ternary_or(int a, int b);
extern int __builtin_ternary_xor(int a, int b);
extern int __builtin_ternary_cmp(int a, int b);
extern int __builtin_ternary_shl(int a, int shift);
extern int __builtin_ternary_shr(int a, int shift);
extern int __builtin_ternary_rol(int a, int shift);
extern int __builtin_ternary_ror(int a, int shift);
extern t32_t __builtin_ternary_tb2t(int a);
extern int __builtin_ternary_tt2b(t32_t v);
extern float __builtin_ternary_t2f(t32_t v);
extern t32_t __builtin_ternary_f2t(float v);

// Ternary-specific comparison builtins (return ternary results)
extern t32_t __builtin_ternary_cmplt(t32_t a, t32_t b);
extern t32_t __builtin_ternary_cmpeq(t32_t a, t32_t b);
extern t32_t __builtin_ternary_cmpgt(t32_t a, t32_t b);
extern t32_t __builtin_ternary_cmpneq(t32_t a, t32_t b);
extern t64_t __builtin_ternary_cmplt_t64(t64_t a, t64_t b);
extern t64_t __builtin_ternary_cmpeq_t64(t64_t a, t64_t b);
extern t64_t __builtin_ternary_cmpgt_t64(t64_t a, t64_t b);
extern t64_t __builtin_ternary_cmpneq_t64(t64_t a, t64_t b);

// Memory operations (tld/tst)
extern t32_t __builtin_ternary_load_t32(const void *addr);
extern void __builtin_ternary_store_t32(void *addr, t32_t value);
extern t64_t __builtin_ternary_load_t64(const void *addr);
extern void __builtin_ternary_store_t64(void *addr, t64_t value);

// Vector operations - SIMD accelerated ternary computations
extern tv32_t __builtin_ternary_add_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_sub_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_mul_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_and_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_or_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_xor_tv32(tv32_t a, tv32_t b);
extern tv32_t __builtin_ternary_not_tv32(tv32_t a);
extern tv32_t __builtin_ternary_cmp_tv32(tv32_t a, tv32_t b);

extern tv64_t __builtin_ternary_add_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_sub_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_mul_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_and_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_or_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_xor_tv64(tv64_t a, tv64_t b);
extern tv64_t __builtin_ternary_not_tv64(tv64_t a);
extern tv64_t __builtin_ternary_cmp_tv64(tv64_t a, tv64_t b);

// Balanced-ternary string literals (e.g. "1 0 -1 1")
#define T32_BT_STR(s) __ternary_bt_str_t32(s)
#define T64_BT_STR(s) __ternary_bt_str_t64(s)
#define TERNARY_LITERAL(s) __ternary_bt_str_t32(s)

// Conversion macros
#define ternary_to_int(t) ((int)(t))
#define int_to_ternary(i) ((t32_t)(i))
#define ternary_to_float(t) ((float)(t))
#define float_to_ternary(f) ((t32_t)(f))

// Helper macros for operations
#define ternary_select(cond, t, f) ((cond) ? (t) : (f))
#define ternary_abs(x) ((x) < 0 ? -(x) : (x))

// Inline helper functions
static inline int ternary_is_zero(t32_t x) {
    return x == 0;
}

static inline int ternary_sign(t32_t x) {
    return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

#endif // TERNARY_H
