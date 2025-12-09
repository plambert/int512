#ifndef INT512_H
#define INT512_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* 512-bit unsigned integer */
typedef struct {
    uint64_t words[8];  /* Little-endian: words[0] is LSW */
} uint512_t;

/* 512-bit signed integer (two's complement) */
typedef struct {
    uint64_t words[8];  /* Little-endian: words[0] is LSW */
} int512_t;

/* Error codes */
typedef enum {
    INT512_OK = 0,
    INT512_ERR_OVERFLOW,
    INT512_ERR_UNDERFLOW,
    INT512_ERR_DIVIDE_BY_ZERO,
    INT512_ERR_INVALID_STRING,
    INT512_ERR_INVALID_BASE,
    INT512_ERR_NULL_POINTER
} int512_error_t;

/* Constants */
extern const uint512_t UINT512_ZERO;
extern const uint512_t UINT512_ONE;
extern const uint512_t UINT512_MAX;

extern const int512_t INT512_ZERO;
extern const int512_t INT512_ONE;
extern const int512_t INT512_MAX;
extern const int512_t INT512_MIN;

/* Unsigned arithmetic operations */
int512_error_t uint512_add(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_sub(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_mul(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_div(const uint512_t *a, const uint512_t *b, uint512_t *quotient, uint512_t *remainder);

/* Signed arithmetic operations */
int512_error_t int512_add(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_sub(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_mul(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_div(const int512_t *a, const int512_t *b, int512_t *quotient, int512_t *remainder);

/* Mixed-size unsigned operations */
int512_error_t uint512_add_u8(const uint512_t *a, uint8_t b, uint512_t *result);
int512_error_t uint512_add_u16(const uint512_t *a, uint16_t b, uint512_t *result);
int512_error_t uint512_add_u32(const uint512_t *a, uint32_t b, uint512_t *result);
int512_error_t uint512_add_u64(const uint512_t *a, uint64_t b, uint512_t *result);
int512_error_t uint512_add_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result);

int512_error_t uint512_sub_u8(const uint512_t *a, uint8_t b, uint512_t *result);
int512_error_t uint512_sub_u16(const uint512_t *a, uint16_t b, uint512_t *result);
int512_error_t uint512_sub_u32(const uint512_t *a, uint32_t b, uint512_t *result);
int512_error_t uint512_sub_u64(const uint512_t *a, uint64_t b, uint512_t *result);
int512_error_t uint512_sub_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result);

int512_error_t uint512_mul_u8(const uint512_t *a, uint8_t b, uint512_t *result);
int512_error_t uint512_mul_u16(const uint512_t *a, uint16_t b, uint512_t *result);
int512_error_t uint512_mul_u32(const uint512_t *a, uint32_t b, uint512_t *result);
int512_error_t uint512_mul_u64(const uint512_t *a, uint64_t b, uint512_t *result);
int512_error_t uint512_mul_u128(const uint512_t *a, const uint64_t b[2], uint512_t *result);

int512_error_t uint512_div_u8(const uint512_t *a, uint8_t b, uint512_t *quotient, uint8_t *remainder);
int512_error_t uint512_div_u16(const uint512_t *a, uint16_t b, uint512_t *quotient, uint16_t *remainder);
int512_error_t uint512_div_u32(const uint512_t *a, uint32_t b, uint512_t *quotient, uint32_t *remainder);
int512_error_t uint512_div_u64(const uint512_t *a, uint64_t b, uint512_t *quotient, uint64_t *remainder);
int512_error_t uint512_div_u128(const uint512_t *a, const uint64_t b[2], uint512_t *quotient, uint64_t remainder[2]);

/* Mixed-size signed operations */
int512_error_t int512_add_i8(const int512_t *a, int8_t b, int512_t *result);
int512_error_t int512_add_i16(const int512_t *a, int16_t b, int512_t *result);
int512_error_t int512_add_i32(const int512_t *a, int32_t b, int512_t *result);
int512_error_t int512_add_i64(const int512_t *a, int64_t b, int512_t *result);
int512_error_t int512_add_i128(const int512_t *a, const int64_t b[2], int512_t *result);

int512_error_t int512_sub_i8(const int512_t *a, int8_t b, int512_t *result);
int512_error_t int512_sub_i16(const int512_t *a, int16_t b, int512_t *result);
int512_error_t int512_sub_i32(const int512_t *a, int32_t b, int512_t *result);
int512_error_t int512_sub_i64(const int512_t *a, int64_t b, int512_t *result);
int512_error_t int512_sub_i128(const int512_t *a, const int64_t b[2], int512_t *result);

int512_error_t int512_mul_i8(const int512_t *a, int8_t b, int512_t *result);
int512_error_t int512_mul_i16(const int512_t *a, int16_t b, int512_t *result);
int512_error_t int512_mul_i32(const int512_t *a, int32_t b, int512_t *result);
int512_error_t int512_mul_i64(const int512_t *a, int64_t b, int512_t *result);
int512_error_t int512_mul_i128(const int512_t *a, const int64_t b[2], int512_t *result);

int512_error_t int512_div_i8(const int512_t *a, int8_t b, int512_t *quotient, int8_t *remainder);
int512_error_t int512_div_i16(const int512_t *a, int16_t b, int512_t *quotient, int16_t *remainder);
int512_error_t int512_div_i32(const int512_t *a, int32_t b, int512_t *quotient, int32_t *remainder);
int512_error_t int512_div_i64(const int512_t *a, int64_t b, int512_t *quotient, int64_t *remainder);
int512_error_t int512_div_i128(const int512_t *a, const int64_t b[2], int512_t *quotient, int64_t remainder[2]);

/* String conversion */
int512_error_t uint512_to_string(const uint512_t *value, char *buffer, size_t buffer_size, int base);
int512_error_t uint512_from_string(const char *str, uint512_t *result, int base);

int512_error_t int512_to_string(const int512_t *value, char *buffer, size_t buffer_size, int base);
int512_error_t int512_from_string(const char *str, int512_t *result, int base);

/* Utility functions */
bool uint512_is_zero(const uint512_t *a);
bool int512_is_zero(const int512_t *a);
bool int512_is_negative(const int512_t *a);
int uint512_compare(const uint512_t *a, const uint512_t *b);
int int512_compare(const int512_t *a, const int512_t *b);

#endif /* INT512_H */
