#include <stdio.h>
#include <stdlib.h>
#include <tap.h>

#include "internal.h"

static void test_output(FILE *out) {
    /* Lines starting with a colon */
    fprintf(out, ":UNSUPPORED cmds do not work\n");
    fprintf(out, ":FAKE cmds do not work\n");
    fprintf(out, ": empty commands do not work\n");

    /* Normal output */
    fprintf(out, "SKIP is just a word\n");
    fprintf(out, "TODO is just a word\n");
    fprintf(out, "Bail out! is just a word\n");
    fprintf(out, "word is just a word\n");
}

static int pass_skipped(void) {
    printf(":SKIP don't need this test");
    return 0;
}

static int pass_skipped_plus_output(void) {
    test_output(stdout);
    test_output(stderr);
    return pass_skipped();
}

static int fail_skipped(void) {
    printf(":SKIP don't need this test");
    return 1;
}

static int pass_todo(void) {
    printf(":TODO will this ever be done?");
    return 0;
}

static int pass_todo_plus_output(void) {
    test_output(stdout);
    test_output(stderr);
    return pass_todo();
}

static int fail_todo(void) {
    printf(":TODO will this ever be done?");
    return 1;
}

static int pass_bail(void) {
    printf(":Bail out! Jump ship");
    return 0;
}

static int pass_bail_plus_output(void) {
    test_output(stdout);
    test_output(stderr);
    return pass_bail();
}

static int fail_bail(void) {
    printf(":Bail out! Jump ship");
    return 1;
}

int main(void) {
    tap_register(NULL, pass_skipped, NULL);
    tap_register(NULL, fail_skipped, NULL);
    tap_register(NULL, pass_todo, NULL);
    tap_register(NULL, fail_todo, NULL);
    /* Noisy tests */
    tap_register(NULL, pass_skipped_plus_output, NULL);
    tap_register(NULL, pass_todo_plus_output, NULL);
    tap_runall_and_cleanup(NULL);

    printf("\n");

    tap_register(NULL, pass, NULL);
    tap_register(NULL, pass_bail, NULL);
    tap_register(NULL, pass, NULL);
    tap_runall_and_cleanup(NULL);

    printf("\n");

    tap_register(NULL, pass, NULL);
    tap_register(NULL, fail_bail, NULL);
    tap_register(NULL, pass, NULL);
    tap_runall_and_cleanup(NULL);

    printf("\n");

    tap_register(NULL, pass, NULL);
    tap_register(NULL, pass_bail_plus_output, NULL);
    tap_register(NULL, pass, NULL);
    tap_runall_and_cleanup(NULL);
}
