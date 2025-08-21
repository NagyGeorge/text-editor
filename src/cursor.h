#ifndef CURSOR_H
#define CURSOR_H

#include "buffer.h"

typedef struct {
    int x;
    int y;
} Cursor;

void cursor_init(Cursor *cur);
void cursor_move_left(Cursor *cur);
void cursor_move_right(Cursor *cur, int max_col);
void cursor_move_up(Cursor *cur);
void cursor_move_down(Cursor *cur, int max_row);
void cursor_clamp(Cursor *cur, int max_row, int line_length);
void cursor_set_column(Cursor *cur, int col);
void cursor_next_line(Cursor *cur, int max_row);

#endif