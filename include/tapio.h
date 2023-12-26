#ifndef __TAP_IO_H__
#define __TAP_IO_H__
#include <tapstruct.h>
#include <taptest.h>

struct tap_duration {
    struct timespec t1, t0;
};

struct tap_seconds {
    char mprefix;
    double secs;
};

struct tap_seconds tap_duration_to_secs(struct tap_duration *d);

int tap_pipe_setup(int fds[2]);

int tap_parse_cmd(const char *line, struct tap_cmd **d_cmd);

int tap_trim_string(const char *in, char **out);

void tap_replace_string(char *in, char c, char d);

void tap_print_line(const char *line);

void tap_printf_line(const char *fmt, ...);

void tap_print_testpoint(bool success, struct test *test,
                         struct tap_duration *duration, const char *directive);

void tap_print_internal_error(int err, struct test *test, const char *reason);

#endif /* __TAP_IO_H__ */
