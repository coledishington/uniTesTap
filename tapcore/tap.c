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
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <unistd.h>

#define MAX_TESTS 200

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

static int tap_parse_directive(const char *line, char **d_directive) {
    const char *newline;
    char *directive;
    size_t cmd_len;

    if (strncasecmp(":" TAP_DIRECTIVE_SKIP, line, sizeof(TAP_DIRECTIVE_SKIP)) !=
            0 &&
        strncasecmp(":" TAP_DIRECTIVE_TODO, line, sizeof(TAP_DIRECTIVE_TODO)) !=
            0) {
        return 0;
    }

    /* Copy everything before the first newline */
    newline = strchr(line, '\n');
    if (newline) {
        cmd_len = newline - line;
    } else {
        cmd_len = strlen(line);
    }

    /* Skip past the ':' */
    directive = strndup(line + 1, cmd_len - 1);
    if (!directive) {
        return errno;
    }

    *d_directive = directive;
    return 0;
}

static int tap_process_test_output(int test_fd, char **d_directive) {
    char *directive = NULL;
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;

    test_fp = fdopen(test_fd, "r");
    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        char *ln_directive = NULL;
        int err;

        if (bytes == 0 || *line == '\n') {
            continue;
        }
        err = tap_parse_directive(line, &ln_directive);
        if (err != 0) {
            break;
        }
        if (!ln_directive) {
            /* Debug from the test, output as TAP comment */
            printf("# %s\n", line);
            continue;
        }
        if (directive) {
            /* Only allow one directive command per test, warn the extra is
             * ignored */
            printf("# One directive command per test: ignoring '%s'\n",
                   ln_directive);
            free(ln_directive);
        }
        directive = ln_directive;
    }
    if (directive) {
        *d_directive = directive;
    }
    free(line);
    return 0;
}

static void tap_run_test_and_exit(struct test *test) {
    int res;

    res = test->funct();
    fflush(NULL);
    _exit(res);
}

static int tap_evaluate(struct test *test) {
    int pipefd[2] = {-1, -1};
    char *directive = NULL;
    int wres, err;
    pid_t cpid;

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
        _exit(-1);
    }
    if (cpid == -1) {
        err = errno;
        close(pipefd[TAP_PIPE_RX]);
        close(pipefd[TAP_PIPE_TX]);
        tap_print_internal_error(err, test, "failed to fork process");
        return err;
    }
    close(pipefd[TAP_PIPE_TX]);

    err = tap_process_test_output(pipefd[TAP_PIPE_RX], &directive);
    if (err != 0) {
        free(directive);
        return err;
    }
    close(pipefd[TAP_PIPE_RX]);

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
