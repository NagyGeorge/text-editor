// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include "term.h"

static volatile sig_atomic_t g_resize = 0;
static void on_winch(int _) { (void)_; g_resize = 1; }

static void move_cursor(int row, int col) {
    if (row < 1) row = 1;
    if (col < 1) col = 1;
    printf("\x1b[%d;%dH", row, col);
}

enum Mode { MODE_NORMAL, MODE_INSERT, MODE_COMMAND };

#define MAX_LINES 1000
#define MAX_COLS  512
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
    char cmd[64] = "";
    int cmd_len = 0;

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
            printf(":%s", cmd);
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
                cmd_len = 0;
                cmd[0] = '\0';
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
            if (c == 27) { // ESC
                mode = MODE_NORMAL;
                snprintf(status, sizeof(status), "NORMAL mode");
            } else if (c == 127) { // backspace
                if (cmd_len > 0) cmd[--cmd_len] = '\0';
            } else if (c == '\r' || c == '\n') {
                cmd[cmd_len] = '\0';
                if (strcmp(cmd, "wq") == 0) {
                    save_file(save_path);
                    break;
                } else if (strcmp(cmd, "q") == 0) {
                    break;
                } else if (strcmp(cmd, "dd") == 0) {
                    int li = cy - 1;
                    if (li < max_line_used) {
                        for (int i = li; i < max_line_used - 1; i++) {
                            memcpy(lines[i], lines[i + 1], line_len[i + 1]);
                            line_len[i] = line_len[i + 1];
                        }
                        if (max_line_used > 1) {
                            line_len[max_line_used - 1] = 0;
                            max_line_used--;
                            if (cy > max_line_used) cy = max_line_used;
                        }
                        snprintf(status, sizeof(status), "Deleted line");
                    }
                } else {
                    snprintf(status, sizeof(status), "Unknown command: %s", cmd);
                }
                mode = MODE_NORMAL;
            } else if (isprint(c) && cmd_len < (int)sizeof(cmd) - 1) {
                cmd[cmd_len++] = c;
                cmd[cmd_len] = '\0';
            }
        }
    }
    return 0;
}

