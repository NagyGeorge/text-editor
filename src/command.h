#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>

#define MAX_LINES 1000
#define MAX_COLS 512

enum Mode { MODE_NORMAL, MODE_INSERT, MODE_COMMAND };

struct EditorState {
    enum Mode *mode;
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

#endif // COMMAND_H
