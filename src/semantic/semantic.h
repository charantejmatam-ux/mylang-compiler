#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"

// Semantic analyzer structure
typedef struct {
    SymbolTable* symbols;
    int error_count;
    int in_loop;
    DataType current_function_return_type;
    int has_return;
} SemanticAnalyzer;

// Function prototypes
SemanticAnalyzer* create_semantic_analyzer(void);
void free_semantic_analyzer(SemanticAnalyzer* analyzer);
void analyze_program(SemanticAnalyzer* analyzer, ASTNode* program);
int get_semantic_error_count(SemanticAnalyzer* analyzer);

#endif
