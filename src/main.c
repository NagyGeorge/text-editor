// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "term.h"

static volatile sig_atomic_t g_resize = 0;
static void on_winch(int _) { (void)_; g_resize = 1; }

// 1-based terminal addressing helper
static void move_cursor(int row, int col) {
    // Clamp to sane values
    if (row < 1) row = 1;
    if (col < 1) col = 1;
    printf("\x1b[%d;%dH", row, col);
}

int main(void) {
    if (term_enter_raw() != 0) {
        perror("term_enter_raw");
        return 1;
    }
    atexit(term_leave_raw_atexit);
    signal(SIGWINCH, on_winch);

    int cols = 0, rows = 0;
    term_query_size(&cols, &rows);

    // Leave last row for a status bar
    int view_rows = rows - 1;
    if (view_rows < 1) view_rows = 1;

    // Cursor position (1-based for convenience with ANSI sequences)
    int cx = 1, cy = 1;

    for (;;) {
        // Full redraw
        printf("\x1b[2J\x1b[H"); // clear + home

        // Draw gutter like vim/kilo (tildes on empty lines)
        for (int r = 1; r <= view_rows; r++) {
            printf("~\r\n");
        }

        // Status bar (inverse video)
        printf("\x1b[7m"); // enter reverse video
        printf(" NORMAL  h j k l  |  Ctrl-Q quit  |  Pos %d,%d  Size %dx%d ", cy, cx, cols, rows);
        // Pad to full width
        int printed = 34; // rough prefix count; not exact, we’ll just clear line next
        (void)printed;
        printf("\x1b[0m"); // exit reverse video
        printf("\x1b[K");  // clear to end of line

        // Place cursor
        move_cursor(cy, cx);

        fflush(stdout);

        // Handle resize
        if (g_resize) {
            g_resize = 0;
            term_query_size(&cols, &rows);
            view_rows = rows - 1;
            if (view_rows < 1) view_rows = 1;
            if (cy > view_rows) cy = view_rows;
            if (cx > cols) cx = cols;
        }

        // Read one byte (raw mode has a short timeout)
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n != 1) continue;

        if (c == 17) {            // Ctrl-Q
            break;
        } else if (c == 'h') {    // left
            if (cx > 1) cx--;
        } else if (c == 'l') {    // right
            if (cx < cols) cx++;
        } else if (c == 'k') {    // up
            if (cy > 1) cy--;
        } else if (c == 'j') {    // down
            if (cy < view_rows) cy++;
        } else {
            // ignore other keys for now
        }
    }

    return 0;
}

