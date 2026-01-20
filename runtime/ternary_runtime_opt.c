#include "ternary_runtime.h"

// Optimized runtime with inline assembly (x86_64 example)
// Fallback to C if not supported

#ifdef __x86_64__

int __ternary_add(int a, int b) {
    int result;
    __asm__ (
        "add %1, %0"  // Placeholder: assume ternary add maps to add
        : "=r"(result)
        : "r"(a), "0"(b)
    );
    return result;
}

#else

// Fallback to C
int __ternary_add(int a, int b) {
    return a + b;  // Simplified
}

#endif