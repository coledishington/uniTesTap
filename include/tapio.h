#ifndef __TAP_IO_H__
#define __TAP_IO_H__
#include <taptest.h>

int tap_pipe_setup(int fds[2]);

void tap_print_testpoint(bool success, struct test *test,
                         const char *directive);

void tap_print_internal_error(int err, struct test *test, const char *reason);

#endif /* __TAP_IO_H__ */
