#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <tap.h>
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
    const char *newline;
    size_t cmd_len;
    int err;

    ctype = line_to_cmd_type(line);
    if (ctype == tap_cmd_type_unknown) {
        return 0;
    }

    /* Copy everything before the first newline */
    newline = strchr(line, '\n');
    if (newline) {
        cmd_len = newline - line;
    } else {
        cmd_len = strlen(line);
    }

    /* Skip past the ':' */
    err = tap_cmd_strndup(ctype, line + 1, cmd_len - 1, &cmd);
    if (err != 0) {
        return err;
    }

    /* Overwrite TAP command to be the most broadly supported version */
    switch (ctype)
    {
    case tap_cmd_type_skip:
        memcpy(cmd->str, TAP_DIRECTIVE_SKIP, sizeof(TAP_DIRECTIVE_SKIP) - 1);
        break;
    case tap_cmd_type_todo:
        memcpy(cmd->str, TAP_DIRECTIVE_TODO, sizeof(TAP_DIRECTIVE_TODO) - 1);
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