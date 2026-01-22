// Test file for Ternary GCC Plugin
// Compile with: gcc -fplugin=./ternary_plugin.so -fplugin-arg-ternary_plugin-lower -fplugin-arg-ternary_plugin-arith -fplugin-arg-ternary_plugin-logic -fplugin-arg-ternary_plugin-cmp -fplugin-arg-ternary_plugin-shift -fplugin-arg-ternary_plugin-conv -Iinclude -c test_ternary.c

#define TERNARY_COND_T int
#include "ternary_helpers.h"

extern int __builtin_ternary_and(int a, int b);
extern int __builtin_ternary_or(int a, int b);
extern int __builtin_ternary_xor(int a, int b);
extern int __builtin_ternary_shl(int a, int shift);
extern int __builtin_ternary_shr(int a, int shift);
extern int __builtin_ternary_rol(int a, int shift);
extern int __builtin_ternary_ror(int a, int shift);
extern t32_t __builtin_ternary_tb2t(int a);
extern int __builtin_ternary_tt2b(t32_t v);
extern float __builtin_ternary_t2f(t32_t v);
extern t32_t __builtin_ternary_f2t(float v);

int test_integral(int cond, int a, int b) {
    // This will be lowered to __ternary_select_i32(cond, a, b)
    return cond ? a : b;
}

unsigned int test_unsigned(int cond, unsigned int a, unsigned int b) {
    // This will be lowered to __ternary_select_u32(cond, a, b)
    return cond ? a : b;
}

float test_float(int cond, float a, float b) {
    // This will be lowered to __ternary_select_f32(cond, a, b)
    return cond ? a : b;
}

double test_double(int cond, double a, double b) {
    // This will be lowered to __ternary_select_f64(cond, a, b)
    return cond ? a : b;
}

int test_ternary_add(int a, int b) {
    // This builtin would be lowered to __ternary_add(a, b)
    return __ternary_add(a, b);
}

int test_ternary_mul(int a, int b) {
    return __ternary_mul(a, b);
}

int test_ternary_not(int a) {
    return __ternary_not(a);
}

int test_builtin_select(int cond, int a, int b) {
    return __ternary_select_i32(cond, a, b);
}

int test_ternary_sub(int a, int b) {
    return __ternary_sub(a, b);
}

int test_ternary_div(int a, int b) {
    return __ternary_div(a, b);
}

int test_ternary_mod(int a, int b) {
    return __ternary_mod(a, b);
}

int test_ternary_neg(int a) {
    return __ternary_neg(a);
}

int test_ternary_cmp(int a, int b) {
    return __ternary_cmp(a, b);
}

int test_ternary_and(int a, int b) {
    return __builtin_ternary_and(a, b);
}

int test_ternary_or(int a, int b) {
    return __builtin_ternary_or(a, b);
}

int test_ternary_xor(int a, int b) {
    return __builtin_ternary_xor(a, b);
}

int test_ternary_shl(int a, int shift) {
    return __builtin_ternary_shl(a, shift);
}

int test_ternary_shr(int a, int shift) {
    return __builtin_ternary_shr(a, shift);
}

int test_ternary_rol(int a, int shift) {
    return __builtin_ternary_rol(a, shift);
}

int test_ternary_ror(int a, int shift) {
    return __builtin_ternary_ror(a, shift);
}

t32_t test_ternary_tb2t(int a) {
    return __builtin_ternary_tb2t(a);
}

int test_ternary_tt2b(t32_t v) {
    return __builtin_ternary_tt2b(v);
}

float test_ternary_t2f(t32_t v) {
    return __builtin_ternary_t2f(v);
}

t32_t test_ternary_f2t(float v) {
    return __builtin_ternary_f2t(v);
}

int main() {
    int result1 = test_integral(1, 10, 20);  // Should be 10
    unsigned int result2 = test_unsigned(0, 100, 200);  // Should be 200
    float result3 = test_float(1, 1.5f, 2.5f);  // Should be 1.5
    double result4 = test_double(0, 3.14, 2.71);  // Should be 2.71
    int result5 = test_ternary_add(1, 2);  // Ternary add
    int result6 = test_ternary_mul(3, 4);  // Ternary mul
    int result7 = test_ternary_not(5);     // Ternary not
    int result8 = test_builtin_select(1, 7, 8);  // Builtin select
    int result9 = test_ternary_sub(10, 3);       // Ternary sub
    int result10 = test_ternary_div(15, 3);      // Ternary div
    int result11 = test_ternary_mod(17, 5);      // Ternary mod
    int result12 = test_ternary_neg(5);          // Ternary neg
    int result13 = test_ternary_cmp(10, 5);      // Ternary cmp (should be +1)
    int result14 = test_ternary_and(-1, 1);
    int result15 = test_ternary_or(-1, 1);
    int result16 = test_ternary_xor(-1, 1);
    int result17 = test_ternary_shl(2, 1);
    int result18 = test_ternary_shr(9, 1);
    int result19 = test_ternary_rol(3, 2);
    int result20 = test_ternary_ror(3, 2);
    t32_t tval = test_ternary_tb2t(5);
    int result21 = test_ternary_tt2b(tval);
    float result22 = test_ternary_t2f(tval);
    t32_t tfromf = test_ternary_f2t(3.2f);
    int result23 = test_ternary_tt2b(tfromf);

    return result1 + result2 + (int)result3 + (int)result4 + result5 + result6 + result7 + result8 +
           result9 + result10 + result11 + result12 + result13 + result14 + result15 + result16 +
           result17 + result18 + result19 + result20 + result21 + (int)result22 + result23;
}
