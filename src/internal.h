#ifndef __TAP_INTERNAL_H__
#define __TAP_INTERNAL_H__
#include <stdbool.h>
#include <stdint.h>
#include <tap.h>

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

#define TAP_DIRECTIVE_SKIP "SKIP"
#define TAP_DIRECTIVE_TODO "TODO"
#define TAP_PIPE_RX 0
#define TAP_PIPE_TX 1

struct test {
    char *description;
    test_t funct;
    size_t id;
};

struct tap_string;
typedef struct tap_string tap_string_t;

int tap_pipe_setup(int fds[2]);

tap_string_t *tap_string_ctor(const char *str);

int tap_string_concat(tap_string_t *tstr, const char *str);

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...);

char *tap_string_dtor(tap_string_t *tstr, bool free_str);

void tap_print_testpoint(bool success, struct test *test,
                         const char *directive);

void tap_print_internal_error(int err, struct test *test, const char *reason);

#endif /* __TAP_INTERNAL_H__ */
