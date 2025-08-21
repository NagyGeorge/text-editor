#ifndef DISPLAY_H
#define DISPLAY_H

#include "buffer.h"

typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND
} EditorMode;

typedef struct {
    int cols;
    int rows;
    int view_rows;
    char status[256];
} Display;

void display_init(Display *disp);
void display_refresh(Display *disp, Buffer *buf, int cx, int cy, EditorMode mode, const char *save_path, const char *cmd_buffer);
void display_set_status(Display *disp, const char *msg);
void display_update_size(Display *disp);
void display_move_cursor(int row, int col);

#endif