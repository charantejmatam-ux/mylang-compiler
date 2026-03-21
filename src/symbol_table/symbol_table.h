#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../utils/utils.h"

typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION
} SymbolKind;

typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_LIST,       /* list variable */
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

typedef struct Symbol {
    char*      name;
    SymbolKind kind;
    DataType   type;
    int        scope_level;
    int        line_declared;
    int        param_count;
    DataType*  param_types;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol** buckets;
    int      bucket_count;
    int      current_scope;
} SymbolTable;

SymbolTable* create_symbol_table(int bucket_count);
void         free_symbol_table(SymbolTable* table);
void         enter_scope(SymbolTable* table);
void         exit_scope(SymbolTable* table);
int          declare_symbol(SymbolTable* table, char* name, SymbolKind kind,
                            DataType type, int line);
Symbol*      lookup_symbol(SymbolTable* table, char* name);
Symbol*      lookup_symbol_current_scope(SymbolTable* table, char* name);
void         print_symbol_table(SymbolTable* table);

#endif