#ifndef __INTERNAL_H__
#define __INTERNAL_H__
#include <poll.h>
#include <stdbool.h>
#include <tapstruct.h>
#include <taptest.h>

struct test_run {
    struct test test;
    tap_cmd_t *cmd;
    pid_t pid;
    int outfd;
    int exitstatus;
};

#endif /* __INTERNAL_H__ */
