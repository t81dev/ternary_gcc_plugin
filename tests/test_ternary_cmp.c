#include <stdio.h>
#include <stdint.h>
#include "include/ternary.h"
#include "include/ternary_runtime.h"

// Test for ternary-specific comparison operations
// These return ternary results (-1, 0, +1) instead of binary (0, 1)

void test_ternary_comparisons() {
    printf("Testing ternary-specific comparison operations\n");

    // Test with t32 values
    t32_t a = __ternary_tb2t_t32(5);   // 5 in ternary
    t32_t b = __ternary_tb2t_t32(3);   // 3 in ternary
    t32_t c = __ternary_tb2t_t32(5);   // 5 in ternary (same as a)

    printf("a = 5, b = 3, c = 5 (in ternary representation)\n");

    // Test cmplt: should return -1 if a < b, 0 otherwise
    t32_t result_cmplt_ab = __ternary_cmplt_t32(a, b);  // 5 < 3? No -> 0
    t32_t result_cmplt_ba = __ternary_cmplt_t32(b, a);  // 3 < 5? Yes -> -1

    // Test cmpeq: should return +1 if equal, 0 otherwise
    t32_t result_cmpeq_ac = __ternary_cmpeq_t32(a, c);  // 5 == 5? Yes -> +1
    t32_t result_cmpeq_ab = __ternary_cmpeq_t32(a, b);  // 5 == 3? No -> 0

    // Test cmpgt: should return +1 if a > b, 0 otherwise
    t32_t result_cmpgt_ab = __ternary_cmpgt_t32(a, b);  // 5 > 3? Yes -> +1
    t32_t result_cmpgt_ba = __ternary_cmpgt_t32(b, a);  // 3 > 5? No -> 0

    // Test cmpneq: should return +1 if not equal, 0 otherwise
    t32_t result_cmpneq_ab = __ternary_cmpneq_t32(a, b);  // 5 != 3? Yes -> +1
    t32_t result_cmpneq_ac = __ternary_cmpneq_t32(a, c);  // 5 != 5? No -> 0

    // Convert results back to binary for display
    printf("cmplt(a,b) = %d (expected 0)\n", __ternary_tt2b_t32(result_cmplt_ab));
    printf("cmplt(b,a) = %d (expected -1)\n", __ternary_tt2b_t32(result_cmplt_ba));
    printf("cmpeq(a,c) = %d (expected 1)\n", __ternary_tt2b_t32(result_cmpeq_ac));
    printf("cmpeq(a,b) = %d (expected 0)\n", __ternary_tt2b_t32(result_cmpeq_ab));
    printf("cmpgt(a,b) = %d (expected 1)\n", __ternary_tt2b_t32(result_cmpgt_ab));
    printf("cmpgt(b,a) = %d (expected 0)\n", __ternary_tt2b_t32(result_cmpgt_ba));
    printf("cmpneq(a,b) = %d (expected 1)\n", __ternary_tt2b_t32(result_cmpneq_ab));
    printf("cmpneq(a,c) = %d (expected 0)\n", __ternary_tt2b_t32(result_cmpneq_ac));

    // Test with t64 values
    t64_t a64 = (t64_t)5;
    t64_t b64 = (t64_t)3;
    t64_t c64 = (t64_t)5;

    t64_t result64_cmplt = __ternary_cmplt_t64(a64, b64);
    t64_t result64_cmpeq = __ternary_cmpeq_t64(a64, c64);
    t64_t result64_cmpgt = __ternary_cmpgt_t64(a64, b64);
    t64_t result64_cmpneq = __ternary_cmpneq_t64(a64, b64);

    printf("\nTesting t64 comparisons:\n");
    printf("cmplt(a64,b64) = %lld (decoded: %lld)\n", (long long)result64_cmplt, __ternary_tt2b_t64(result64_cmplt));
    printf("cmpeq(a64,c64) = %lld (decoded: %lld)\n", (long long)result64_cmpeq, __ternary_tt2b_t64(result64_cmpeq));
    printf("cmpgt(a64,b64) = %lld (decoded: %lld)\n", (long long)result64_cmpgt, __ternary_tt2b_t64(result64_cmpgt));
    printf("cmpneq(a64,b64) = %lld (decoded: %lld)\n", (long long)result64_cmpneq, __ternary_tt2b_t64(result64_cmpneq));
}

int main() {
    test_ternary_comparisons();
    return 0;
}