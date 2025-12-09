#include "int512.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* ============================================================================
 * Constants
 * ============================================================================ */

const uint512_t UINT512_ZERO = {{0, 0, 0, 0, 0, 0, 0, 0}};
const uint512_t UINT512_ONE = {{1, 0, 0, 0, 0, 0, 0, 0}};
const uint512_t UINT512_MAX = {{UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};

const int512_t INT512_ZERO = {{0, 0, 0, 0, 0, 0, 0, 0}};
const int512_t INT512_ONE = {{1, 0, 0, 0, 0, 0, 0, 0}};
const int512_t INT512_MAX = {{UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                              UINT64_MAX, UINT64_MAX, UINT64_MAX, INT64_MAX}};
const int512_t INT512_MIN = {{0, 0, 0, 0, 0, 0, 0, (uint64_t)INT64_MIN}};

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

bool uint512_is_zero(const uint512_t *a) {
    if (!a) return true;
    for (int i = 0; i < 8; i++) {
        if (a->words[i] != 0) return false;
    }
    return true;
}

bool int512_is_zero(const int512_t *a) {
    return uint512_is_zero((const uint512_t *)a);
}

bool int512_is_negative(const int512_t *a) {
    if (!a) return false;
    return (a->words[7] & ((uint64_t)1 << 63)) != 0;
}

int uint512_compare(const uint512_t *a, const uint512_t *b) {
    if (!a || !b) return 0;
    for (int i = 7; i >= 0; i--) {
        if (a->words[i] > b->words[i]) return 1;
        if (a->words[i] < b->words[i]) return -1;
    }
    return 0;
}

int int512_compare(const int512_t *a, const int512_t *b) {
    if (!a || !b) return 0;
    bool a_neg = int512_is_negative(a);
    bool b_neg = int512_is_negative(b);
    if (a_neg != b_neg) return a_neg ? -1 : 1;
    return uint512_compare((const uint512_t *)a, (const uint512_t *)b);
}

/* ============================================================================
 * Unsigned Arithmetic Operations
 * ============================================================================ */

int512_error_t uint512_add(const uint512_t *a, const uint512_t *b, uint512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    uint64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t sum = a->words[i] + carry;
        carry = (sum < carry) ? 1 : 0;
        sum += b->words[i];
        carry += (sum < b->words[i]) ? 1 : 0;
        result->words[i] = sum;
    }

    return carry ? INT512_ERR_OVERFLOW : INT512_OK;
}

int512_error_t uint512_sub(const uint512_t *a, const uint512_t *b, uint512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    uint64_t borrow = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t diff = a->words[i] - borrow;
        borrow = (diff > a->words[i]) ? 1 : 0;
        uint64_t temp = diff - b->words[i];
        borrow += (temp > diff) ? 1 : 0;
        result->words[i] = temp;
    }

    return borrow ? INT512_ERR_UNDERFLOW : INT512_OK;
}

int512_error_t uint512_mul(const uint512_t *a, const uint512_t *b, uint512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    uint512_t temp = UINT512_ZERO;
    bool overflow = false;

    for (int i = 0; i < 8; i++) {
        if (b->words[i] == 0) continue;

        uint64_t carry = 0;
        for (int j = 0; j < 8; j++) {
            if (i + j >= 8) {
                if (a->words[j] != 0) overflow = true;
                continue;
            }

            /* Multiply with 128-bit intermediate */
            __uint128_t prod = (__uint128_t)a->words[j] * b->words[i] + temp.words[i + j] + carry;
            temp.words[i + j] = (uint64_t)prod;
            carry = (uint64_t)(prod >> 64);
        }
        if (carry != 0) overflow = true;
    }

    *result = temp;
    return overflow ? INT512_ERR_OVERFLOW : INT512_OK;
}

int512_error_t uint512_div(const uint512_t *a, const uint512_t *b, uint512_t *quotient, uint512_t *remainder) {
    if (!a || !b || !quotient || !remainder) return INT512_ERR_NULL_POINTER;
    if (uint512_is_zero(b)) return INT512_ERR_DIVIDE_BY_ZERO;

    *quotient = UINT512_ZERO;
    *remainder = *a;

    if (uint512_compare(a, b) < 0) return INT512_OK;

    /* Find highest set bit in dividend */
    int dividend_bits = 512;
    for (int i = 7; i >= 0; i--) {
        if (a->words[i] != 0) {
            dividend_bits = i * 64 + 63;
            uint64_t word = a->words[i];
            while (!(word & ((uint64_t)1 << (dividend_bits % 64)))) {
                dividend_bits--;
            }
            break;
        }
    }

    /* Find highest set bit in divisor */
    int divisor_bits = 512;
    for (int i = 7; i >= 0; i--) {
        if (b->words[i] != 0) {
            divisor_bits = i * 64 + 63;
            uint64_t word = b->words[i];
            while (!(word & ((uint64_t)1 << (divisor_bits % 64)))) {
                divisor_bits--;
            }
            break;
        }
    }

    /* Long division */
    uint512_t current = UINT512_ZERO;
    for (int i = dividend_bits; i >= 0; i--) {
        /* Shift current left by 1 */
        uint64_t carry = 0;
        for (int j = 0; j < 8; j++) {
            uint64_t new_carry = (current.words[j] >> 63);
            current.words[j] = (current.words[j] << 1) | carry;
            carry = new_carry;
        }

        /* Set lowest bit to bit i of dividend */
        int word_idx = i / 64;
        int bit_idx = i % 64;
        if (a->words[word_idx] & ((uint64_t)1 << bit_idx)) {
            current.words[0] |= 1;
        }

        /* If current >= divisor, subtract and set quotient bit */
        if (uint512_compare(&current, b) >= 0) {
            uint512_sub(&current, b, &current);
            word_idx = i / 64;
            bit_idx = i % 64;
            quotient->words[word_idx] |= ((uint64_t)1 << bit_idx);
        }
    }

    *remainder = current;
    return INT512_OK;
}

/* ============================================================================
 * Signed Arithmetic Operations
 * ============================================================================ */

static void int512_negate(int512_t *a) {
    /* Two's complement: flip bits and add 1 */
    for (int i = 0; i < 8; i++) {
        a->words[i] = ~a->words[i];
    }
    uint64_t carry = 1;
    for (int i = 0; i < 8; i++) {
        uint64_t sum = a->words[i] + carry;
        carry = (sum < a->words[i]) ? 1 : 0;
        a->words[i] = sum;
    }
}

int512_error_t int512_add(const int512_t *a, const int512_t *b, int512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    bool a_neg = int512_is_negative(a);
    bool b_neg = int512_is_negative(b);

    uint512_add((const uint512_t *)a, (const uint512_t *)b, (uint512_t *)result);

    bool result_neg = int512_is_negative(result);

    /* Check for overflow: same sign inputs produce different sign output */
    if (a_neg == b_neg && a_neg != result_neg) {
        return a_neg ? INT512_ERR_UNDERFLOW : INT512_ERR_OVERFLOW;
    }

    return INT512_OK;
}

int512_error_t int512_sub(const int512_t *a, const int512_t *b, int512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    bool a_neg = int512_is_negative(a);
    bool b_neg = int512_is_negative(b);

    uint512_sub((const uint512_t *)a, (const uint512_t *)b, (uint512_t *)result);

    bool result_neg = int512_is_negative(result);

    /* Check for overflow: a - b where a and b have different signs */
    if (a_neg != b_neg && a_neg != result_neg) {
        return a_neg ? INT512_ERR_UNDERFLOW : INT512_ERR_OVERFLOW;
    }

    return INT512_OK;
}

int512_error_t int512_mul(const int512_t *a, const int512_t *b, int512_t *result) {
    if (!a || !b || !result) return INT512_ERR_NULL_POINTER;

    int512_t abs_a = *a;
    int512_t abs_b = *b;
    bool negate_result = false;

    if (int512_is_negative(a)) {
        int512_negate(&abs_a);
        negate_result = !negate_result;
    }
    if (int512_is_negative(b)) {
        int512_negate(&abs_b);
        negate_result = !negate_result;
    }

    int512_error_t err = uint512_mul((const uint512_t *)&abs_a, (const uint512_t *)&abs_b, (uint512_t *)result);

    if (negate_result) {
        int512_negate(result);
    }

    /* Check for overflow */
    if (err == INT512_ERR_OVERFLOW) return INT512_ERR_OVERFLOW;
    if (!negate_result && int512_is_negative(result)) return INT512_ERR_OVERFLOW;
    if (negate_result && !int512_is_negative(result) && !int512_is_zero(result)) return INT512_ERR_OVERFLOW;

    return INT512_OK;
}

int512_error_t int512_div(const int512_t *a, const int512_t *b, int512_t *quotient, int512_t *remainder) {
    if (!a || !b || !quotient || !remainder) return INT512_ERR_NULL_POINTER;
    if (int512_is_zero(b)) return INT512_ERR_DIVIDE_BY_ZERO;

    int512_t abs_a = *a;
    int512_t abs_b = *b;
    bool negate_quotient = false;
    bool negate_remainder = false;

    if (int512_is_negative(a)) {
        int512_negate(&abs_a);
        negate_quotient = !negate_quotient;
        negate_remainder = true;
    }
    if (int512_is_negative(b)) {
        int512_negate(&abs_b);
        negate_quotient = !negate_quotient;
    }

    int512_error_t err = uint512_div((const uint512_t *)&abs_a, (const uint512_t *)&abs_b,
                                     (uint512_t *)quotient, (uint512_t *)remainder);

    if (err != INT512_OK) return err;

    if (negate_quotient) {
        int512_negate(quotient);
    }
    if (negate_remainder) {
        int512_negate(remainder);
    }

    return INT512_OK;
}

/* ============================================================================
 * Mixed-size Unsigned Operations
 * ============================================================================ */

int512_error_t uint512_add_u8(const uint512_t *a, uint8_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_add(a, &temp, result);
}

int512_error_t uint512_add_u16(const uint512_t *a, uint16_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_add(a, &temp, result);
}

int512_error_t uint512_add_u32(const uint512_t *a, uint32_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_add(a, &temp, result);
}

int512_error_t uint512_add_u64(const uint512_t *a, uint64_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_add(a, &temp, result);
}

int512_error_t uint512_add_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result) {
    uint512_t temp = {{b[0], b[1], 0, 0, 0, 0, 0, 0}};
    return uint512_add(a, &temp, result);
}

int512_error_t uint512_sub_u8(const uint512_t *a, uint8_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_sub(a, &temp, result);
}

int512_error_t uint512_sub_u16(const uint512_t *a, uint16_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_sub(a, &temp, result);
}

int512_error_t uint512_sub_u32(const uint512_t *a, uint32_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_sub(a, &temp, result);
}

int512_error_t uint512_sub_u64(const uint512_t *a, uint64_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_sub(a, &temp, result);
}

int512_error_t uint512_sub_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result) {
    uint512_t temp = {{b[0], b[1], 0, 0, 0, 0, 0, 0}};
    return uint512_sub(a, &temp, result);
}

int512_error_t uint512_mul_u8(const uint512_t *a, uint8_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_mul(a, &temp, result);
}

int512_error_t uint512_mul_u16(const uint512_t *a, uint16_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_mul(a, &temp, result);
}

int512_error_t uint512_mul_u32(const uint512_t *a, uint32_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_mul(a, &temp, result);
}

int512_error_t uint512_mul_u64(const uint512_t *a, uint64_t b, uint512_t *result) {
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    return uint512_mul(a, &temp, result);
}

int512_error_t uint512_mul_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result) {
    uint512_t temp = {{b[0], b[1], 0, 0, 0, 0, 0, 0}};
    return uint512_mul(a, &temp, result);
}

int512_error_t uint512_div_u8(const uint512_t *a, uint8_t b, uint512_t *quotient, uint8_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t rem512;
    int512_error_t err = uint512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (uint8_t)rem512.words[0];
    }
    return err;
}

int512_error_t uint512_div_u16(const uint512_t *a, uint16_t b, uint512_t *quotient, uint16_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t rem512;
    int512_error_t err = uint512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (uint16_t)rem512.words[0];
    }
    return err;
}

int512_error_t uint512_div_u32(const uint512_t *a, uint32_t b, uint512_t *quotient, uint32_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t rem512;
    int512_error_t err = uint512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (uint32_t)rem512.words[0];
    }
    return err;
}

int512_error_t uint512_div_u64(const uint512_t *a, uint64_t b, uint512_t *quotient, uint64_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    uint512_t temp = {{b, 0, 0, 0, 0, 0, 0, 0}};
    uint512_t rem512;
    int512_error_t err = uint512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = rem512.words[0];
    }
    return err;
}

int512_error_t uint512_div_u128(const uint512_t *a, const uint64_t b[2], uint512_t *quotient, uint64_t remainder[2]) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    uint512_t temp = {{b[0], b[1], 0, 0, 0, 0, 0, 0}};
    uint512_t rem512;
    int512_error_t err = uint512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        remainder[0] = rem512.words[0];
        remainder[1] = rem512.words[1];
    }
    return err;
}

/* ============================================================================
 * Mixed-size Signed Operations
 * ============================================================================ */

int512_error_t int512_add_i8(const int512_t *a, int8_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_add(a, &temp, result);
}

int512_error_t int512_add_i16(const int512_t *a, int16_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_add(a, &temp, result);
}

int512_error_t int512_add_i32(const int512_t *a, int32_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_add(a, &temp, result);
}

int512_error_t int512_add_i64(const int512_t *a, int64_t b, int512_t *result) {
    int512_t temp;
    if (b < 0) {
        temp = (int512_t){{(uint64_t)b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_add(a, &temp, result);
}

int512_error_t int512_add_i128(const int512_t *a, const int64_t b[2], int512_t *result) {
    int512_t temp;
    if (b[1] < 0) {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], 0, 0, 0, 0, 0, 0}};
    }
    return int512_add(a, &temp, result);
}

int512_error_t int512_sub_i8(const int512_t *a, int8_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_sub(a, &temp, result);
}

int512_error_t int512_sub_i16(const int512_t *a, int16_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_sub(a, &temp, result);
}

int512_error_t int512_sub_i32(const int512_t *a, int32_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_sub(a, &temp, result);
}

int512_error_t int512_sub_i64(const int512_t *a, int64_t b, int512_t *result) {
    int512_t temp;
    if (b < 0) {
        temp = (int512_t){{(uint64_t)b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_sub(a, &temp, result);
}

int512_error_t int512_sub_i128(const int512_t *a, const int64_t b[2], int512_t *result) {
    int512_t temp;
    if (b[1] < 0) {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], 0, 0, 0, 0, 0, 0}};
    }
    return int512_sub(a, &temp, result);
}

int512_error_t int512_mul_i8(const int512_t *a, int8_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_mul(a, &temp, result);
}

int512_error_t int512_mul_i16(const int512_t *a, int16_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_mul(a, &temp, result);
}

int512_error_t int512_mul_i32(const int512_t *a, int32_t b, int512_t *result) {
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_mul(a, &temp, result);
}

int512_error_t int512_mul_i64(const int512_t *a, int64_t b, int512_t *result) {
    int512_t temp;
    if (b < 0) {
        temp = (int512_t){{(uint64_t)b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b, 0, 0, 0, 0, 0, 0, 0}};
    }
    return int512_mul(a, &temp, result);
}

int512_error_t int512_mul_i128(const int512_t *a, const int64_t b[2], int512_t *result) {
    int512_t temp;
    if (b[1] < 0) {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], 0, 0, 0, 0, 0, 0}};
    }
    return int512_mul(a, &temp, result);
}

int512_error_t int512_div_i8(const int512_t *a, int8_t b, int512_t *quotient, int8_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    int512_t rem512;
    int512_error_t err = int512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (int8_t)rem512.words[0];
    }
    return err;
}

int512_error_t int512_div_i16(const int512_t *a, int16_t b, int512_t *quotient, int16_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    int512_t rem512;
    int512_error_t err = int512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (int16_t)rem512.words[0];
    }
    return err;
}

int512_error_t int512_div_i32(const int512_t *a, int32_t b, int512_t *quotient, int32_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    int512_t temp;
    int64_t ext_b = (int64_t)b;
    if (ext_b < 0) {
        temp = (int512_t){{(uint64_t)ext_b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)ext_b, 0, 0, 0, 0, 0, 0, 0}};
    }
    int512_t rem512;
    int512_error_t err = int512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (int32_t)rem512.words[0];
    }
    return err;
}

int512_error_t int512_div_i64(const int512_t *a, int64_t b, int512_t *quotient, int64_t *remainder) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    int512_t temp;
    if (b < 0) {
        temp = (int512_t){{(uint64_t)b, UINT64_MAX, UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b, 0, 0, 0, 0, 0, 0, 0}};
    }
    int512_t rem512;
    int512_error_t err = int512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        *remainder = (int64_t)rem512.words[0];
    }
    return err;
}

int512_error_t int512_div_i128(const int512_t *a, const int64_t b[2], int512_t *quotient, int64_t remainder[2]) {
    if (!remainder) return INT512_ERR_NULL_POINTER;
    int512_t temp;
    if (b[1] < 0) {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], UINT64_MAX, UINT64_MAX,
                           UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX}};
    } else {
        temp = (int512_t){{(uint64_t)b[0], (uint64_t)b[1], 0, 0, 0, 0, 0, 0}};
    }
    int512_t rem512;
    int512_error_t err = int512_div(a, &temp, quotient, &rem512);
    if (err == INT512_OK) {
        remainder[0] = (int64_t)rem512.words[0];
        remainder[1] = (int64_t)rem512.words[1];
    }
    return err;
}

/* ============================================================================
 * String Conversion Functions
 * ============================================================================ */

int512_error_t uint512_to_string(const uint512_t *value, char *buffer, size_t buffer_size, int base) {
    if (!value || !buffer || buffer_size == 0) return INT512_ERR_NULL_POINTER;
    if (base < 2 || base > 36) return INT512_ERR_INVALID_BASE;

    if (uint512_is_zero(value)) {
        if (buffer_size < 2) return INT512_ERR_INVALID_STRING;
        buffer[0] = '0';
        buffer[1] = '\0';
        return INT512_OK;
    }

    char temp[600];
    int pos = 0;
    uint512_t num = *value;

    while (!uint512_is_zero(&num)) {
        uint512_t base_val = {{(uint64_t)base, 0, 0, 0, 0, 0, 0, 0}};
        uint512_t quotient, remainder;
        uint512_div(&num, &base_val, &quotient, &remainder);

        int digit = (int)remainder.words[0];
        temp[pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num = quotient;
    }

    if ((size_t)pos + 1 > buffer_size) return INT512_ERR_INVALID_STRING;

    for (int i = 0; i < pos; i++) {
        buffer[i] = temp[pos - 1 - i];
    }
    buffer[pos] = '\0';

    return INT512_OK;
}

int512_error_t uint512_from_string(const char *str, uint512_t *result, int base) {
    if (!str || !result) return INT512_ERR_NULL_POINTER;
    if (base < 2 || base > 36) return INT512_ERR_INVALID_BASE;

    *result = UINT512_ZERO;

    while (*str == ' ' || *str == '\t') str++;

    if (*str == '+') str++;

    if (*str == '\0') return INT512_ERR_INVALID_STRING;

    uint512_t base_val = {{(uint64_t)base, 0, 0, 0, 0, 0, 0, 0}};

    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            return INT512_ERR_INVALID_STRING;
        }

        if (digit >= base) return INT512_ERR_INVALID_STRING;

        uint512_t temp;
        int512_error_t err = uint512_mul(result, &base_val, &temp);
        if (err != INT512_OK) return err;

        uint512_t digit_val = {{(uint64_t)digit, 0, 0, 0, 0, 0, 0, 0}};
        err = uint512_add(&temp, &digit_val, result);
        if (err != INT512_OK) return err;

        str++;
    }

    return INT512_OK;
}

int512_error_t int512_to_string(const int512_t *value, char *buffer, size_t buffer_size, int base) {
    if (!value || !buffer || buffer_size == 0) return INT512_ERR_NULL_POINTER;
    if (base < 2 || base > 36) return INT512_ERR_INVALID_BASE;

    if (int512_is_zero(value)) {
        if (buffer_size < 2) return INT512_ERR_INVALID_STRING;
        buffer[0] = '0';
        buffer[1] = '\0';
        return INT512_OK;
    }

    int offset = 0;
    int512_t num = *value;

    if (int512_is_negative(value)) {
        if (buffer_size < 2) return INT512_ERR_INVALID_STRING;
        buffer[0] = '-';
        offset = 1;
        int512_negate(&num);
    }

    return uint512_to_string((const uint512_t *)&num, buffer + offset, buffer_size - offset, base);
}

int512_error_t int512_from_string(const char *str, int512_t *result, int base) {
    if (!str || !result) return INT512_ERR_NULL_POINTER;
    if (base < 2 || base > 36) return INT512_ERR_INVALID_BASE;

    while (*str == ' ' || *str == '\t') str++;

    bool negative = false;
    if (*str == '-') {
        negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    int512_error_t err = uint512_from_string(str, (uint512_t *)result, base);
    if (err != INT512_OK) return err;

    if (negative) {
        int512_negate(result);
    }

    return INT512_OK;
}
