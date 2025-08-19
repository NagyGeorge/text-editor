#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include "term.h"

#define BUF_MAX 1024

static volatile sig_atomic_t g_resize = 0;
static void on_winch(int _) { (void)_; g_resize = 1; }

int main(void) {
    if (term_enter_raw() != 0) {
        perror("term_enter_raw");
        return 1;
    }
    atexit(term_leave_raw_atexit);
    signal(SIGWINCH, on_winch);

    char buffer[BUF_MAX];
    size_t buflen = 0;

    int cols = 0, rows = 0;
    term_query_size(&cols, &rows);

    while (1) {
        // clear and redraw
        printf("\x1b[2J\x1b[H"); // clear screen, move cursor home
        printf("Simple buffer (Ctrl-S save, Ctrl-Q quit):\n\n");
        fwrite(buffer, 1, buflen, stdout);
        fflush(stdout);

        // handle resize
        if (g_resize) {
            g_resize = 0;
            term_query_size(&cols, &rows);
        }

        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1) {
            if (c == 17) {         // Ctrl-Q
                break;
            } else if (c == 19) {  // Ctrl-S
                FILE *f = fopen("out.txt", "w");
                if (f) {
                    fwrite(buffer, 1, buflen, f);
                    fclose(f);
                }
            } else if (c == 127) { // Backspace
                if (buflen > 0) buflen--;
            } else if (isprint(c)) {
                if (buflen < BUF_MAX) {
                    buffer[buflen++] = c;
                }
            }
        }
    }
    return 0;
}

