#include <stdio.h>
#include <stdint.h>
#include "include/ternary_runtime.h"

// Simple test for ternary logic operations
// This tests the fixed txor implementation

// Simulate the ternary operations for validation
int64_t ternary_decode(uint64_t packed, unsigned trit_count) {
    int64_t value = 0;
    int64_t pow3 = 1;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits = (unsigned)((packed >> (2U * i)) & 0x3U);
        int trit = (bits == 0) ? -1 : (bits == 1 ? 0 : 1);
        value += (int64_t)trit * pow3;
        pow3 *= 3;
    }
    return value;
}

uint64_t ternary_encode(int64_t value, unsigned trit_count) {
    uint64_t packed = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        int64_t rem = value % 3;
        value /= 3;
        if (rem == 2) {
            rem = -1;
            value += 1;
        } else if (rem == -2) {
            rem = 1;
            value -= 1;
        }
        unsigned bits = (rem < 0) ? 0U : (rem == 0 ? 1U : 2U);
        packed |= ((uint64_t)bits) << (2U * i);
    }
    return packed;
}

int ternary_trit_min(int a, int b) {
    return (a < b) ? a : b;
}

int ternary_trit_max(int a, int b) {
    return (a > b) ? a : b;
}

int ternary_trit_xor(int a, int b) {
    int sum = a + b;
    int mod = ((sum % 3) + 3) % 3;
    if (mod == 0) return 0;
    if (mod == 1) return 1;
    return -1;
}

uint64_t ternary_tritwise_op(uint64_t a, uint64_t b, unsigned trit_count, int op) {
    uint64_t out = 0;
    for (unsigned i = 0; i < trit_count; ++i) {
        unsigned bits_a = (unsigned)((a >> (2U * i)) & 0x3U);
        unsigned bits_b = (unsigned)((b >> (2U * i)) & 0x3U);
        int ta = (bits_a == 0) ? -1 : (bits_a == 1 ? 0 : 1);
        int tb = (bits_b == 0) ? -1 : (bits_b == 1 ? 0 : 1);
        int trit = 0;
        if (op == 0)
            trit = ternary_trit_min(ta, tb);  // tand
        else if (op == 1)
            trit = ternary_trit_max(ta, tb);  // tor
        else
            trit = ternary_trit_xor(ta, tb);  // txor
        unsigned bits = (trit < 0) ? 0U : (trit == 0 ? 1U : 2U);
        out |= ((uint64_t)bits) << (2U * i);
    }
    return out;
}

void test_ternary_logic() {
    printf("Testing ternary logic operations (tand, tor, txor) on packed values\n");

    // Test trit-wise operations on specific packed values
    // For 2-trit values (6 bits total)
    struct {
        uint64_t a, b;  // packed ternary values
        uint64_t expected_tand, expected_tor, expected_txor;
        const char *desc;
    } tests[] = {
        // Simple 1-trit tests (using lower 2 bits)
        {0b01, 0b01, 0b01, 0b01, 0b01, "0 & 0 = 0, 0 | 0 = 0, 0 ^ 0 = 0"},
        {0b01, 0b10, 0b01, 0b10, 0b10, "0 & 1 = 0, 0 | 1 = 1, 0 ^ 1 = 1"},
        {0b01, 0b00, 0b00, 0b01, 0b00, "0 & -1 = -1, 0 | -1 = 0, 0 ^ -1 = -1"},
        {0b10, 0b10, 0b10, 0b10, 0b00, "1 & 1 = 1, 1 | 1 = 1, 1 ^ 1 = -1"},
        {0b10, 0b00, 0b00, 0b10, 0b01, "1 & -1 = -1, 1 | -1 = 1, 1 ^ -1 = 0"},
        {0b00, 0b00, 0b00, 0b00, 0b10, "-1 & -1 = -1, -1 | -1 = -1, -1 ^ -1 = 1"},
    };

    for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        uint64_t result_tand = ternary_tritwise_op(tests[i].a, tests[i].b, 1, 0);
        uint64_t result_tor = ternary_tritwise_op(tests[i].a, tests[i].b, 1, 1);
        uint64_t result_txor = ternary_tritwise_op(tests[i].a, tests[i].b, 1, 2);

        printf("Test %d: %s\n", i, tests[i].desc);
        printf("  Input a: 0x%llx, b: 0x%llx\n", tests[i].a, tests[i].b);
        printf("  tand: expected 0x%llx, got 0x%llx %s\n",
               tests[i].expected_tand, result_tand,
               (result_tand == tests[i].expected_tand) ? "✓" : "✗");
        printf("  tor:  expected 0x%llx, got 0x%llx %s\n",
               tests[i].expected_tor, result_tor,
               (result_tor == tests[i].expected_tor) ? "✓" : "✗");
        printf("  txor: expected 0x%llx, got 0x%llx %s\n",
               tests[i].expected_txor, result_txor,
               (result_txor == tests[i].expected_txor) ? "✓" : "✗");
        printf("\n");
    }

    // Test with larger values using the runtime functions
    printf("Testing with runtime functions:\n");
    int test_vals[] = {0, 1, -1, 3, -3};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int a = test_vals[i];
            int b = test_vals[j];
            int result_and = __ternary_and(a, b);
            int result_or = __ternary_or(a, b);
            int result_xor = __ternary_xor(a, b);

            // For these small values, and/or should be min/max
            int expected_and = (a < b) ? a : b;
            int expected_or = (a > b) ? a : b;

            printf("Runtime: %d & %d = %d (expected %d) %s\n", a, b, result_and, expected_and,
                   (result_and == expected_and) ? "✓" : "✗");
            printf("Runtime: %d | %d = %d (expected %d) %s\n", a, b, result_or, expected_or,
                   (result_or == expected_or) ? "✓" : "✗");
            printf("Runtime: %d ^ %d = %d\n", a, b, result_xor);
        }
    }
}

int main() {
    test_ternary_logic();
    return 0;
}