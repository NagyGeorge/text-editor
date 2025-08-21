#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>
#include "buffer.h"

struct EditorState {
    int *mode;
    int *cy;
    int *max_line_used;
    char (*lines)[MAX_COLS];
    int *line_len;
    const char *save_path;
    void (*save_file)(const char *path);
};

void command_reset(char *status_buf, size_t status_len);
const char *command_buffer(void);
int command_handle_char(unsigned char c, struct EditorState *ed);

#endif
