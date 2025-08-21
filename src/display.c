#include <stdio.h>
#include <string.h>
#include "display.h"
#include "term.h"

void display_init(Display *disp) {
    term_query_size(&disp->cols, &disp->rows);
    disp->view_rows = disp->rows - 1;
    if (disp->view_rows < 1) disp->view_rows = 1;
    strcpy(disp->status, "NORMAL mode — Ctrl-S save, Ctrl-Q quit");
}

void display_refresh(Display *disp, Buffer *buf, int cx, int cy, EditorMode mode, const char *save_path, const char *cmd_buffer) {
    printf("\x1b[2J\x1b[H");
    printf("\x1b[?25h");
    
    for (int r = 1; r <= disp->view_rows; r++) {
        int idx = r - 1;
        if (idx < MAX_LINES && buf->line_len[idx] > 0) {
            fwrite(buf->lines[idx], 1, buf->line_len[idx], stdout);
        } else {
            putchar('~');
        }
        printf("\x1b[K\r\n");
    }
    
    printf("\x1b[7m");
    printf(" %s | %s | ",
           mode == MODE_NORMAL ? "NORMAL" :
           mode == MODE_INSERT ? "INSERT" : "COMMAND",
           save_path);
    if (mode == MODE_COMMAND) {
        printf(":%s", cmd_buffer ? cmd_buffer : "");
    } else {
        printf("%s", disp->status);
    }
    printf(" | Pos %d,%d ", cy, cx);
    printf("\x1b[0m\x1b[K");
    
    display_move_cursor(cy, cx);
    fflush(stdout);
}

void display_set_status(Display *disp, const char *msg) {
    strncpy(disp->status, msg, sizeof(disp->status) - 1);
    disp->status[sizeof(disp->status) - 1] = '\0';
}

void display_update_size(Display *disp) {
    term_query_size(&disp->cols, &disp->rows);
    disp->view_rows = disp->rows - 1;
    if (disp->view_rows < 1) disp->view_rows = 1;
}

void display_move_cursor(int row, int col) {
    if (row < 1) row = 1;
    if (col < 1) col = 1;
    printf("\x1b[%d;%dH", row, col);
}