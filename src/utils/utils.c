#include "utils.h"
#include <stdarg.h>
#include <time.h>

static int debug_level = 0;

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    return ptr;
}
void* safe_realloc(void* ptr, size_t new_size) {
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size > 0) {
        fprintf(stderr, "Fatal: Out of memory in realloc\n");
        exit(1);
    }
    return new_ptr;
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

char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* new_str = safe_malloc(len + 1);
    strcpy(new_str, s);
    return new_str;
}

char* str_cat(const char* a, const char* b) {
    if (!a && !b) return NULL;
    if (!a) return str_dup(b);
    if (!b) return str_dup(a);
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    char* result = safe_malloc(len_a + len_b + 1);
    
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return 0;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = safe_malloc(size + 1);
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

int write_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) return 0;
    
    fprintf(file, "%s", content);
    fclose(file);
    return 1;
}

int file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void error_at(int line, int col, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "Error at line %d, column %d: ", line, col);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
    exit(1);
}

void warning_at(int line, int col, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "Warning at line %d, column %d: ", line, col);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

void debug_print(int level, const char* format, ...) {
    if (level > debug_level) return;
    
    va_list args;
    va_start(args, format);
    
    printf("[DEBUG] ");
    vprintf(format, args);
    printf("\n");
    
    va_end(args);
}

void set_debug_level(int level) {
    debug_level = level;
}
