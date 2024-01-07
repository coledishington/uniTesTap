#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <tap.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <taputil.h>

#include "config.h"
#include "internal.h"

#define MAX_TESTS 200

struct TAP {
    size_t n_tests;
    struct test tests[MAX_TESTS];
};

/* Static variable used if no state is passed by caller */
static struct TAP *handle = NULL;

static inline TAP *get_handle(struct TAP *tap) { return tap ? tap : handle; }

static void tap_report_testrun(struct test_run *run) {
    struct test *test = &run->test;
    int wres = run->exitstatus;
    const char *directive = NULL;
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

    if (tap_cmd_is_directive(run->cmd)) {
        directive = run->cmd->str;
    }
    tap_print_testpoint(passed, test, &run->duration, directive);
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
    int err;

    tap = get_handle(tap);
    if (!tap) {
        err = tap_init(&handle);
        if (err != 0) {
            return err;
        }
        tap = handle;
    }

    assert(tap->n_tests + 1 < MAX_TESTS);

    if (in_description) {
        err = tap_trim_string(in_description, &description);
        if (err != 0) {
            return err;
        }
        tap_replace_string(description, '\n', ' ');
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
        struct test *test = &tap->tests[idx];
        struct test_run testrun = {0};

        err = tap_start_testrun(test, &testrun);
        if (err != 0) {
            break;
        }

        err = tap_wait_for_testrun(&testrun);
        if (err != 0) {
            break;
        }

        if (tap_cmd_is_bailed(testrun.cmd)) {
            tap_print_line(testrun.cmd->str);
            tap_cleanup_testrun(&testrun);
            break;
        }

        tap_report_testrun(&testrun);
        tap_cleanup_testrun(&testrun);
    }

    if (err != 0) {
        printf(TAP_BAILOUT " internal test runner error %s(%d): ",
               strerror(err), err);
    }
    return err;
}

void tap_cleanup(struct TAP *tap) {
    bool passed_handle;

    passed_handle = !!tap;
    tap = get_handle(tap);
    if (!tap) {
        return;
    }

    for (size_t i = 0; i < tap->n_tests; i++) {
        free(tap->tests[i].description);
    }
    free(tap);

    if (!passed_handle) {
        handle = NULL;
    }
}
