#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "include/ternary.h"
#include "include/ternary_runtime.h"

// Test for ternary vector operations

void test_ternary_vector_operations() {
    printf("Testing ternary vector operations\n");

    // Test tv32_t operations (vector of 2 x t32_t)
    t32_t val1 = __ternary_tb2t_t32(5);
    t32_t val2 = __ternary_tb2t_t32(3);
    t32_t val3 = __ternary_tb2t_t32(10);
    t32_t val4 = __ternary_tb2t_t32(7);

    // Pack into vectors (manual for now - in real usage this would be done by compiler)
    tv32_t vec_a = ((tv32_t)(uint64_t)val1) | (((tv32_t)(uint64_t)val2) << 64);
    tv32_t vec_b = ((tv32_t)(uint64_t)val3) | (((tv32_t)(uint64_t)val4) << 64);

    printf("Vector A: [%" PRId64 ", %" PRId64 "]\n",
           __ternary_tt2b_t32((t32_t)(uint64_t)vec_a),
           __ternary_tt2b_t32((t32_t)(uint64_t)(vec_a >> 64)));
    printf("Vector B: [%" PRId64 ", %" PRId64 "]\n",
           __ternary_tt2b_t32((t32_t)(uint64_t)vec_b),
           __ternary_tt2b_t32((t32_t)(uint64_t)(vec_b >> 64)));

    // Test vector addition
    tv32_t vec_add = __ternary_add_tv32(vec_a, vec_b);
    int64_t add0 = __ternary_tt2b_t32((t32_t)(uint64_t)vec_add);
    int64_t add1 = __ternary_tt2b_t32((t32_t)(uint64_t)(vec_add >> 64));
    printf("Vector Add: [%" PRId64 ", %" PRId64 "] (expected [15, 10])\n", add0, add1);

    // Test vector AND
    tv32_t vec_and = __ternary_and_tv32(vec_a, vec_b);
    int64_t and0 = __ternary_tt2b_t32((t32_t)(uint64_t)vec_and);
    int64_t and1 = __ternary_tt2b_t32((t32_t)(uint64_t)(vec_and >> 64));
    printf("Vector AND: [%" PRId64 ", %" PRId64 "]\n", and0, and1);

    printf("Vector operations test completed\n");
}

int main() {
    test_ternary_vector_operations();
    return 0;
}