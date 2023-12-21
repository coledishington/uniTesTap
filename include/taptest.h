#ifndef __TAP_TEST_H__
#define __TAP_TEST_H__
#include <sys/types.h>
#include <tap.h>

#define TAP_DIRECTIVE_SKIP "SKIP"
#define TAP_DIRECTIVE_TODO "TODO"
#define TAP_PIPE_RX 0
#define TAP_PIPE_TX 1

struct test {
    char *description;
    test_t funct;
    size_t id;
};

#endif /* __TAP_TEST_H__ */
