#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <tapstruct.h>

int tap_cmd_strndup(enum tap_cmd_type ctype, const char *line, size_t n_copy,
                    struct tap_cmd **d_cmd) {
    struct tap_cmd *cmd;

    cmd = calloc(1, sizeof(*cmd) + n_copy + 1);
    if (!cmd) {
        return errno;
    }
    cmd->type = ctype;
    memcpy(cmd->str, line, n_copy);
    *d_cmd = cmd;
    return 0;
}
