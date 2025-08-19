// src/term.c
#include "term.h"
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

static struct termios orig;

int term_enter_raw(void) {
    if (!isatty(STDIN_FILENO)) return -1;
    if (tcgetattr(STDIN_FILENO, &orig) == -1) return -1;

    struct termios raw = orig;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN]  = 0;   // non-blocking reads with short timeout
    raw.c_cc[VTIME] = 1;   // 100 ms

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return -1;

    // Hide cursor, clear screen, move home
    write(STDOUT_FILENO, "\x1b[?25l\x1b[2J\x1b[H", 10);
    return 0;
}

void term_leave_raw_atexit(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    // Clear, move home, show cursor
    write(STDOUT_FILENO, "\x1b[2J\x1b[H\x1b[?25h", 12);
}

void term_query_size(int *cols, int *rows) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (cols) *cols = ws.ws_col;
    if (rows) *rows = ws.ws_row;
}

