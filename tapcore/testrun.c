#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tap.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "internal.h"

static int tap_process_testrun_output(struct test_run *testrun) {
    struct test *test = &testrun->test;
    tap_cmd_t *cmd = testrun->cmd;
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;
    int err = 0;

    test_fp = fdopen(testrun->outfd, "r");
    if (!test_fp) {
        err = errno;
        tap_print_internal_error(err, test, "failed open pipe as stream");
        return err;
    }

    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        tap_cmd_t *line_cmd = NULL;

        if (bytes == 0 || *line == '\n') {
            continue;
        }

        err = tap_parse_cmd(line, &line_cmd);
        if (err != 0) {
            tap_print_internal_error(err, test,
                                     "failed to parse tap cmd from line");
            break;
        }
        if (!line_cmd) {
            /* Debug from the test, output as TAP comment */
            tap_printf_line("# %s\n", line);
            continue;
        }
        if (cmd) {
            /* Only allow one directive command per test, warn the extra is
             * ignored */
            tap_printf_line("# One directive command per test: ignoring '%s'",
                            line_cmd->str);
            free(line_cmd);
            continue;
        }
        cmd = line_cmd;
    }
    if (err != 0) {
        free(cmd);
    } else if (cmd) {
        testrun->cmd = cmd;
    }
    free(line);
    return err;
}

static void tap_run_test_and_exit(struct test *test) {
    int res;

    res = test->funct();
    fflush(NULL);
    _exit(res);
}

static void tap_exit_testrun(struct test_run *run) {
    if (run->outfd != -1) {
        close(run->outfd);
        run->outfd = -1;
    }
    run->pid = -1;
    run->exited = true;
}

int tap_start_testrun(struct test *test, struct test_run *run) {
    struct timespec start;
    int pipefd[2] = {-1, -1};
    pid_t cpid;
    int err;

    /* Communicate fail condition on pipe */
    err = tap_pipe_setup(pipefd);
    if (err != 0) {
        tap_print_internal_error(err, test, "failed to create pipe");
        return err;
    }

    err = clock_gettime(CLOCK_MONOTONIC, &start);
    if (err != 0) {
        tap_print_internal_error(err, test, "failed to get monotonic time");
        return err;
    }

    /* Flush stdout and stderr to avoid child duplicating buffered output */
    fflush(NULL);
    cpid = fork();
    if (cpid == 0) {
        close(pipefd[TAP_PIPE_RX]);
        dup2(pipefd[TAP_PIPE_TX], STDOUT_FILENO);
        dup2(pipefd[TAP_PIPE_TX], STDERR_FILENO);
        close(pipefd[TAP_PIPE_TX]);
        /* Child process will run test and exit */
        tap_run_test_and_exit(test);
        /* Child should have already exited */
        _exit(EINVAL);
    }
    if (cpid == -1) {
        err = errno;
        close(pipefd[TAP_PIPE_RX]);
        close(pipefd[TAP_PIPE_TX]);
        tap_print_internal_error(err, test, "failed to fork process");
        return err;
    }
    close(pipefd[TAP_PIPE_TX]);

    *run = (struct test_run){
        .test = *test,
        .outfd = pipefd[TAP_PIPE_RX],
        .pid = cpid,
        .exitstatus = -1,
        .cmd = NULL,
        .duration =
            (struct tap_duration){
                .t0 = start,
            },
    };
    return 0;
}

int tap_wait_for_testrun(struct test_run *runs, size_t n_runs,
                         struct pollfd *fds) {
    for (size_t idx = 0; idx < n_runs; idx++) {
        fds[idx] = (struct pollfd){
            .fd = runs[idx].outfd,
            .events = POLLIN,
        };
    }

    while (true) {
        unsigned int n_exited = 0;
        int nfds_ready;

        nfds_ready = poll(fds, n_runs, 1000);
        if (nfds_ready == -1 && errno != EINTR) {
            return errno;
        } else if (nfds_ready < 1) {
            continue;
        }

        /* First pass to find exited processes */
        for (size_t idx = 0; idx < n_runs; idx++) {
            struct test_run *run;
            struct pollfd *pfd;
            int err;
            int res;

            pfd = &fds[idx];
            /* Check if the child has closed its end of the pipe */
            if ((pfd->revents & (POLLERR | POLLHUP)) == 0) {
                continue;
            }

            /* Attempt to reap the child that closed its stdout/stderr */
            run = &runs[idx];
            res = waitpid(run->pid, &run->exitstatus, WNOHANG);
            if (res < 0) {
                return errno;
            }
            if (res == 0) {
                continue;
            }
            err = clock_gettime(CLOCK_MONOTONIC, &run->duration.t1);
            if (err != 0) {
                tap_print_internal_error(err, &run->test,
                                         "failed to get monotonic time");
                return err;
            }

            if ((pfd->revents & POLLIN) != 0) {
                err = tap_process_testrun_output(run);
                if (err != 0) {
                    tap_exit_testrun(run);
                    return err;
                }
            }

            tap_exit_testrun(run);
            n_exited++;
        }
        if (n_exited > 0) {
            /* Prioritise reaping processes to reading output */
            break;
        }

        /* Second pass to read any data from still running processes */
        for (size_t idx = 0; idx < n_runs; idx++) {
            struct pollfd *pfd;
            int err;

            pfd = &fds[idx];
            if ((pfd->revents & POLLIN) == 0) {
                continue;
            }
            err = tap_process_testrun_output(&runs[idx]);
            if (err != 0) {
                return err;
            }
        }
    }

    return 0;
}

void tap_cleanup_testrun(struct test_run *run) {
    tap_exit_testrun(run);
    free(run->cmd);
    run->cmd = NULL;
    run->test = (struct test){0};
}
