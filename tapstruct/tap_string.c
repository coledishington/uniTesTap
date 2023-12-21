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

struct tap_string {
    size_t alloced;
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
    tstr->alloced = alloc_len;
    return 0;
}

static int tap_string_grow_to_fit(tap_string_t *tstr, size_t append_len) {
    size_t new_len;

    new_len = tstr->len + append_len;
    if (new_len + 1 <= tstr->alloced) {
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

tap_string_t *tap_string_ctor(const char *str) {
    tap_string_t *tstr;
    size_t str_len;

    tstr = calloc(1, sizeof(*tstr));
    if (!tstr) {
        return NULL;
    }
    if (!str) {
        return tstr;
    }
    str_len = strlen(str);
    if (tap_string_grow(tstr, str_len) != 0) {
        return NULL;
    }
    tap_string_concat_danger(tstr, str, str_len);
    return tstr;
}

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

int tap_string_concat_printf(tap_string_t *tstr, const char *fmt, ...) {
    char buf[256];
    int n_written;
    va_list ap;
    int err;

    va_start(ap, fmt);
    n_written = vsnprintf(buf, ARRAY_LEN(buf), fmt, ap);
    va_end(ap);
    if (n_written < 0) {
        return n_written;
    }

    err = tap_string_grow_to_fit(tstr, n_written);
    if (err != 0) {
        return err;
    }

    if (n_written + 1 <= ARRAY_LEN(buf)) {
        tap_string_concat_danger(tstr, buf, n_written);
        return 0;
    }

    va_start(ap, fmt);
    n_written =
        vsnprintf(tstr->data + tstr->len, tstr->alloced - tstr->len, fmt, ap);
    va_end(ap);
    return n_written < 0 ? -1 : 0;
}

char *tap_string_dtor(tap_string_t *tstr, bool free_str) {
    if (!tstr) {
        return NULL;
    }
    if (!free_str) {
        return tstr->data;
    }
    free(tstr->data);
    return NULL;
}
