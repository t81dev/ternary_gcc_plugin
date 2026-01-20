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
typedef uint16_t t6_t;   /* 6 trits -> 12 bits */
typedef uint32_t t12_t;  /* 12 trits -> 24 bits */
typedef uint64_t t24_t;  /* 24 trits -> 48 bits */
#endif

// Vector types
typedef uint64_t v2t6_t;   // 2 x t6
typedef uint64_t v4t6_t;   // 4 x t6
typedef uint64_t v2t12_t;  // 2 x t12
typedef uint64_t v4t12_t;  // 4 x t12

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
extern t6_t __builtin_ternary_tb2t(int a);
extern int __builtin_ternary_tt2b(t6_t v);
extern float __builtin_ternary_t2f(t6_t v);
extern t6_t __builtin_ternary_f2t(float v);

// Vector builtins
extern v2t6_t __builtin_ternary_add_v2t6(v2t6_t a, v2t6_t b);
extern v4t12_t __builtin_ternary_mul_v4t12(v4t12_t a, v4t12_t b);

// Convenience macros for literals (placeholders)
#define TERNARY_LITERAL(x) ((t12_t)(x))

// Conversion macros
#define ternary_to_int(t) ((int)(t))
#define int_to_ternary(i) ((t12_t)(i))
#define ternary_to_float(t) ((float)(t))
#define float_to_ternary(f) ((t12_t)(f))

// Helper macros for operations
#define ternary_select(cond, t, f) ((cond) ? (t) : (f))
#define ternary_abs(x) ((x) < 0 ? -(x) : (x))

// Inline helper functions
static inline int ternary_is_zero(t12_t x) {
    return x == 0;
}

static inline int ternary_sign(t12_t x) {
    return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

#endif // TERNARY_H