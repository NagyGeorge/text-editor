#include <stdio.h>
#include <string.h>
#include "buffer.h"

void buffer_init(Buffer *buf) {
    memset(buf->lines, 0, sizeof(buf->lines));
    memset(buf->line_len, 0, sizeof(buf->line_len));
    buf->max_line_used = 1;
}

void buffer_save_file(Buffer *buf, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < buf->max_line_used; i++) {
        fwrite(buf->lines[i], 1, buf->line_len[i], f);
        fputc('\n', f);
    }
    fclose(f);
}

void buffer_load_file(Buffer *buf, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    
    buffer_init(buf);
    int row = 0;
    char line[MAX_COLS];
    
    while (fgets(line, sizeof(line), f) && row < MAX_LINES) {
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[--len] = '\0';
        }
        if (len > MAX_COLS - 1) len = MAX_COLS - 1;
        memcpy(buf->lines[row], line, len);
        buf->line_len[row] = len;
        row++;
    }
    
    buf->max_line_used = (row > 0) ? row : 1;
    fclose(f);
}

void buffer_insert_char(Buffer *buf, int row, int col, char c) {
    if (row < 0 || row >= MAX_LINES) return;
    if (col < 0 || col > buf->line_len[row]) return;
    if (buf->line_len[row] >= MAX_COLS - 1) return;
    
    memmove(&buf->lines[row][col + 1], &buf->lines[row][col], 
            buf->line_len[row] - col);
    buf->lines[row][col] = c;
    buf->line_len[row]++;
    
    if (row + 1 > buf->max_line_used) {
        buf->max_line_used = row + 1;
    }
}

void buffer_delete_char(Buffer *buf, int row, int col) {
    if (row < 0 || row >= MAX_LINES) return;
    if (col < 0 || col >= buf->line_len[row]) return;
    
    memmove(&buf->lines[row][col], &buf->lines[row][col + 1],
            buf->line_len[row] - col - 1);
    buf->line_len[row]--;
}

void buffer_new_line(Buffer *buf, int row, int col) {
    if (row < 0 || row >= MAX_LINES - 1) return;
    if (col < 0 || col > buf->line_len[row]) return;
    
    int rest = buf->line_len[row] - col;
    if (rest < 0) rest = 0;
    
    if (rest > 0) {
        memmove(buf->lines[row + 1], &buf->lines[row][col], rest);
        buf->line_len[row + 1] = rest;
    } else {
        buf->line_len[row + 1] = 0;
    }
    
    buf->line_len[row] = col;
    
    if (row + 2 > buf->max_line_used) {
        buf->max_line_used = row + 2;
    }
}

void buffer_delete_line(Buffer *buf, int row) {
    if (row < 0 || row >= buf->max_line_used) return;
    
    for (int i = row; i < buf->max_line_used - 1; i++) {
        memcpy(buf->lines[i], buf->lines[i + 1], buf->line_len[i + 1]);
        buf->line_len[i] = buf->line_len[i + 1];
    }
    
    if (buf->max_line_used > 1) {
        buf->line_len[buf->max_line_used - 1] = 0;
        buf->max_line_used--;
    }
}

int buffer_get_line_length(Buffer *buf, int row) {
    if (row < 0 || row >= MAX_LINES) return 0;
    return buf->line_len[row];
}

const char* buffer_get_line(Buffer *buf, int row) {
    if (row < 0 || row >= MAX_LINES) return "";
    return buf->lines[row];
}