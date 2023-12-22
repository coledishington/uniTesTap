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
#include <taputil.h>
#include <unistd.h>

#define MAX_TESTS 200

struct TAP {
    size_t n_tests;
    struct test tests[MAX_TESTS];
};

/* Static variable used if no state is passed by caller */
static struct TAP *handle = NULL;

static inline TAP *get_handle(struct TAP *tap) { return tap ? tap : handle; }

static void tap_report_test(struct test *test, int wres,
                            const char *directive) {
    bool passed;

    passed = false;
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

static int tap_process_test_output(int test_fd, tap_cmd_t **d_cmd) {
    tap_cmd_t *cmd = NULL;
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;

    test_fp = fdopen(test_fd, "r");
    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        tap_cmd_t *line_cmd = NULL;
        int err;

        if (bytes == 0 || *line == '\n') {
            continue;
        }

        err = tap_parse_cmd(line, &line_cmd);
        if (err != 0) {
            break;
        }
        if (!line_cmd) {
            /* Debug from the test, output as TAP comment */
            printf("# %s\n", line);
            continue;
        }
        if (cmd) {
            /* Only allow one directive command per test, warn the extra is
             * ignored */
            printf("# One directive command per test: ignoring '%s'",
                   line_cmd->str);
            free(line_cmd);
            continue;
        }
        cmd = line_cmd;
    }
    if (cmd) {
        *d_cmd = cmd;
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

static int tap_evaluate(struct test *test, bool *bailed) {
    int pipefd[2] = {-1, -1};
    tap_cmd_t *cmd = NULL;
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

    err = tap_process_test_output(pipefd[TAP_PIPE_RX], &cmd);
    if (err != 0) {
        free(cmd);
        return err;
    }
    close(pipefd[TAP_PIPE_RX]);

    /* Wait for child to exit */
    cpid = waitpid(cpid, &wres, 0);
    if (cpid < 0) {
        free(cmd);
        /* Child process is lost */
        return errno;
    }

    /* Test status is meaningless if the test bailed */
    *bailed = cmd && cmd->type == tap_cmd_type_bail;
    if (*bailed) {
        printf("%s\n", cmd->str);
        free(cmd);
        return 0;
    }

    /* Report test status */
    tap_report_test(test, wres, cmd ? cmd->str : NULL);
    free(cmd);
    return 0;
}

int tap_init(struct TAP **d_tap) {
    struct TAP *tap;

    if (d_tap) {
        d_tap = &handle;
    }
    tap = calloc(1, sizeof(*tap));
    if (!tap) {
        return errno;
    }
    *d_tap = tap;
    return 0;
}

int tap_register(struct TAP *tap, test_t funct, const char *in_description) {
    char *description = NULL;

    tap = get_handle(tap);
    if (!tap) {
        int err;

        err = tap_init(&handle);
        if (err != 0) {
            return err;
        }
        tap = handle;
    }

    assert(tap->n_tests + 1 < MAX_TESTS);

    if (in_description) {
        description = strdup(in_description);
        if (!description) {
            return errno;
        }
    }

    tap->tests[tap->n_tests] = (struct test){
        .id = tap->n_tests + 1,
        .funct = funct,
        .description = description,
    };
    tap->n_tests++;
    return 0;
}

int tap_runall(struct TAP *tap) {
    int err;

    tap = get_handle(tap);
    printf("1..%zu\n", tap->n_tests);
    for (size_t idx = 0; idx < tap->n_tests; idx++) {
        bool test_bailed = false;
        struct test *test;

        test = &tap->tests[idx];
        err = tap_evaluate(test, &test_bailed);
        if (err != 0) {
            printf(TAP_BAILOUT " internal test runner error %s(%d): ",
                   strerror(err), err);
            break;
        } else if (test_bailed) {
            break;
        }
    }
    return err;
}

void tap_cleanup(struct TAP *tap) {
    bool passed_handle;

    passed_handle = !!tap;
    tap = get_handle(tap);
    free(tap);

    if (!passed_handle) {
        handle = NULL;
    }
}
