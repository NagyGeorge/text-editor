#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "command.h"

static char cmd[64];
static int cmd_len;
static char *status_buf;
static size_t status_buf_len;

void command_reset(char *status_buf_in, size_t status_len_in) {
    status_buf = status_buf_in;
    status_buf_len = status_len_in;
    cmd_len = 0;
    cmd[0] = '\0';
}

const char *command_buffer(void) {
    return cmd;
}

int command_handle_char(unsigned char c, struct EditorState *ed) {
    if (c == 27) { // ESC
        *(ed->mode) = 0;
        if (status_buf)
            snprintf(status_buf, status_buf_len, "NORMAL mode");
    } else if (c == 127) { // backspace
        if (cmd_len > 0) cmd[--cmd_len] = '\0';
    } else if (c == '\r' || c == '\n') { // execute command
        cmd[cmd_len] = '\0';
        if (strcmp(cmd, "wq") == 0) {
            if (ed->save_file) ed->save_file(ed->save_path);
            return 1; // quit after saving
        } else if (strcmp(cmd, "q") == 0) {
            return 1; // quit
        } else if (strcmp(cmd, "dd") == 0) {
            int li = *(ed->cy) - 1;
            if (li < *(ed->max_line_used)) {
                for (int i = li; i < *(ed->max_line_used) - 1; i++) {
                    memcpy(ed->lines[i], ed->lines[i + 1], ed->line_len[i + 1]);
                    ed->line_len[i] = ed->line_len[i + 1];
                }
                if (*(ed->max_line_used) > 1) {
                    ed->line_len[*(ed->max_line_used) - 1] = 0;
                    (*(ed->max_line_used))--;
                    if (*(ed->cy) > *(ed->max_line_used))
                        *(ed->cy) = *(ed->max_line_used);
                }
                if (status_buf)
                    snprintf(status_buf, status_buf_len, "Deleted line");
            }
        } else {
            if (status_buf)
                snprintf(status_buf, status_buf_len, "Unknown command: %s", cmd);
        }
        *(ed->mode) = 0;
    } else if (isprint(c) && cmd_len < (int)sizeof(cmd) - 1) {
        cmd[cmd_len++] = c;
        cmd[cmd_len] = '\0';
    }
    return 0;
}
