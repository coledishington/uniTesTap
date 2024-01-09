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
    struct tap_duration duration;
    bool exited;
};

int tap_start_testrun(struct test *test, struct test_run *testrun);

int tap_wait_for_testrun(struct test_run *testruns, size_t n_runs,
                         struct pollfd *test_poll);

void tap_cleanup_testrun(struct test_run *testrun);

#endif /* __INTERNAL_H__ */
