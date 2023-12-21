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
    tap_register(pass_skipped, NULL);
    tap_register(fail_skipped, NULL);
    tap_register(pass_todo, NULL);
    tap_register(fail_todo, NULL);
    tap_runall();
}
