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

#define MAX_TESTS 10

struct test_cfg {
    char *description;
    test_t test;
};

struct cfg {
    size_t n_tests;
    struct test_cfg tests[MAX_TESTS];
};

struct cfg cfg = {
    .n_tests = 0,
};

static void tap_print_testpoint(bool success, size_t test_id,
                                const char *description) {
    printf("%s %zu%s%s\n", success ? "ok" : "not ok", test_id,
           description ? " - " : "", description ? description : "");
}

static void tap_report_test(size_t test_id, struct test_cfg *test_cfg,
                            int wres) {
    int res;

    if (WIFEXITED(wres)) {
        res = WEXITSTATUS(wres);
        tap_print_testpoint(res == 0, test_id, test_cfg->description);
    } else if (WIFSIGNALED(wres)) {
        int sig = WTERMSIG(wres);
        const char *sig_name = strsignal(sig);

        if (!sig_name) {
            sig_name = "UNKNOWN";
        }
        printf("# test terminated via %s(%d)\n", sig_name, sig);
        tap_print_testpoint(false, test_id, test_cfg->description);
    } else {
        printf("# test exited for unknown reason\n");
        tap_print_testpoint(false, test_id, test_cfg->description);
    }
}

static void tap_process_test_output(int test_fd) {
    size_t line_len = 0;
    char *line = NULL;
    ssize_t bytes;
    FILE *test_fp;

    test_fp = fdopen(test_fd, "r");
    for (; (bytes = getline(&line, &line_len, test_fp)) != -1;) {
        if (bytes == 0 || *line == '\n') {
            continue;
        }
        /* Print test output as TAP comment */
        printf("# %s", line);
    }
    free(line);
}

static void tap_run_test_and_exit(struct test_cfg *test_cfg) {
    int res;

    res = test_cfg->test();
    fflush(NULL);
    _exit(res);
}

static int tap_evaluate(size_t test_id, struct test_cfg *test_cfg) {
    int pipefd[2] = {-1, -1};
    int res, wres;
    pid_t cpid;

    res = pipe(pipefd);
    if (res == -1) {
        int err = errno;
        printf("# failed to create pipe %s(%d)\n", strerror(err), err);
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
        tap_run_test_and_exit(test_cfg);
        _exit(-1);
    }
    if (cpid == -1) {
        int err = errno;
        /* Child process spawning failed */
        printf("# internal test runner error %s(%d)\n", strerror(err), err);
        tap_print_testpoint(false, test_id, NULL);
        close(pipefd[0]);
        close(pipefd[1]);
        return err;
    }
    close(pipefd[1]);

    tap_process_test_output(pipefd[0]);
    close(pipefd[0]);

    /* Wait for child to exit */
    cpid = waitpid(cpid, &wres, 0);
    if (cpid < 0) {
        /* Child process is lost */
        return errno;
    }

    /* Report test success */
    tap_report_test(test_id, test_cfg, wres);
    return 0;
}

int tap_register(test_t test, const char *in_description) {
    char *description = NULL;

    assert(cfg.n_tests + 1 < MAX_TESTS);

    if (in_description) {
        description = strdup(in_description);
        if (!description) {
            return errno;
        }
    }

    cfg.tests[cfg.n_tests] = (struct test_cfg){
        .test = test,
        .description = description,
    };
    cfg.n_tests++;
    return 0;
}

int tap_runall(void) {
    int err = 0;

    printf("1..%zu\n", cfg.n_tests);
    for (size_t test_idx = 0; test_idx < cfg.n_tests; test_idx++) {
        struct test_cfg *test_cfg = &cfg.tests[test_idx];
        size_t test_id = test_idx + 1;
        err = tap_evaluate(test_id, test_cfg);
        if (err != 0) {
            printf("Bail out! internal test runner error %s(%d): ",
                   strerror(err), err);
            break;
        }
    }
    return err;
}
