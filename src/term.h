// src/term.h
#ifndef TERM_H
#define TERM_H

// Enter/leave raw terminal mode and query window size.
int  term_enter_raw(void);
void term_leave_raw_atexit(void);
void term_query_size(int *cols, int *rows);

#endif

