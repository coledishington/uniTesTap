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
#include <unistd.h>

#include "config.h"
#include "internal.h"

#define MAX_TEST_PROCESSES 32
#define MAX_TESTS 200

struct TAP {
    struct test tests[MAX_TESTS];
    size_t n_tests;
    unsigned int n_runners;
};

/* Static variable used if no state is passed by caller */
static struct TAP *handle = NULL;

static inline TAP *get_handle(struct TAP *tap) { return tap ? tap : handle; }

static inline int get_or_create_handle(struct TAP **d_tap) {
    int err;

    *d_tap = get_handle(*d_tap);
    if (*d_tap) {
        return 0;
    }

    err = tap_init(&handle);
    if (err != 0) {
        return err;
    }
    *d_tap = handle;
    return 0;
}

static int tap_report_testrun(struct test_run *run) {
    struct test *test = &run->test;
    int wres = run->exitstatus;
    const char *directive = NULL;
    bool passed = false;
    int err;

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
        printf("# test %zu: terminated via %s(%d)\n", test->id, sig_name, sig);
    } else {
        printf("# test %zu: exited for unknown reason\n", test->id);
    }

    if (tap_cmd_is_directive(run->cmd)) {
        directive = run->cmd->str;
    }
    err = tap_print_testpoint(passed, test, &run->duration, directive);
    return err;
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

int tap_set_option(TAP *tap, TAP_OPTION option, ...) {
    va_list ap;
    int err;

    if (option != TAP_OPTION_N_RUNNERS) {
        return EINVAL;
    }

    err = get_or_create_handle(&tap);
    if (err != 0) {
        return err;
    }

    va_start(ap, option);
    tap->n_runners = va_arg(ap, int);
    va_end(ap);
    return 0;
}

int tap_register(struct TAP *tap, test_t funct, const char *in_description) {
    char *description = NULL;
    int err;

    err = get_or_create_handle(&tap);
    if (err != 0) {
        return err;
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
    struct test_run runs[MAX_TESTS] = {0};
    struct test_run running[MAX_TEST_PROCESSES] = {0};
    struct pollfd fds[MAX_TEST_PROCESSES];
    size_t n_running_slots, next_testid;
    unsigned int n_running, n_finished;
    bool bailed = false;
    int err = 0;

    tap = get_handle(tap);

    for (size_t idx = 0; idx < MAX_TEST_PROCESSES; idx++) {
        running[idx] = (struct test_run){.outfd = -1, .pid = -1};
    }

    /* Handle running less tests than available running slots */
    n_running_slots = tap->n_runners;
    if (n_running_slots <= 0) {
        /* Default to using all cores. One will monitor the tests */
        n_running_slots = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    }
    if (tap->n_tests < n_running_slots) {
        n_running_slots = tap->n_tests;
    }
    if (n_running_slots > MAX_TEST_PROCESSES) {
        n_running_slots = MAX_TEST_PROCESSES;
    }

    /* Trigger and wait on tests */
    printf("1..%zu\n", tap->n_tests);
    for (next_testid = 0, n_running = 0, n_finished = 0;
         (n_finished < tap->n_tests && !bailed) || n_running > 0;) {
        /* Start tests in any free slots */
        for (size_t ridx = 0;
             ridx < n_running_slots && next_testid < tap->n_tests && !bailed;
             ridx++) {
            struct test_run *run;
            struct test *test;

            run = &running[ridx];
            if (run->test.id != 0) {
                /* Slot is currently taken */
                continue;
            }

            test = &tap->tests[next_testid];
            err = tap_start_testrun(test, run);
            if (err != 0) {
                bailed = true;
                break;
            }
            next_testid++;
            n_running++;
        }

        err = tap_wait_for_testrun(running, n_running_slots, fds);
        if (err != 0) {
            bailed = true;
            break;
        }

        /* Check for any bailed tests if nothing has bailed */
        for (size_t ridx = 0; !bailed && ridx < n_running_slots; ridx++) {
            struct test_run *run;

            run = &running[ridx];
            if (tap_cmd_is_bailed(run->cmd)) {
                bailed = true;
            }
        }

        /* Report on any finished tests */
        for (size_t ridx = 0; ridx < n_running_slots; ridx++) {
            struct test_run *run;

            run = &running[ridx];
            if (!run->exited || run->test.id == 0) {
                continue;
            }
            runs[run->test.id - 1] = *run;
            running[ridx] = (struct test_run){.outfd = -1, .pid = -1};
            n_finished++;
            n_running--;
        }
    }

    /* Report testruns up to first bail */
    for (size_t idx = 0; idx < ARRAY_LEN(runs); idx++) {
        struct test_run *run;

        run = &runs[idx];
        /* Test runs past a bail should not be trusted */
        if (!run->exited) {
            break;
        } else if (tap_cmd_is_bailed(run->cmd)) {
            err = tap_print_line(run->cmd->str);
            break;
        }
        tap_report_testrun(run);
    }
    /* Cleanup after all testruns */
    for (size_t idx = 0; idx < ARRAY_LEN(runs); idx++) {
        struct test_run *run;

        run = &runs[idx];
        tap_cleanup_testrun(run);
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
