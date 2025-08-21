#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

#define MAX_LINES 1000
#define MAX_COLS 256

typedef struct {
    char lines[MAX_LINES][MAX_COLS];
    int line_len[MAX_LINES];
    int max_line_used;
} Buffer;

void buffer_init(Buffer *buf);
void buffer_save_file(Buffer *buf, const char *path);
void buffer_load_file(Buffer *buf, const char *path);
void buffer_insert_char(Buffer *buf, int row, int col, char c);
void buffer_delete_char(Buffer *buf, int row, int col);
void buffer_new_line(Buffer *buf, int row, int col);
void buffer_delete_line(Buffer *buf, int row);
int buffer_get_line_length(Buffer *buf, int row);
const char* buffer_get_line(Buffer *buf, int row);

#endif