#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>

/* ── memory ── */
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void* safe_realloc(void* ptr, size_t new_size);
void  safe_free(void** ptr);

/* ── strings ── */
char* str_dup(const char* s);
char* str_cat(const char* a, const char* b);
int   str_ends_with(const char* str, const char* suffix);

/* ── files ── */
char* read_file(const char* filename);
int   write_file(const char* filename, const char* content);
int   file_exists(const char* filename);

/* ── error reporting ──────────────────────────────────────────────────────
 *
 * error_at()   — records a compile error (line/col + message).
 *                Does NOT exit — compilation continues so we collect all
 *                errors and show them together.
 *
 * fatal_at()   — unrecoverable error (e.g. out of memory). Exits immediately.
 *
 * warning_at() — non-fatal diagnostic.
 *
 * get_error_count() / print_all_errors() / clear_errors() — used by main()
 * to decide whether to proceed to the next phase and to display the list.
 * ──────────────────────────────────────────────────────────────────────── */

#define MAX_ERRORS 64

typedef struct {
    int  line;
    int  col;
    char message[512];
} CompileError;

/* global error list (filled by error_at) */
extern CompileError g_errors[];
extern int          g_error_count;

void error_at(int line, int col, const char* format, ...);
void fatal_at(int line, int col, const char* format, ...);  /* exits */
void warning_at(int line, int col, const char* format, ...);

int  get_error_count(void);
void print_all_errors(void);
void clear_errors(void);

/* ── debug ── */
void debug_print(int level, const char* format, ...);
void set_debug_level(int level);

#endif