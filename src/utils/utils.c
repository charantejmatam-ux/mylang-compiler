#include "utils.h"
#include <stdarg.h>

/* ── global error list ── */
CompileError g_errors[MAX_ERRORS];
int          g_error_count = 0;

static int debug_level = 0;

/* ─────────────────────────────────────────
   MEMORY
───────────────────────────────────────── */

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t new_size) {
    void* p = realloc(ptr, new_size);
    if (!p && new_size > 0) {
        fprintf(stderr, "Fatal: Out of memory in realloc\n");
        exit(1);
    }
    return p;
}

void* safe_calloc(size_t count, size_t size) {
    void* ptr = calloc(count, size);
    if (!ptr && count > 0 && size > 0) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    return ptr;
}

void safe_free(void** ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

/* ─────────────────────────────────────────
   STRINGS
───────────────────────────────────────── */

char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* ns = safe_malloc(len + 1);
    strcpy(ns, s);
    return ns;
}

char* str_cat(const char* a, const char* b) {
    if (!a && !b) return NULL;
    if (!a) return str_dup(b);
    if (!b) return str_dup(a);
    size_t la = strlen(a), lb = strlen(b);
    char* r = safe_malloc(la + lb + 1);
    strcpy(r, a);
    strcat(r, b);
    return r;
}

int str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t sl = strlen(str), xl = strlen(suffix);
    if (xl > sl) return 0;
    return strcmp(str + sl - xl, suffix) == 0;
}

/* ─────────────────────────────────────────
   FILES
───────────────────────────────────────── */

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = safe_malloc(size + 1);
    size_t n  = fread(buf, 1, size, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int write_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

int file_exists(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

/* ─────────────────────────────────────────
   ERROR REPORTING
   
   error_at() records the error in g_errors[] and returns.
   The caller (parser / semantic) keeps going so we collect
   ALL errors in one pass, not just the first one.

   Duplicate suppression: if the same line+col was already
   recorded, skip it — avoids flooding the user with the same
   error from different call sites.
───────────────────────────────────────── */

void error_at(int line, int col, const char* format, ...) {

    /* suppress duplicates */
    for (int i = 0; i < g_error_count; i++) {
        if (g_errors[i].line == line && g_errors[i].col == col)
            return;
    }

    if (g_error_count >= MAX_ERRORS) {
        fprintf(stderr, "Too many errors — stopping.\n");
        exit(1);
    }

    CompileError* e = &g_errors[g_error_count++];
    e->line = line;
    e->col  = col;

    va_list args;
    va_start(args, format);
    vsnprintf(e->message, sizeof(e->message), format, args);
    va_end(args);

    /* also print immediately so the user sees errors as they occur */
    fprintf(stderr, "Error at line %d, col %d: %s\n", line, col, e->message);
}

/* fatal_at — for truly unrecoverable situations (bad file, OOM, etc.) */
void fatal_at(int line, int col, const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Fatal error at line %d, col %d: ", line, col);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void warning_at(int line, int col, const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Warning at line %d, col %d: ", line, col);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

int get_error_count(void) { return g_error_count; }

void print_all_errors(void) {
    if (g_error_count == 0) return;
    fprintf(stderr, "\n── %d error(s) found ──\n", g_error_count);
    for (int i = 0; i < g_error_count; i++) {
        fprintf(stderr, "  [%d] Line %d, col %d: %s\n",
                i + 1,
                g_errors[i].line,
                g_errors[i].col,
                g_errors[i].message);
    }
}

void clear_errors(void) { g_error_count = 0; }

/* ─────────────────────────────────────────
   DEBUG
───────────────────────────────────────── */

void debug_print(int level, const char* format, ...) {
    if (level > debug_level) return;
    va_list args;
    va_start(args, format);
    printf("[DEBUG] ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void set_debug_level(int level) { debug_level = level; }