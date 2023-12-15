#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

void tap_print_testpoint(bool success, struct test *test,
                         const char *directive) {
    tap_string_t *tstr;
    char *str;

    tstr = tap_string_ctor(NULL);
    tap_string_concat_printf(tstr, "%s %zu", success ? "ok" : "not ok",
                             test->id);
    if (test->description) {
        tap_string_concat(tstr, " - ");
        tap_string_concat(tstr, test->description);
    }
    if (directive) {
        tap_string_concat(tstr, " # ");
        tap_string_concat(tstr, directive);
    }
    str = tap_string_dtor(tstr, false);
    printf("%s\n", str);
    free(str);
}

void tap_print_internal_error(int err, struct test *test, const char *reason) {
    tap_string_t *tstr;
    char *str;

    tstr = tap_string_ctor(NULL);

    tap_string_concat_printf(tstr, "# internal test runner error %s(%d): ",
                             strerror(err), err, reason);
    str = tap_string_dtor(tstr, false);
    printf("%s\n", str);
    free(str);
    tap_print_testpoint(false, test, NULL);
}