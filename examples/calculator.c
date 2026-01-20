#include <stdio.h>
#include <stdlib.h>
#include "ternary.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <op> <a> <b>\nOps: add, mul, sub, div\n", argv[0]);
        return 1;
    }

    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
    t12_t ta = int_to_ternary(a);
    t12_t tb = int_to_ternary(b);
    t12_t result;

    if (strcmp(argv[1], "add") == 0) {
        result = ta + tb;
    } else if (strcmp(argv[1], "mul") == 0) {
        result = ta * tb;
    } else if (strcmp(argv[1], "sub") == 0) {
        result = ta - tb;
    } else if (strcmp(argv[1], "div") == 0) {
        result = ta / tb;
    } else {
        printf("Unknown op\n");
        return 1;
    }

    printf("Ternary %s: %d %s %d = %d\n", argv[1], a, argv[1], b, ternary_to_int(result));
    return 0;
}