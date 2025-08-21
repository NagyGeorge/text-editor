#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "editor.h"
#include "command.h"

void editor_init(Editor *ed, const char *filename) {
    buffer_init(&ed->buffer);
    display_init(&ed->display);
    cursor_init(&ed->cursor);
    ed->mode = MODE_NORMAL;
    ed->running = 1;
    
    strncpy(ed->save_path, filename ? filename : "out.txt", sizeof(ed->save_path) - 1);
    ed->save_path[sizeof(ed->save_path) - 1] = '\0';
    
    if (filename) {
        buffer_load_file(&ed->buffer, filename);
    }
}

void editor_handle_normal_input(Editor *ed, unsigned char c) {
    char status[256];
    
    switch (c) {
        case 17:
            editor_quit(ed);
            break;
        case 19:
            editor_save(ed);
            snprintf(status, sizeof(status), "Wrote %s", ed->save_path);
            display_set_status(&ed->display, status);
            break;
        case 'h':
            cursor_move_left(&ed->cursor);
            break;
        case 'l':
            cursor_move_right(&ed->cursor, 
                buffer_get_line_length(&ed->buffer, ed->cursor.y - 1) + 1);
            break;
        case 'k':
            cursor_move_up(&ed->cursor);
            break;
        case 'j':
            cursor_move_down(&ed->cursor, ed->display.view_rows);
            break;
        case 'I':
            cursor_set_column(&ed->cursor, 1);
            editor_set_mode(ed, MODE_INSERT);
            display_set_status(&ed->display, "INSERT — ESC to return");
            break;
        case ':':
            editor_set_mode(ed, MODE_COMMAND);
            command_reset(ed->display.status, sizeof(ed->display.status));
            break;
    }
}

void editor_handle_insert_input(Editor *ed, unsigned char c) {
    int row = ed->cursor.y - 1;
    int col = ed->cursor.x - 1;
    
    if (c == 27) {
        editor_set_mode(ed, MODE_NORMAL);
        display_set_status(&ed->display, "NORMAL mode");
    } else if (c == 127) {
        if (col > 0 && buffer_get_line_length(&ed->buffer, row) > 0) {
            buffer_delete_char(&ed->buffer, row, col - 1);
            cursor_move_left(&ed->cursor);
        }
    } else if (c == '\r' || c == '\n') {
        buffer_new_line(&ed->buffer, row, col);
        cursor_next_line(&ed->cursor, MAX_LINES);
    } else if (isprint(c)) {
        buffer_insert_char(&ed->buffer, row, col, c);
        ed->cursor.x++;
    }
}

void editor_process_input(Editor *ed, unsigned char c) {
    if (ed->mode == MODE_NORMAL) {
        editor_handle_normal_input(ed, c);
    } else if (ed->mode == MODE_INSERT) {
        editor_handle_insert_input(ed, c);
    } else if (ed->mode == MODE_COMMAND) {
        int mode_int = (int)ed->mode;
        
        static Editor *current_editor = NULL;
        current_editor = ed;
        
        void save_wrapper(const char *path) {
            if (current_editor) {
                buffer_save_file(&current_editor->buffer, path);
            }
        }
        
        struct EditorState cmd_state = {
            .mode = &mode_int,
            .cy = &ed->cursor.y,
            .max_line_used = &ed->buffer.max_line_used,
            .lines = ed->buffer.lines,
            .line_len = ed->buffer.line_len,
            .save_path = ed->save_path,
            .save_file = save_wrapper,
        };
        if (command_handle_char(c, &cmd_state)) {
            editor_quit(ed);
        }
        ed->mode = (EditorMode)mode_int;
    }
    
    int line_len = buffer_get_line_length(&ed->buffer, ed->cursor.y - 1);
    cursor_clamp(&ed->cursor, ed->display.view_rows, line_len);
}

void editor_save(Editor *ed) {
    buffer_save_file(&ed->buffer, ed->save_path);
}

void editor_quit(Editor *ed) {
    ed->running = 0;
}

void editor_set_mode(Editor *ed, EditorMode mode) {
    ed->mode = mode;
}