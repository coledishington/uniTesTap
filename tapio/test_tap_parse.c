#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <tapio.h>
#include <taputil.h>

size_t test_counter = 1;

void positive_tests(void) {
    struct {
        const char *name;
        const char *input_line;
        const char *output_str;
        int output_type;
    } testcases[] = {
        {
            .name = "Skip uppercase cmd with no comment",
            .input_line = ":SKIP",
            .output_type = tap_cmd_type_skip,
            .output_str = "SKIP",
        },
        {
            .name = "Todo uppercase cmd with no comment",
            .input_line = ":TODO",
            .output_type = tap_cmd_type_todo,
            .output_str = "TODO",
        },
        {
            .name = "Bail uppercase cmd with no comment",
            .input_line = ":BAIL OUT!",
            .output_type = tap_cmd_type_bail,
            .output_str = "Bail out!",
        },
        {
            .name = "Skip lowercase cmd with no comment",
            .input_line = ":skip",
            .output_type = tap_cmd_type_skip,
            .output_str = "SKIP",
        },
        {
            .name = "Todo lowercase cmd with no comment",
            .input_line = ":todo",
            .output_type = tap_cmd_type_todo,
            .output_str = "TODO",
        },
        {
            .name = "Bail cmd lowercase with no comment",
            .input_line = ":bail out!",
            .output_type = tap_cmd_type_bail,
            .output_str = "Bail out!",
        },
        {
            .name = "Skip mixed-case cmd with no comment",
            .input_line = ":SkIp",
            .output_type = tap_cmd_type_skip,
            .output_str = "SKIP",
        },
        {
            .name = "Todo mixed-case cmd with no comment",
            .input_line = ":ToDo",
            .output_type = tap_cmd_type_todo,
            .output_str = "TODO",
        },
        {
            .name = "Bail cmd mixed-case with no comment",
            .input_line = ":bAiL OuT!",
            .output_type = tap_cmd_type_bail,
            .output_str = "Bail out!",
        },
        {
            .name = "Skip mixed case cmd with comment",
            .input_line = ":SkIp lEt's sKip dowN to the next tesT",
            .output_type = tap_cmd_type_skip,
            .output_str = "SKIP lEt's sKip dowN to the next tesT",
        },
        {
            .name = "TODO mixed cmd with comment",
            .input_line = ":ToDo loOkIng for soMEthing todo",
            .output_type = tap_cmd_type_todo,
            .output_str = "TODO loOkIng for soMEthing todo",
        },
        {
            .name = "Bail cmd uppercase with comment",
            .input_line = ":BAIL OUT! this is a hard PROblem",
            .output_type = tap_cmd_type_bail,
            .output_str = "Bail out! this is a hard PROblem",
        },
        {
            .name = "Skip mixed case cmd with comment split with newline",
            .input_line = ":SkIp lEt's sKip dowN\n to the next tesT",
            .output_type = tap_cmd_type_skip,
            .output_str = "SKIP lEt's sKip dowN  to the next tesT",
        },
    };

    for (size_t idx = 0; idx < ARRAY_LEN(testcases); idx++) {
        struct tap_cmd *cmd = NULL;
        size_t test_id = idx + test_counter;
        int err;

        err = tap_parse_cmd(testcases[idx].input_line, &cmd);
        if (err != 0) {
            printf("not ok %zu - %s failed due to retcode (%d != 0)\n", test_id,
                   testcases[idx].name, err);
            continue;
        }

        if (testcases[idx].output_type != cmd->type) {
            printf(
                "not ok %zu - %s failed to parse correct cmd type"
                " (%d != %d)\n",
                test_id, testcases[idx].name, testcases[idx].output_type,
                cmd->type);
            continue;
        }

        if (!cmd) {
            printf("not ok %zu - %s cmd is NULL\n", test_id,
                   testcases[idx].name);
            continue;
        }

        if (!cmd->str) {
            printf("not ok %zu - %s cmd->str is NULL\n", test_id,
                   testcases[idx].name);
            continue;
        }

        if (strcmp(testcases[idx].output_str, cmd->str) != 0) {
            printf(
                "not ok %zu - %s incorrectly translated command"
                " ('%s' != '%s')\n",
                test_id, testcases[idx].name, testcases[idx].output_str,
                cmd->str);
            continue;
        }

        printf("ok %zu - %s\n", test_id, testcases[idx].name);
    }

    test_counter += ARRAY_LEN(testcases);
}

void negative_tests(void) {
    struct {
        const char *name;
        const char *input_line;
    } testcases[] = {
        {
            .name = "Empty line",
            .input_line = "",
        },
        {
            .name = "Newline",
            .input_line = "\n",
        },
        {
            .name = "Debug text",
            .input_line = "suspect_function() returns 5",
        },
        {
            .name = "Command without colon",
            .input_line = "SKIP to my lou",
        },
        {
            .name = "Command with semi-colon",
            .input_line = ";SKIP to my lou",
        },
        {
            .name = "Command with double colon",
            .input_line = "::SKIP to my lou",
        },
        {
            .name = "Prefix of command",
            .input_line = ":SKI",
        },
        {
            .name = "Prefix of command with description",
            .input_line = ":SKI is a narrow strip of semi-rigid material worn "
                          "underfoot to glide over snow",
        },
    };

    for (size_t idx = 0; idx < ARRAY_LEN(testcases); idx++) {
        struct tap_cmd *cmd = NULL;
        size_t test_id = idx + test_counter;
        int err;

        err = tap_parse_cmd(testcases[idx].input_line, &cmd);
        if (err != 0) {
            printf("not ok %zu - %s failed due to retcode (%d != 0)\n", test_id,
                   testcases[idx].name, err);
            continue;
        }

        if (cmd) {
            printf("not ok %zu - %s parsed non-cmd as a valid command\n",
                   test_id, testcases[idx].name);
            continue;
        }

        printf("ok %zu - %s\n", test_id, testcases[idx].name);
    }

    test_counter += ARRAY_LEN(testcases);
}

int main(void) {
    positive_tests();
    negative_tests();
    printf("1..%zu\n", test_counter - 1);
}
