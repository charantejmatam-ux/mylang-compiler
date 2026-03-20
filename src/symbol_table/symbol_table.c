#include "symbol_table.h"

static unsigned int hash(char* name, int bucket_count) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *name++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % bucket_count;
}

SymbolTable* create_symbol_table(int bucket_count) {
    SymbolTable* table = safe_malloc(sizeof(SymbolTable));
    table->bucket_count = bucket_count;
    table->current_scope = 0;
    table->buckets = safe_calloc(bucket_count, sizeof(Symbol*));
    return table;
}

void free_symbol_table(SymbolTable* table) {
    if (!table) return;
    
    for (int i = 0; i < table->bucket_count; i++) {
        Symbol* sym = table->buckets[i];
        while (sym) {
            Symbol* next = sym->next;
            safe_free((void**)&sym->name);
            safe_free((void**)&sym->param_types);
            safe_free((void**)&sym);
            sym = next;
        }
    }
    
    safe_free((void**)&table->buckets);
    safe_free((void**)&table);
}

void enter_scope(SymbolTable* table) {
    table->current_scope++;
}

void exit_scope(SymbolTable* table) {
    // Remove all symbols in current scope
    for (int i = 0; i < table->bucket_count; i++) {
        Symbol** curr = &table->buckets[i];
        while (*curr) {
            if ((*curr)->scope_level == table->current_scope) {
                Symbol* to_remove = *curr;
                *curr = (*curr)->next;
                safe_free((void**)&to_remove->name);
                safe_free((void**)&to_remove->param_types);
                safe_free((void**)&to_remove);
            } else {
                curr = &(*curr)->next;
            }
        }
    }
    
    table->current_scope--;
}

int declare_symbol(SymbolTable* table, char* name, SymbolKind kind, 
                   DataType type, int line) {
    unsigned int index = hash(name, table->bucket_count);
    
    // Check if already declared in current scope
    Symbol* existing = table->buckets[index];
    while (existing) {
        if (strcmp(existing->name, name) == 0 && 
            existing->scope_level == table->current_scope) {
            return 0;  // Already declared in this scope
        }
        existing = existing->next;
    }
    
    // Create new symbol
    Symbol* sym = safe_malloc(sizeof(Symbol));
    sym->name = str_dup(name);
    sym->kind = kind;
    sym->type = type;
    sym->scope_level = table->current_scope;
    sym->line_declared = line;
    sym->param_count = 0;
    sym->param_types = NULL;
    sym->next = table->buckets[index];
    
    table->buckets[index] = sym;
    return 1;
}

Symbol* lookup_symbol(SymbolTable* table, char* name) {
    unsigned int index = hash(name, table->bucket_count);
    
    Symbol* sym = table->buckets[index];
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

Symbol* lookup_symbol_current_scope(SymbolTable* table, char* name) {
    unsigned int index = hash(name, table->bucket_count);
    
    Symbol* sym = table->buckets[index];
    while (sym) {
        if (strcmp(sym->name, name) == 0 && 
            sym->scope_level == table->current_scope) {
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

void print_symbol_table(SymbolTable* table) {
    printf("\n=== Symbol Table (Scope %d) ===\n", table->current_scope);
    
    for (int i = 0; i < table->bucket_count; i++) {
        Symbol* sym = table->buckets[i];
        while (sym) {
            printf("  %s (scope %d, line %d) - ", 
                   sym->name, sym->scope_level, sym->line_declared);
            
            if (sym->kind == SYM_VARIABLE) {
                printf("variable");
            } else {
                printf("function");
            }
            
            printf(" [type: ");
            switch (sym->type) {
                case TYPE_INT: printf("int"); break;
                case TYPE_VOID: printf("void"); break;
                default: printf("unknown");
            }
            printf("]\n");
            
            sym = sym->next;
        }
    }
}
