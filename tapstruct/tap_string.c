/**
 * @file tap_string.c
 *
 * Implements a simple dynamic string buffer.
 */
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <tapstruct.h>
#include <taputil.h>

#include "config.h"

struct tap_string {
    size_t allocated;
    size_t len;
    char *data;
};

static size_t tap_ceil_power_two(size_t n) {
    /* Bit twiddling
     * See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
     */
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#if SIZE_MAX == UINT64_MAX
    n |= n >> 32;
#elif SIZE_MAX != UINT32_MAX
    STATIC_ASSERT(false, "size_t is not 32 or 64 bits");
#endif
    n++;
    /* Handle the special case of zero */
    n += n == 0;
    return n;
}

static size_t tap_next_alloc_size(size_t n) {
    /* Avoid the smaller numbers where exponential growth is slow */
    if (n < 16) {
        return 16;
    }
    return tap_ceil_power_two(n);
}

static int tap_string_grow(tap_string_t *tstr, size_t new_len) {
    size_t alloc_len;
    char *new_data;

    alloc_len = tap_next_alloc_size(new_len + 1);
    new_data = realloc(tstr->data, alloc_len);
    if (!new_data) {
        return errno;
    }
    tstr->data = new_data;
    tstr->allocated = alloc_len;
    return 0;
}

static int tap_string_grow_to_fit(tap_string_t *tstr, size_t append_len) {
    size_t new_len;

    new_len = tstr->len + append_len;
    if (new_len + 1 <= tstr->allocated) {
        return 0;
    }
    return tap_string_grow(tstr, new_len + 1);
}

static void tap_string_concat_danger(tap_string_t *tstr, const char *str,
                                     size_t str_len) {
    char *tstr_end;

    tstr_end = tstr->data + tstr->len;
    strcpy(tstr_end, str);
    tstr->len += str_len;
}

int tap_string_ctor(tap_string_t **d_tstr, const char *fmt, ...) {
    tap_string_t *tstr;
    va_list ap;
    int err;

    tstr = calloc(1, sizeof(*tstr));
    if (!tstr) {
        return errno;
    }
    if (!fmt) {
        *d_tstr = tstr;
        return 0;
    }
    va_start(ap, fmt);
    err = tap_string_concat_vprintf(tstr, fmt, ap);
    va_end(ap);
    if (err != 0) {
        return err;
    }
    *d_tstr = tstr;
    return 0;
}

const char *tap_string_borrow(tap_string_t *tstr) { return tstr->data; }

int tap_string_concat(tap_string_t *tstr, const char *str) {
    size_t str_len = strlen(str);
    int err;

    err = tap_string_grow_to_fit(tstr, str_len);
    if (err != 0) {
        return err;
    }
    tap_string_concat_danger(tstr, str, str_len);
    return 0;
}

int tap_string_concat_vprintf(tap_string_t *tstr, const char *fmt, va_list ap) {
    char buf[256] = {0};
    va_list ap_copy;
    int n_written;
    int err;

    /* Copy ap for later if buf doesn't fit the resultant string */
    va_copy(ap_copy, ap);
    n_written = vsnprintf(buf, ARRAY_LEN(buf), fmt, ap_copy);
    va_end(ap_copy);
    if (n_written < 0) {
        return EIO;
    }
    /* n_written excludes the space needed for the null byte */
    n_written++;

    err = tap_string_grow_to_fit(tstr, n_written);
    if (err != 0) {
        return err;
    }

    if (n_written <= ARRAY_LEN(buf)) {
        tap_string_concat_danger(tstr, buf, n_written - 1);
        return 0;
    }

    n_written =
        vsnprintf(tstr->data + tstr->len, tstr->allocated - tstr->len, fmt, ap);
    return n_written < 0 ? EIO : 0;
}

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...) {
    va_list ap;
    int err;

    va_start(ap, fmt);
    err = tap_string_concat_vprintf(tstr, fmt, ap);
    va_end(ap);
    return err;
}

void tap_string_dtor(tap_string_t *tstr) {
    if (!tstr) {
        return;
    }
    free(tstr->data);
    free(tstr);
}
