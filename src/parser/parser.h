#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../ast/ast.h"

// Parser structure
typedef struct {
    Lexer* lexer;
    Token* current_token;
    Token* peek_token;
    int error_count;
} Parser;

// Function prototypes
Parser* init_parser(Lexer* lexer);
void free_parser(Parser* parser);
ASTNode* parse_program(Parser* parser);
int get_parser_error_count(Parser* parser);

#endif
