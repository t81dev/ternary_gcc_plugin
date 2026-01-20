#ifndef TERNARY_PLUGIN_H
#define TERNARY_PLUGIN_H

#include <stdint.h>

// Vector ternary types (packed)
typedef uint64_t v2t6_t;   // 2 x t6
typedef uint64_t v4t6_t;   // 4 x t6
typedef uint64_t v2t12_t;  // 2 x t12
typedef uint64_t v4t12_t;  // 4 x t12

// Vector builtins (placeholders)
extern v2t6_t __builtin_ternary_add_v2t6(v2t6_t a, v2t6_t b);
extern v4t12_t __builtin_ternary_mul_v4t12(v4t12_t a, v4t12_t b);

#endif