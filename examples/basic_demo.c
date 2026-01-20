#include <stdio.h>
#include "ternary.h"

int main() {
    // Basic arithmetic
    t12_t a = 1;
    t12_t b = -1;
    t12_t c = a + b;
    printf("Basic: 1 + (-1) = %d\n", ternary_to_int(c));

    // Ternary multiply
    t12_t d = 2;
    t12_t e = 3;
    t12_t f = d * e;
    printf("Multiply: 2 * 3 = %d\n", ternary_to_int(f));

    // Mixed code
    int binary = 5;
    t12_t ternary = int_to_ternary(binary);
    t12_t result = ternary + 1;
    int back = ternary_to_int(result);
    printf("Mixed: 5 (binary) -> ternary -> +1 -> %d (binary)\n", back);

    return 0;
}