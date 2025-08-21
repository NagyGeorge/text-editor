#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "display.h"
#include "cursor.h"

typedef struct {
    Buffer buffer;
    Display display;
    Cursor cursor;
    EditorMode mode;
    char save_path[256];
    int running;
} Editor;

void editor_init(Editor *ed, const char *filename);
void editor_handle_normal_input(Editor *ed, unsigned char c);
void editor_handle_insert_input(Editor *ed, unsigned char c);
void editor_process_input(Editor *ed, unsigned char c);
void editor_save(Editor *ed);
void editor_quit(Editor *ed);
void editor_set_mode(Editor *ed, EditorMode mode);

#endif