#ifndef __TAP_UTIL_H__
#define __TAP_UTIL_H__

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

#endif /* __TAP_UTIL_H__ */
