#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <tap.h>
#include <tapio.h>
#include <tapstruct.h>
#include <taputil.h>

#define STARTSWITH_CMD(line, cmd) (strncasecmp(":" cmd, line, sizeof(cmd)) == 0)

static enum tap_cmd_type line_to_cmd_type(const char *line) {
    enum tap_cmd_type ctype;

    ctype = tap_cmd_type_unknown;
    if (STARTSWITH_CMD(line, TAP_DIRECTIVE_SKIP)) {
        ctype = tap_cmd_type_skip;
    } else if (STARTSWITH_CMD(line, TAP_DIRECTIVE_TODO)) {
        ctype = tap_cmd_type_todo;
    } else if (STARTSWITH_CMD(line, TAP_BAILOUT)) {
        ctype = tap_cmd_type_bail;
    }
    return ctype;
}

int tap_parse_cmd(const char *line, struct tap_cmd **d_cmd) {
    struct tap_cmd *cmd = NULL;
    enum tap_cmd_type ctype;
    char *formatted_line;
    int err;

    ctype = line_to_cmd_type(line);
    if (ctype == tap_cmd_type_unknown) {
        return 0;
    }

    err = tap_trim_string(line + 1, &formatted_line);
    if (err != 0) {
        return err;
    }
    tap_replace_string(formatted_line, '\n', ' ');

    /* Skip past the ':' */
    err = tap_cmd_strndup(ctype, formatted_line, strlen(formatted_line), &cmd);
    free(formatted_line);
    if (err != 0) {
        return err;
    }

    /* Overwrite TAP command to be the most broadly supported version */
    switch (ctype) {
        case tap_cmd_type_skip:
            memcpy(cmd->str, TAP_DIRECTIVE_SKIP,
                   sizeof(TAP_DIRECTIVE_SKIP) - 1);
            break;
        case tap_cmd_type_todo:
            memcpy(cmd->str, TAP_DIRECTIVE_TODO,
                   sizeof(TAP_DIRECTIVE_TODO) - 1);
            break;
        case tap_cmd_type_bail:
            memcpy(cmd->str, TAP_BAILOUT, sizeof(TAP_BAILOUT) - 1);
            break;
        default:
            break;
    }

    *d_cmd = cmd;
    return 0;
}

int tap_trim_string(const char *in, char **d_out) {
    size_t trim_len;
    char *out;

    /* Skip over preceding whitespace */
    for (; isspace(*in); in++)
        ;

    /* Skip over trailing whitespace */
    trim_len = strlen(in);
    for (; isspace(in[trim_len - 1]); trim_len--)
        ;

    out = strndup(in, trim_len);
    if (!out) {
        return errno;
    }
    *d_out = out;
    return 0;
}

void tap_replace_string(char *in, char c, char d) {
    char *pos = in;

    for (; (pos = strchr(pos, c)) != NULL;) {
        *pos = d;
    }
}
