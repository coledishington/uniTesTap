#ifndef __TAP_STRUCT_H__
#define __TAP_STRUCT_H__
#include <stdbool.h>

struct tap_string;
typedef struct tap_string tap_string_t;

tap_string_t *tap_string_ctor(const char *str);

int tap_string_concat(tap_string_t *tstr, const char *str);

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...);

char *tap_string_dtor(tap_string_t *tstr, bool free_str);

#endif /* __TAP_STRUCT_H__ */
