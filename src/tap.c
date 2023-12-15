#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tap.h>
#include <unistd.h>

#include "internal.h"

#define MAX_TESTS 10

struct cfg {
    size_t n_tests;
    struct test tests[MAX_TESTS];
};

struct cfg cfg = {
    .n_tests = 0,
};

static void tap_report_test(struct test *test, int wres,
                            const char *directive) {
    bool passed = false;

    if (WIFEXITED(wres)) {
        passed = WEXITSTATUS(wres) == 0;
    } else if (WIFSIGNALED(wres)) {
        const char *sig_name;
        int sig;

        sig = WTERMSIG(wres);
        sig_name = strsignal(sig);
        if (!sig_name) {
            sig_name = "UNKNOWN";
        }
        printf("# test terminated via %s(%d)\n", sig_name, sig);
    } else {
        printf("# test %zu exited for unknown reason\n", test->id);
    }
    tap_print_testpoint(passed, test, directive);
}

static char *tap_process_test_output(int test_fd) {
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;

    test_fp = fdopen(test_fd, "r");
    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        if (bytes == 0 || *line == '\n') {
            continue;
        }
        if (strncasecmp(":skip", line, sizeof(":skip") - 1) == 0) {
            if (line[bytes - 1] == '\n') {
                line[bytes - 1] = '\0';
            }
            memmove(line, line + 1, bytes);
            return line;
        }

        /* Print remaining test output as TAP comment */
        printf("# %s", line);
    }
    free(line);
    return NULL;
}

static void tap_run_test_and_exit(struct test *test) {
    int res;

    res = test->funct();
    fflush(NULL);
    _exit(res);
}

static int tap_evaluate(struct test *test) {
    int pipefd[2] = {-1, -1};
    char *directive;
    int res, wres;
    pid_t cpid;

    res = pipe(pipefd);
    if (res == -1) {
        int err = errno;
        tap_print_internal_error(err, test, "failed to create pipe");
        return err;
    }

    /* Flush stdout and stderr to avoid child duplicating buffered output */
    fflush(NULL);
    cpid = fork();
    if (cpid == 0) {
        close(pipefd[0]); /* Close read side */
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        /* Child process will run test and exit */
        tap_run_test_and_exit(test);
        _exit(-1);
    }
    if (cpid == -1) {
        int err;

        err = errno;
        close(pipefd[0]);
        close(pipefd[1]);
        tap_print_internal_error(err, test, "failed to fork process");
        return err;
    }
    close(pipefd[1]);

    directive = tap_process_test_output(pipefd[0]);
    close(pipefd[0]);

    /* Wait for child to exit */
    cpid = waitpid(cpid, &wres, 0);
    if (cpid < 0) {
        /* Child process is lost */
        return errno;
    }

    /* Report test success */
    tap_report_test(test, wres, directive);
    free(directive);
    return 0;
}

int tap_register(test_t funct, const char *in_description) {
    char *description = NULL;

    assert(cfg.n_tests + 1 < MAX_TESTS);

    if (in_description) {
        description = strdup(in_description);
        if (!description) {
            return errno;
        }
    }

    cfg.tests[cfg.n_tests] = (struct test){
        .id = cfg.n_tests + 1,
        .funct = funct,
        .description = description,
    };
    cfg.n_tests++;
    return 0;
}

int tap_runall(void) {
    int err = 0;

    printf("1..%zu\n", cfg.n_tests);
    for (size_t idx = 0; idx < cfg.n_tests; idx++) {
        struct test *test;
        int err;

        test = &cfg.tests[idx];
        err = tap_evaluate(test);
        if (err != 0) {
            printf("Bail out! internal test runner error %s(%d): ",
                   strerror(err), err);
            break;
        }
    }
    return err;
}
