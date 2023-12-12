#include <stdio.h>
#include <stdlib.h>
#include <tap.h>

static int pass_skipped(void) {
    printf(":SKIP don't need this test");
    return 0;
}

static int fail_skipped(void) {
    printf(":SKIP don't need this test");
    return 1;
}

static int pass_todo(void) {
    printf(":TODO will this ever be done?");
    return 0;
}

static int fail_todo(void) {
    printf(":TODO will this ever be done?");
    return 1;
}

int main(void) {
    tap_register(NULL, pass_skipped, NULL);
    tap_register(NULL, fail_skipped, NULL);
    tap_register(NULL, pass_todo, NULL);
    tap_register(NULL, fail_todo, NULL);
    tap_runall_and_cleanup(NULL);
}
