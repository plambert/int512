#include "int512.h"
#include "ctest.h"
#include <string.h>

/* ============================================================================
 * Unsigned Arithmetic Tests
 * ============================================================================ */

CTEST(uint512, add_simple) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_add(&a, &b, &result));
    ASSERT_EQUAL(150ULL, result.words[0]);
}

CTEST(uint512, add_with_carry) {
    uint512_t a = {{UINT64_MAX, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{1, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_add(&a, &b, &result));
    ASSERT_EQUAL(0ULL, result.words[0]);
    ASSERT_EQUAL(1ULL, result.words[1]);
}

CTEST(uint512, add_overflow) {
    uint512_t result;
    ASSERT_EQUAL(INT512_ERR_OVERFLOW, uint512_add(&UINT512_MAX, &UINT512_ONE, &result));
}

CTEST(uint512, sub_simple) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_sub(&a, &b, &result));
    ASSERT_EQUAL(50ULL, result.words[0]);
}

CTEST(uint512, sub_with_borrow) {
    uint512_t a = {{0, 1, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{1, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_sub(&a, &b, &result));
    ASSERT_EQUAL(UINT64_MAX, result.words[0]);
    ASSERT_EQUAL(0ULL, result.words[1]);
}

CTEST(uint512, sub_underflow) {
    uint512_t a = {{50, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_ERR_UNDERFLOW, uint512_sub(&a, &b, &result));
}

CTEST(uint512, mul_simple) {
    uint512_t a = {{10, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{20, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_mul(&a, &b, &result));
    ASSERT_EQUAL(200ULL, result.words[0]);
}

CTEST(uint512, mul_large) {
    uint512_t a = {{UINT64_MAX, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{2, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_mul(&a, &b, &result));
    ASSERT_EQUAL(UINT64_MAX - 1, result.words[0]);
    ASSERT_EQUAL(1ULL, result.words[1]);
}

CTEST(uint512, mul_overflow) {
    uint512_t a = {{0, 0, 0, 0, 0, 0, 0, UINT64_MAX}};
    uint512_t b = {{2, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_ERR_OVERFLOW, uint512_mul(&a, &b, &result));
}

CTEST(uint512, div_simple) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{10, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t quotient, remainder;

    ASSERT_EQUAL(INT512_OK, uint512_div(&a, &b, &quotient, &remainder));
    ASSERT_EQUAL(10ULL, quotient.words[0]);
    ASSERT_EQUAL(0ULL, remainder.words[0]);
}

CTEST(uint512, div_with_remainder) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{30, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t quotient, remainder;

    ASSERT_EQUAL(INT512_OK, uint512_div(&a, &b, &quotient, &remainder));
    ASSERT_EQUAL(3ULL, quotient.words[0]);
    ASSERT_EQUAL(10ULL, remainder.words[0]);
}

CTEST(uint512, div_by_zero) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t quotient, remainder;

    ASSERT_EQUAL(INT512_ERR_DIVIDE_BY_ZERO, uint512_div(&a, &UINT512_ZERO, &quotient, &remainder));
}

/* ============================================================================
 * Signed Arithmetic Tests
 * ============================================================================ */

CTEST(int512, add_positive) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_add(&a, &b, &result));
    ASSERT_EQUAL(150ULL, result.words[0]);
}

CTEST(int512, add_negative) {
    int512_t a = {{(uint64_t)-100, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                   UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    int512_t b = {{(uint64_t)-50, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                   UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_add(&a, &b, &result));
    ASSERT_TRUE(int512_is_negative(&result));
}

CTEST(int512, add_overflow_positive) {
    int512_t result;
    ASSERT_EQUAL(INT512_ERR_OVERFLOW, int512_add(&INT512_MAX, &INT512_ONE, &result));
}

CTEST(int512, sub_positive) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_sub(&a, &b, &result));
    ASSERT_EQUAL(50ULL, result.words[0]);
}

CTEST(int512, mul_positive) {
    int512_t a = {{10, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b = {{20, 0, 0, 0, 0, 0, 0, 0}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_mul(&a, &b, &result));
    ASSERT_EQUAL(200ULL, result.words[0]);
}

CTEST(int512, mul_negative) {
    int512_t a = {{10, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b;
    /* Create -20 in two's complement */
    b.words[0] = (uint64_t)-20;
    for (int i = 1; i < 8; i++) b.words[i] = UINT64_MAX;

    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_mul(&a, &b, &result));
    ASSERT_TRUE(int512_is_negative(&result));
}

CTEST(int512, div_positive) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b = {{10, 0, 0, 0, 0, 0, 0, 0}};
    int512_t quotient, remainder;

    ASSERT_EQUAL(INT512_OK, int512_div(&a, &b, &quotient, &remainder));
    ASSERT_EQUAL(10ULL, quotient.words[0]);
    ASSERT_EQUAL(0ULL, remainder.words[0]);
}

CTEST(int512, div_by_zero) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t quotient, remainder;

    ASSERT_EQUAL(INT512_ERR_DIVIDE_BY_ZERO, int512_div(&a, &INT512_ZERO, &quotient, &remainder));
}

/* ============================================================================
 * Mixed-size Operations Tests
 * ============================================================================ */

CTEST(uint512, add_u8) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_add_u8(&a, 50, &result));
    ASSERT_EQUAL(150ULL, result.words[0]);
}

CTEST(uint512, add_u64) {
    uint512_t a = {{UINT64_MAX - 10, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_add_u64(&a, 20, &result));
    ASSERT_EQUAL(9ULL, result.words[0]);
    ASSERT_EQUAL(1ULL, result.words[1]);
}

CTEST(uint512, mul_u32) {
    uint512_t a = {{1000, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_mul_u32(&a, 1000, &result));
    ASSERT_EQUAL(1000000ULL, result.words[0]);
}

CTEST(uint512, div_u16) {
    uint512_t a = {{1000, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t quotient;
    uint16_t remainder;

    ASSERT_EQUAL(INT512_OK, uint512_div_u16(&a, 30, &quotient, &remainder));
    ASSERT_EQUAL(33ULL, quotient.words[0]);
    ASSERT_EQUAL(10, remainder);
}

CTEST(int512, add_i32) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_add_i32(&a, -50, &result));
    ASSERT_EQUAL(50ULL, result.words[0]);
}

CTEST(int512, mul_i64) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_mul_i64(&a, -10, &result));
    ASSERT_TRUE(int512_is_negative(&result));
}

/* ============================================================================
 * String Conversion Tests
 * ============================================================================ */

CTEST(uint512, to_string_decimal) {
    uint512_t value = {{12345, 0, 0, 0, 0, 0, 0, 0}};
    char buffer[200];

    ASSERT_EQUAL(INT512_OK, uint512_to_string(&value, buffer, sizeof(buffer), 10));
    ASSERT_STR("12345", buffer);
}

CTEST(uint512, to_string_hex) {
    uint512_t value = {{255, 0, 0, 0, 0, 0, 0, 0}};
    char buffer[200];

    ASSERT_EQUAL(INT512_OK, uint512_to_string(&value, buffer, sizeof(buffer), 16));
    ASSERT_STR("ff", buffer);
}

CTEST(uint512, to_string_binary) {
    uint512_t value = {{5, 0, 0, 0, 0, 0, 0, 0}};
    char buffer[200];

    ASSERT_EQUAL(INT512_OK, uint512_to_string(&value, buffer, sizeof(buffer), 2));
    ASSERT_STR("101", buffer);
}

CTEST(uint512, from_string_decimal) {
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_from_string("12345", &result, 10));
    ASSERT_EQUAL(12345ULL, result.words[0]);
}

CTEST(uint512, from_string_hex) {
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_from_string("FF", &result, 16));
    ASSERT_EQUAL(255ULL, result.words[0]);
}

CTEST(uint512, from_string_binary) {
    uint512_t result;

    ASSERT_EQUAL(INT512_OK, uint512_from_string("101", &result, 2));
    ASSERT_EQUAL(5ULL, result.words[0]);
}

CTEST(uint512, from_string_invalid) {
    uint512_t result;

    ASSERT_EQUAL(INT512_ERR_INVALID_STRING, uint512_from_string("12G", &result, 10));
}

CTEST(uint512, from_string_invalid_base) {
    uint512_t result;

    ASSERT_EQUAL(INT512_ERR_INVALID_BASE, uint512_from_string("123", &result, 1));
    ASSERT_EQUAL(INT512_ERR_INVALID_BASE, uint512_from_string("123", &result, 37));
}

CTEST(int512, to_string_positive) {
    int512_t value = {{12345, 0, 0, 0, 0, 0, 0, 0}};
    char buffer[200];

    ASSERT_EQUAL(INT512_OK, int512_to_string(&value, buffer, sizeof(buffer), 10));
    ASSERT_STR("12345", buffer);
}

CTEST(int512, to_string_negative) {
    int512_t value;
    /* Create -12345 in two's complement */
    value.words[0] = (uint64_t)-12345;
    for (int i = 1; i < 8; i++) value.words[i] = UINT64_MAX;

    char buffer[200];

    ASSERT_EQUAL(INT512_OK, int512_to_string(&value, buffer, sizeof(buffer), 10));
    ASSERT_STR("-12345", buffer);
}

CTEST(int512, from_string_positive) {
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_from_string("12345", &result, 10));
    ASSERT_EQUAL(12345ULL, result.words[0]);
}

CTEST(int512, from_string_negative) {
    int512_t result;

    ASSERT_EQUAL(INT512_OK, int512_from_string("-12345", &result, 10));
    ASSERT_TRUE(int512_is_negative(&result));
}

/* ============================================================================
 * Utility Function Tests
 * ============================================================================ */

CTEST(uint512, is_zero_true) {
    ASSERT_TRUE(uint512_is_zero(&UINT512_ZERO));
}

CTEST(uint512, is_zero_false) {
    ASSERT_FALSE(uint512_is_zero(&UINT512_ONE));
}

CTEST(int512, is_zero_true) {
    ASSERT_TRUE(int512_is_zero(&INT512_ZERO));
}

CTEST(int512, is_negative_false) {
    ASSERT_FALSE(int512_is_negative(&INT512_ONE));
}

CTEST(int512, is_negative_true) {
    int512_t neg;
    neg.words[0] = (uint64_t)-1;
    for (int i = 1; i < 8; i++) neg.words[i] = UINT64_MAX;

    ASSERT_TRUE(int512_is_negative(&neg));
}

CTEST(uint512, compare_equal) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{100, 0, 0, 0, 0, 0, 0, 0}};

    ASSERT_EQUAL(0, uint512_compare(&a, &b));
}

CTEST(uint512, compare_less) {
    uint512_t a = {{50, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{100, 0, 0, 0, 0, 0, 0, 0}};

    ASSERT_EQUAL(-1, uint512_compare(&a, &b));
}

CTEST(uint512, compare_greater) {
    uint512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};

    ASSERT_EQUAL(1, uint512_compare(&a, &b));
}

CTEST(int512, compare_positive) {
    int512_t a = {{100, 0, 0, 0, 0, 0, 0, 0}};
    int512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};

    ASSERT_EQUAL(1, int512_compare(&a, &b));
}

CTEST(int512, compare_negative_positive) {
    int512_t a;
    a.words[0] = (uint64_t)-50;
    for (int i = 1; i < 8; i++) a.words[i] = UINT64_MAX;

    int512_t b = {{50, 0, 0, 0, 0, 0, 0, 0}};

    ASSERT_EQUAL(-1, int512_compare(&a, &b));
}

/* ============================================================================
 * Constants Tests
 * ============================================================================ */

CTEST(constants, uint512_zero) {
    ASSERT_TRUE(uint512_is_zero(&UINT512_ZERO));
}

CTEST(constants, uint512_one) {
    ASSERT_EQUAL(1ULL, UINT512_ONE.words[0]);
    for (int i = 1; i < 8; i++) {
        ASSERT_EQUAL(0ULL, UINT512_ONE.words[i]);
    }
}

CTEST(constants, uint512_max) {
    for (int i = 0; i < 8; i++) {
        ASSERT_EQUAL(UINT64_MAX, UINT512_MAX.words[i]);
    }
}

CTEST(constants, int512_zero) {
    ASSERT_TRUE(int512_is_zero(&INT512_ZERO));
}

CTEST(constants, int512_one) {
    ASSERT_EQUAL(1ULL, INT512_ONE.words[0]);
    for (int i = 1; i < 8; i++) {
        ASSERT_EQUAL(0ULL, INT512_ONE.words[i]);
    }
}

CTEST(constants, int512_max) {
    for (int i = 0; i < 7; i++) {
        ASSERT_EQUAL(UINT64_MAX, INT512_MAX.words[i]);
    }
    ASSERT_EQUAL((uint64_t)INT64_MAX, INT512_MAX.words[7]);
}

CTEST(constants, int512_min) {
    for (int i = 0; i < 7; i++) {
        ASSERT_EQUAL(0ULL, INT512_MIN.words[i]);
    }
    ASSERT_EQUAL((uint64_t)INT64_MIN, INT512_MIN.words[7]);
}

/* ============================================================================
 * Roundtrip String Conversion Tests
 * ============================================================================ */

CTEST(roundtrip, uint512_large_decimal) {
    char buffer[200];
    uint512_t original = {{123456789ULL, 987654321ULL, 0, 0, 0, 0, 0, 0}};
    uint512_t parsed;

    ASSERT_EQUAL(INT512_OK, uint512_to_string(&original, buffer, sizeof(buffer), 10));
    ASSERT_EQUAL(INT512_OK, uint512_from_string(buffer, &parsed, 10));

    ASSERT_EQUAL(0, uint512_compare(&original, &parsed));
}

CTEST(roundtrip, int512_negative) {
    char buffer[200];
    int512_t original;
    original.words[0] = (uint64_t)-999999;
    for (int i = 1; i < 8; i++) original.words[i] = UINT64_MAX;

    int512_t parsed;

    ASSERT_EQUAL(INT512_OK, int512_to_string(&original, buffer, sizeof(buffer), 10));
    ASSERT_EQUAL(INT512_OK, int512_from_string(buffer, &parsed, 10));

    ASSERT_EQUAL(0, int512_compare(&original, &parsed));
}

CTEST(roundtrip, uint512_base36) {
    char buffer[200];
    uint512_t original = {{1234567890ULL, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t parsed;

    ASSERT_EQUAL(INT512_OK, uint512_to_string(&original, buffer, sizeof(buffer), 36));
    ASSERT_EQUAL(INT512_OK, uint512_from_string(buffer, &parsed, 36));

    ASSERT_EQUAL(0, uint512_compare(&original, &parsed));
}
