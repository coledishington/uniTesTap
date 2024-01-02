#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tap.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <unistd.h>

#include "internal.h"

static int tap_process_testrun_output(struct test_run *testrun) {
    tap_cmd_t *cmd = NULL;
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;
    int err = 0;

    test_fp = fdopen(testrun->outfd, "r");
    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        tap_cmd_t *line_cmd = NULL;

        if (bytes == 0 || *line == '\n') {
            continue;
        }

        err = tap_parse_cmd(line, &line_cmd);
        if (err != 0) {
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

int tap_start_testrun(struct test *test, struct test_run *run) {
    int pipefd[2] = {-1, -1};
    pid_t cpid;
    int err;

    /* Communicate fail condition on pipe */
    err = tap_pipe_setup(pipefd);
    if (err != 0) {
        tap_print_internal_error(err, test, "failed to create pipe");
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
    };
    return 0;
}

int tap_wait_for_testrun(struct test_run *run) {
    int exitstatus;
    pid_t rpid;
    int err;

    err = tap_process_testrun_output(run);
    if (err != 0) {
        return err;
    }

    rpid = waitpid(run->pid, &exitstatus, 0);
    if (rpid == -1) {
        err = errno;
        tap_print_internal_error(err, &run->test, "failed waiting for test");
        return err;
    }

    run->exitstatus = exitstatus;
    return 0;
}

void tap_cleanup_testrun(struct test_run *testrun) {
    close(testrun->outfd);
    testrun->outfd = -1;
    free(testrun->cmd);
    testrun->cmd = NULL;
    testrun->pid = -1;
}
