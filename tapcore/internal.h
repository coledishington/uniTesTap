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

int tap_start_testrun(struct test *test, struct test_run *testrun);

int tap_wait_for_testrun(struct test_run *testrun);

void tap_cleanup_testrun(struct test_run *testrun);

#endif /* __INTERNAL_H__ */
