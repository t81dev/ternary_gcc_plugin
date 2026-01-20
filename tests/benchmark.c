#include <stdio.h>
#include <time.h>
#include "ternary_helpers.h"

#define ITERATIONS 100000

int main() {
    clock_t start, end;
    double cpu_time_used;

    // Benchmark ternary add
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        __ternary_add(i % 100, (i + 1) % 100);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Ternary add: %f seconds for %d iterations\n", cpu_time_used, ITERATIONS);

    // Benchmark binary add for comparison
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        (i % 100) + ((i + 1) % 100);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Binary add: %f seconds for %d iterations\n", cpu_time_used, ITERATIONS);

    // Benchmark ternary multiply
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        __ternary_mul(i % 10, (i + 1) % 10);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Ternary mul: %f seconds for %d iterations\n", cpu_time_used, ITERATIONS);

    // Benchmark binary multiply
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        (i % 10) * ((i + 1) % 10);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Binary mul: %f seconds for %d iterations\n", cpu_time_used, ITERATIONS);

    // Dot product example (small)
    int vec_a[10] = {1, -1, 0, 1, -1, 0, 1, -1, 0, 1};
    int vec_b[10] = {-1, 1, 0, -1, 1, 0, -1, 1, 0, -1};
    start = clock();
    int dot = 0;
    for (int i = 0; i < 10; i++) {
        dot = __ternary_add(dot, __ternary_mul(vec_a[i], vec_b[i]));
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Ternary dot product (10 elements): %d in %f seconds\n", dot, cpu_time_used);

    return 0;
}