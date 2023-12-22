#ifndef __TAP_STRUCT_H__
#define __TAP_STRUCT_H__
#include <stdbool.h>

enum tap_cmd_type {
    tap_cmd_type_unknown = -1,
    tap_cmd_type_todo = 0,
    tap_cmd_type_skip,
    tap_cmd_type_bail,
};

struct tap_cmd {
    enum tap_cmd_type type;
    char str[];
};
typedef struct tap_cmd tap_cmd_t;

struct tap_string;
typedef struct tap_string tap_string_t;

int tap_cmd_strndup(enum tap_cmd_type type, const char *line, size_t n_copy,
                    tap_cmd_t **d_cmd);

tap_string_t *tap_string_ctor(const char *str);

int tap_string_concat(tap_string_t *tstr, const char *str);

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...);

char *tap_string_dtor(tap_string_t *tstr, bool free_str);

#endif /* __TAP_STRUCT_H__ */
