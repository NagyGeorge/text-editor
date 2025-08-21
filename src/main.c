#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "term.h"
#include "editor.h"
#include "command.h"

static volatile sig_atomic_t g_resize = 0;
static void on_winch(int _) { (void)_; g_resize = 1; }

int main(int argc, char **argv) {
    const char *filename = (argc > 1 ? argv[1] : "out.txt");
    
    if (term_enter_raw() != 0) {
        perror("term_enter_raw");
        return 1;
    }
    atexit(term_leave_raw_atexit);
    signal(SIGWINCH, on_winch);
    
    Editor ed;
    editor_init(&ed, filename);
    
    while (ed.running) {
        display_refresh(&ed.display, &ed.buffer, ed.cursor.x, ed.cursor.y, 
                       ed.mode, ed.save_path, command_buffer());
        
        if (g_resize) {
            g_resize = 0;
            display_update_size(&ed.display);
            char status[256];
            snprintf(status, sizeof(status), "Resized to %dx%d", 
                    ed.display.cols, ed.display.rows);
            display_set_status(&ed.display, status);
            
            if (ed.cursor.y > ed.display.view_rows) {
                ed.cursor.y = ed.display.view_rows;
            }
        }
        
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n != 1) continue;
        
        editor_process_input(&ed, c);
    }
    
    return 0;
}