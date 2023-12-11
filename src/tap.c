#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <tap.h>
#include <unistd.h>

#define MAX_TESTS 10

struct cfg {
    size_t n_tests;
    test_t tests[MAX_TESTS];
};

struct cfg cfg = {
    .n_tests = 0,
};

static int tap_evaluate(size_t test_id, test_t test) {
    int res;
    int wres;
    pid_t cpid;

    cpid = fork();
    if (cpid == 0) {
        /* Child process will run test */
        res = test();
        _exit(res);
    } else if (cpid == -1) {
        /* Child process spawning failed */
        return errno;
    } else {
        /* Wait for child to exit */
        cpid = waitpid(cpid, &wres, 0);
        if (cpid < 0) {
            /* Child process is lost */
            return errno;
        }

        /* Report test success */
        if (WIFEXITED(wres)) {
            res = WEXITSTATUS(wres);
            switch (res) {
                case 0:
                    printf("ok %zu\n", test_id);
                    break;
                default:
                    printf("not ok %zu\n", test_id);
                    break;
            }
        } else if (WIFSIGNALED(wres)) {
            if (WCOREDUMP(wres)) {
                printf("not ok %zu # test crashed\n", test_id);
            }
        }
    }
    return 0;
}

void tap_register(test_t test) {
    assert(cfg.n_tests + 1 < MAX_TESTS);

    cfg.tests[cfg.n_tests] = test;
    cfg.n_tests++;
}

int tap_runall(void) {
    int err = 0;

    printf("1..%zu\n", cfg.n_tests);
    for (size_t test_idx = 0; test_idx < cfg.n_tests; test_idx++) {
        test_t test = cfg.tests[test_idx];
        size_t test_id = test_idx + 1;
        err = tap_evaluate(test_id, test);
        if (err != 0) {
            printf("Bail out! internal test runner error %s(%d): ",
                   strerror(err), err);
            break;
        }
    }
    return err;
}