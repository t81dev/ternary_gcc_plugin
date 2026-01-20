#include <stdio.h>
#include "ternary_runtime.h"

int main() {
    // Test ABI: call runtime functions
    int a = 5; // Example value
    int b = 3;
    int result = __ternary_add(a, b);
    printf("ABI test: __ternary_add(5, 3) = %d\n", result);

    // Verify it's correct (assuming implementation)
    if (result == 8) {
        printf("ABI test passed\n");
        return 0;
    } else {
        printf("ABI test failed\n");
        return 1;
    }
}