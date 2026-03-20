#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>  // Add this for size_t

// Memory management
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void* safe_realloc(void* ptr, size_t new_size); 
void safe_free(void** ptr);

// String utilities
char* str_dup(const char* s);
char* str_cat(const char* a, const char* b);
int str_ends_with(const char* str, const char* suffix);

// File utilities
char* read_file(const char* filename);
int write_file(const char* filename, const char* content);
int file_exists(const char* filename);

// Error reporting
void error_at(int line, int col, const char* format, ...);
void warning_at(int line, int col, const char* format, ...);

// Debug printing
void debug_print(int level, const char* format, ...);
void set_debug_level(int level);

#endif
