#include "cursor.h"

void cursor_init(Cursor *cur) {
    cur->x = 1;
    cur->y = 1;
}

void cursor_move_left(Cursor *cur) {
    if (cur->x > 1) cur->x--;
}

void cursor_move_right(Cursor *cur, int max_col) {
    if (cur->x <= max_col) cur->x++;
}

void cursor_move_up(Cursor *cur) {
    if (cur->y > 1) cur->y--;
}

void cursor_move_down(Cursor *cur, int max_row) {
    if (cur->y < max_row) cur->y++;
}

void cursor_clamp(Cursor *cur, int max_row, int line_length) {
    if (cur->y < 1) cur->y = 1;
    if (cur->y > max_row) cur->y = max_row;
    if (cur->x < 1) cur->x = 1;
    if (cur->x > line_length + 1) cur->x = line_length + 1;
}

void cursor_set_column(Cursor *cur, int col) {
    cur->x = col;
}

void cursor_next_line(Cursor *cur, int max_row) {
    if (cur->y < max_row) {
        cur->y++;
        cur->x = 1;
    }
}