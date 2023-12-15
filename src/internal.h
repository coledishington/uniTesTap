#ifndef __TAP_INTERNAL_H__
#define __TAP_INTERNAL_H__
#include <stdbool.h>

#define ARRAY_LEN(A) (sizeof(A) / sizeof(*A))

#define _STRING_JOIN(expr1, expr2) (expr1)##(expr2)
#define STRING_JOIN(expr1, expr2) _STRING_JOIN(expr1, expr2)

#define STATIC_ASSERT_ID STRING_JOIN(static_assert_, __LINE__)
#define STATIC_ASSERT_TEST(expr) ((expr) ? 1 : -1)

/* Static assert that uses both the bitmask and array tricks */
#define STATIC_ASSERT(expr)                              \
    typedef struct {                                     \
        int STATIC_ASSERT_ID : STATIC_ASSERT_TEST(expr); \
    } STATIC_ASSERT_ID[STATIC_ASSERT_TEST(expr)]

struct tap_string;
typedef struct tap_string tap_string_t;

tap_string_t *tap_string_ctor(const char *str);

int tap_string_concat(tap_string_t *tstr, const char *str);

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...);

char *tap_string_dtor(tap_string_t *tstr, bool free_str);

#endif /* __TAP_INTERNAL_H__ */
