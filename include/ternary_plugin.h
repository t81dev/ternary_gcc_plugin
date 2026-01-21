#ifndef TERNARY_PLUGIN_H
#define TERNARY_PLUGIN_H

#include <stdint.h>

// Scalar ternary types (when plugin is enabled, these are builtin; otherwise typedefs)
#ifndef TERNARY_USE_BUILTIN_TYPES
typedef uint64_t t32_t;            // 32 trits, 64 bits
typedef unsigned __int128 t64_t;   // 64 trits, 128 bits (GCC extension)
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 256 && !defined(__cplusplus)
typedef unsigned _BitInt(256) t128_t;  // 128 trits, 256 bits (C-only fallback)
#endif
#endif

// Vector ternary types (packed)
typedef uint64_t v2t32_t;   // 2 x t32
typedef uint64_t v4t32_t;   // 4 x t32
typedef unsigned __int128 v2t64_t;  // 2 x t64
typedef unsigned __int128 v4t64_t;  // 4 x t64

// Balanced-ternary string literals (requires runtime helpers)
extern t32_t __ternary_bt_str_t32(const char *s);
extern t64_t __ternary_bt_str_t64(const char *s);
#define T32_LITERAL(s) __ternary_bt_str_t32(s)
#define T64_LITERAL(s) __ternary_bt_str_t64(s)
#define __ternary(N) t##N##_t

// Vector builtins (placeholders)
extern v2t32_t __builtin_ternary_add_v2t32(v2t32_t a, v2t32_t b);
extern v4t64_t __builtin_ternary_mul_v4t64(v4t64_t a, v4t64_t b);

#endif
