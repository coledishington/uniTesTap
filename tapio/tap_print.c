#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tapstruct.h>
#include <taptest.h>

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
    tap_print_line(str);
    free(str);
}

void tap_print_internal_error(int err, struct test *test, const char *reason) {
    tap_string_t *tstr;
    char *str;

    tstr = tap_string_ctor(NULL);

    tap_string_concat_printf(tstr, "# internal test runner error %s(%d): ",
                             strerror(err), err, reason);
    str = tap_string_dtor(tstr, false);
    tap_print_line(str);
    free(str);
    tap_print_testpoint(false, test, NULL);
}
