# int512 - 512-bit Integer Arithmetic Library

A C library providing 512-bit signed and unsigned integer arithmetic operations.

## Types

- `uint512_t` - 512-bit unsigned integer
- `int512_t` - 512-bit signed integer (two's complement)

## Error Handling

Operations return `int512_error_t` with these codes:
- `INT512_OK` - Success
- `INT512_ERR_OVERFLOW` - Arithmetic overflow
- `INT512_ERR_UNDERFLOW` - Arithmetic underflow
- `INT512_ERR_DIVIDE_BY_ZERO` - Division by zero
- `INT512_ERR_INVALID_STRING` - Invalid string format
- `INT512_ERR_INVALID_BASE` - Base not in range 2-36
- `INT512_ERR_NULL_POINTER` - Null pointer argument

## Constants

```c
UINT512_ZERO, UINT512_ONE, UINT512_MAX
INT512_ZERO, INT512_ONE, INT512_MAX, INT512_MIN
```

## Arithmetic Operations

### Basic Operations
```c
int512_error_t uint512_add(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_sub(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_mul(const uint512_t *a, const uint512_t *b, uint512_t *result);
int512_error_t uint512_div(const uint512_t *a, const uint512_t *b,
                           uint512_t *quotient, uint512_t *remainder);

int512_error_t int512_add(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_sub(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_mul(const int512_t *a, const int512_t *b, int512_t *result);
int512_error_t int512_div(const int512_t *a, const int512_t *b,
                          int512_t *quotient, int512_t *remainder);
```

### Mixed-size Operations

Operations with 8, 16, 32, 64, and 128-bit operands:
```c
uint512_add_u8, uint512_add_u16, uint512_add_u32, uint512_add_u64, uint512_add_u128
uint512_sub_u8, uint512_sub_u16, uint512_sub_u32, uint512_sub_u64, uint512_sub_u128
uint512_mul_u8, uint512_mul_u16, uint512_mul_u32, uint512_mul_u64, uint512_mul_u128
uint512_div_u8, uint512_div_u16, uint512_div_u32, uint512_div_u64, uint512_div_u128

int512_add_i8, int512_add_i16, int512_add_i32, int512_add_i64, int512_add_i128
int512_sub_i8, int512_sub_i16, int512_sub_i32, int512_sub_i64, int512_sub_i128
int512_mul_i8, int512_mul_i16, int512_mul_i32, int512_mul_i64, int512_mul_i128
int512_div_i8, int512_div_i16, int512_div_i32, int512_div_i64, int512_div_i128
```

## String Conversion

```c
int512_error_t uint512_to_string(const uint512_t *value, char *buffer,
                                 size_t buffer_size, int base);
int512_error_t uint512_from_string(const char *str, uint512_t *result, int base);

int512_error_t int512_to_string(const int512_t *value, char *buffer,
                                size_t buffer_size, int base);
int512_error_t int512_from_string(const char *str, int512_t *result, int base);
```

Base must be 2-36. Characters 0-9, a-z (case insensitive) are supported.

## Utility Functions

```c
bool uint512_is_zero(const uint512_t *a);
bool int512_is_zero(const int512_t *a);
bool int512_is_negative(const int512_t *a);
int uint512_compare(const uint512_t *a, const uint512_t *b);  // Returns -1, 0, or 1
int int512_compare(const int512_t *a, const int512_t *b);
```

## Building

```bash
make          # Build test executable (auto-downloads ctest.h if needed)
make test     # Build and run tests
make clean    # Clean build artifacts
```

The Makefile automatically downloads `ctest.h` from GitHub if it doesn't exist.

## Testing

The library includes 58 comprehensive tests using CTest covering:
- Basic arithmetic operations
- Overflow/underflow detection
- Mixed-size operations
- String conversion in multiple bases
- Edge cases and error conditions

All tests pass successfully.
