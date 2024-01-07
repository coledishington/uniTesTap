#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <taputil.h>

#include "config.h"

void tap_print_line(const char *line) {
    char *newline;
    int width;

    /* Always print one newline, but never more */
    newline = strchr(line, '\n');
    if (newline) {
        width = newline - line;
    } else {
        width = strlen(line);
    }
    printf("%.*s\n", width, line);
}

void tap_printf_line(const char *fmt, ...) {
    tap_string_t *tstr;
    va_list ap;
    char *str;

    tstr = tap_string_ctor(NULL);
    va_start(ap, fmt);
    tap_string_concat_vprintf(tstr, fmt, ap);
    va_end(ap);
    str = tap_string_dtor(tstr, false);

    tap_print_line(str);
    free(str);
}

void tap_print_testpoint(bool success, struct test *test,
                         struct tap_duration *duration, const char *directive) {
    tap_string_t *tstr;
    double secs;
    char *str;

    tstr = tap_string_ctor(NULL);
    tap_string_concat_printf(tstr, "%s %zu", success ? "ok" : "not ok",
                             test->id);
    tap_string_concat(tstr, " - ");
    if (test->description) {
        tap_string_concat_printf(tstr, "%s ", test->description);
    }
    secs = tap_duration_to_secs(duration);
    tap_string_concat_printf(tstr, "(%.3gs)", secs);

    if (directive) {
        tap_string_concat(tstr, " # ");
        tap_string_concat(tstr, directive);
    }
    str = tap_string_dtor(tstr, false);
    tap_print_line(str);
    free(str);
}

void tap_print_internal_error(int err, struct test *test, const char *reason) {
    tap_string_t *tstr;
    char *str;

    tstr = tap_string_ctor(NULL);
    tap_string_concat(tstr, "# internal test runner error");
    if (test) {
        tap_string_concat_printf(tstr, " whilst handling test %zu", test->id);
    }
    tap_string_concat_printf(tstr, ": %s(%d) %s", strerror(err), err, reason);
    str = tap_string_dtor(tstr, false);

    tap_print_line(str);
    free(str);
}
