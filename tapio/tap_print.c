#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taptest.h>
#include <taputil.h>

#include "config.h"

int tap_print_line(const char *line) {
    char *newline;
    int width;

    /* Always print one newline, but never more */
    newline = strchr(line, '\n');
    if (newline) {
        width = newline - line;
    } else {
        width = strlen(line);
    }
    if (printf("%.*s\n", width, line) < 0) {
        return EIO;
    };
    return 0;
}

int tap_printf_line(const char *fmt, ...) {
    tap_string_t *tstr;
    va_list ap;
    int err;

    err = tap_string_ctor(&tstr, NULL);
    if (err != 0) {
        return err;
    }
    va_start(ap, fmt);
    err = tap_string_concat_vprintf(tstr, fmt, ap);
    va_end(ap);
    if (err != 0) {
        return err;
    }
    err = tap_print_line(tap_string_borrow(tstr));
    tap_string_dtor(tstr);
    return err;
}

int tap_print_testpoint(bool success, struct test *test,
                        struct tap_duration *duration, const char *directive) {
    struct tap_seconds secs;
    tap_string_t *tstr;
    const char *ok;
    int err;

    ok = success ? "ok" : "not ok";
    err = tap_string_ctor(&tstr, "%s %zu - ", ok, test->id);
    if (err != 0) {
        goto done;
    }

    if (test->description) {
        err = tap_string_concat_printf(tstr, "%s ", test->description);
        if (err != 0) {
            goto done;
        }
    }

    secs = tap_duration_to_secs(duration);
    if (secs.mprefix != 0) {
        err = tap_string_concat_printf(tstr, "(%.3g%cs)", secs.secs,
                                       secs.mprefix);
    } else {
        err = tap_string_concat_printf(tstr, "(%.3gs)", secs.secs);
    }
    if (err != 0) {
        goto done;
    }

    if (directive) {
        err = tap_string_concat_printf(tstr, " # %s", directive);
        if (err != 0) {
            goto done;
        }
    }

    err = tap_print_line(tap_string_borrow(tstr));
done:
    tap_string_dtor(tstr);
    return err;
}

int tap_print_internal_error(int internal_err, struct test *test,
                             const char *reason) {
    tap_string_t *tstr;
    int err;

    err = tap_string_ctor(&tstr, "# ");
    if (err != 0) {
        goto done;
    }

    if (test) {
        err = tap_string_concat_printf(tstr, " test %zu: ", test->id);
        if (err != 0) {
            goto done;
        }
    }
    err =
        tap_string_concat_printf(tstr, "internal test runner error : %s(%d) %s",
                                 strerror(internal_err), internal_err, reason);
    if (err != 0) {
        goto done;
    }

    err = tap_print_line(tap_string_borrow(tstr));
done:
    tap_string_dtor(tstr);
    return err;
}
