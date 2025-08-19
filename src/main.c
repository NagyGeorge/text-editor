// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include "term.h"
#include "command.h"

static volatile sig_atomic_t g_resize = 0;
static void on_winch(int _) { (void)_; g_resize = 1; }

static void move_cursor(int row, int col) {
    if (row < 1) row = 1;
    if (col < 1) col = 1;
    printf("\x1b[%d;%dH", row, col);
}

static char lines[MAX_LINES][MAX_COLS];
static int  line_len[MAX_LINES];
static int  max_line_used = 1;

static void save_file(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < max_line_used; i++) {
        fwrite(lines[i], 1, line_len[i], f);
        fputc('\n', f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    const char *save_path = (argc > 1 ? argv[1] : "out.txt");

    if (term_enter_raw() != 0) {
        perror("term_enter_raw");
        return 1;
    }
    atexit(term_leave_raw_atexit);
    signal(SIGWINCH, on_winch);

    int cols = 0, rows = 0;
    term_query_size(&cols, &rows);
    int view_rows = rows - 1;
    if (view_rows < 1) view_rows = 1;

    int cx = 1, cy = 1;
    enum Mode mode = MODE_NORMAL;
    char status[256] = "NORMAL mode — Ctrl-S save, Ctrl-Q quit";

    struct EditorState ed = {
        .mode = &mode,
        .cy = &cy,
        .max_line_used = &max_line_used,
        .lines = lines,
        .line_len = line_len,
        .save_path = save_path,
        .save_file = save_file,
    };

    for (;;) {
        // full redraw
        printf("\x1b[2J\x1b[H");      // clear + home
        printf("\x1b[?25h");          // ensure cursor is visible

        for (int r = 1; r <= view_rows; r++) {
            int idx = r - 1;
            if (idx < MAX_LINES && line_len[idx] > 0) {
                fwrite(lines[idx], 1, line_len[idx], stdout);
            } else {
                putchar('~');
            }
            printf("\x1b[K\r\n");     // clear to end-of-line and newline
        }

        // status bar
        printf("\x1b[7m");
        printf(" %s | %s | ",
               mode == MODE_NORMAL ? "NORMAL" :
               mode == MODE_INSERT ? "INSERT" : "COMMAND",
               save_path);
        if (mode == MODE_COMMAND) {
            printf(":%s", command_buffer());
        } else {
            printf("%s", status);
        }
        printf(" | Pos %d,%d ", cy, cx);
        printf("\x1b[0m\x1b[K");

        // clamp cursor to screen and line
        if (cy < 1) cy = 1;
        if (cy > view_rows) cy = view_rows;
        int ll = line_len[cy - 1];
        if (cx < 1) cx = 1;
        if (cx > ll + 1) cx = ll + 1;

        move_cursor(cy, cx);
        fflush(stdout);

        if (g_resize) {
            g_resize = 0;
            term_query_size(&cols, &rows);
            view_rows = rows - 1;
            if (view_rows < 1) view_rows = 1;
            if (cy > view_rows) cy = view_rows;
            snprintf(status, sizeof(status), "Resized to %dx%d", cols, rows);
        }

        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n != 1) continue;

        if (mode == MODE_NORMAL) {
            if (c == 17) { // Ctrl-Q
                break;
            } else if (c == 19) { // Ctrl-S
                save_file(save_path);
                snprintf(status, sizeof(status), "Wrote %s", save_path);
            } else if (c == 'h') {
                if (cx > 1) cx--;
            } else if (c == 'l') {
                if (cx < cols && cx <= line_len[cy - 1] + 1) cx++;
            } else if (c == 'k') {
                if (cy > 1) cy--;
            } else if (c == 'j') {
                if (cy < view_rows) cy++;
            } else if (c == 'I') { // insert at column 1
                cx = 1;
                mode = MODE_INSERT;
                snprintf(status, sizeof(status), "INSERT — ESC to return");
            } else if (c == ':') { // command mode
                mode = MODE_COMMAND;
                command_reset(status, sizeof(status));
            }
        } else if (mode == MODE_INSERT) {
            if (c == 27) { // ESC
                mode = MODE_NORMAL;
                snprintf(status, sizeof(status), "NORMAL mode");
            } else if (c == 127) { // backspace
                int li = cy - 1;
                if (cx > 1 && line_len[li] > 0) {
                    memmove(&lines[li][cx - 2], &lines[li][cx - 1],
                            line_len[li] - (cx - 1));
                    line_len[li]--;
                    cx--;
                }
            } else if (c == '\r' || c == '\n') { // new line
                if (cy < MAX_LINES) {
                    int li = cy - 1;
                    int rest = line_len[li] - (cx - 1);
                    if (rest < 0) rest = 0;
                    // move tail to next line
                    if (rest > 0 && cy < MAX_LINES) {
                        memmove(lines[li + 1], &lines[li][cx - 1], rest);
                        line_len[li + 1] = rest;
                    } else {
                        line_len[li + 1] = 0;
                    }
                    line_len[li] = cx - 1;
                    cy++;
                    cx = 1;
                    if (cy > max_line_used) max_line_used = cy;
                }
            } else if (isprint(c)) { // insert character
                int li = cy - 1;
                if (line_len[li] < MAX_COLS - 1) {
                    memmove(&lines[li][cx], &lines[li][cx - 1],
                            line_len[li] - (cx - 1));
                    lines[li][cx - 1] = c;
                    line_len[li]++;
                    cx++;
                    if (cy > max_line_used) max_line_used = cy;
                }
            }
        } else if (mode == MODE_COMMAND) {
            if (command_handle_char(c, &ed)) {
                break;
            }
        }
    }
    return 0;
}

