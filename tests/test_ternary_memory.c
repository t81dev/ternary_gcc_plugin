#include <stdio.h>
#include <stdint.h>
#include "include/ternary.h"
#include "include/ternary_runtime.h"

// Test for ternary memory operations (tld/tst)

void test_ternary_memory() {
    printf("Testing ternary memory operations (tld/tst)\n");

    // Test t32 memory operations
    t32_t original_t32 = __ternary_tb2t_t32(42);  // Convert 42 to ternary
    t32_t loaded_t32;
    t32_t *ptr_t32 = &original_t32;

    // Test load
    loaded_t32 = __ternary_load_t32(ptr_t32);
    int64_t decoded_loaded = __ternary_tt2b_t32(loaded_t32);
    printf("t32 load: stored %lld, loaded %lld %s\n",
           (long long)42, (long long)decoded_loaded,
           (decoded_loaded == 42) ? "✓" : "✗");

    // Test store
    t32_t new_value_t32 = __ternary_tb2t_t32(123);
    __ternary_store_t32(ptr_t32, new_value_t32);
    int64_t decoded_stored = __ternary_tt2b_t32(*ptr_t32);
    printf("t32 store: stored %lld, memory contains %lld %s\n",
           (long long)123, (long long)decoded_stored,
           (decoded_stored == 123) ? "✓" : "✗");

    // Test t64 memory operations
    t64_t original_t64 = (t64_t)99;
    t64_t loaded_t64;
    t64_t *ptr_t64 = &original_t64;

    // Test load
    loaded_t64 = __ternary_load_t64(ptr_t64);
    printf("t64 load: stored %lld, loaded %lld %s\n",
           (long long)99, (long long)loaded_t64,
           ((long long)loaded_t64 == 99) ? "✓" : "✗");

    // Test store
    t64_t new_value_t64 = (t64_t)456;
    __ternary_store_t64(ptr_t64, new_value_t64);
    printf("t64 store: stored %lld, memory contains %lld %s\n",
           (long long)456, (long long)*ptr_t64,
           ((long long)*ptr_t64 == 456) ? "✓" : "✗");

    printf("Memory operations test completed\n");
}

int main() {
    test_ternary_memory();
    return 0;
}