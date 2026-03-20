#ifndef CODEGEN_H
#define CODEGEN_H

#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"

typedef struct {
    FILE*        output;
    SymbolTable* symbols;
    int          label_counter;
    int          stack_offset;
    const char*  current_function;  /* name of function being compiled */
} CodeGenerator;

CodeGenerator* create_code_generator(const char* filename);
void           free_code_generator(CodeGenerator* generator);
void           generate_code(CodeGenerator* generator, ASTNode* program);

#endif