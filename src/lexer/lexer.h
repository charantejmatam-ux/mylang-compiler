#ifndef LEXER_H
#define LEXER_H

#include "../utils/utils.h"

// Token types
typedef enum {
    // End of file
    TOKEN_EOF,

    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_CHAR,
    TOKEN_STRING,

    // Keywords
    TOKEN_FUNC,
    TOKEN_VAR,
    TOKEN_STRING_TYPE,   /* 'string' keyword for typed var decl */
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,

    // Delimiters
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    // Special
    TOKEN_UNKNOWN,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

// Lexer structure
typedef struct {
    char* source;
    int position;
    int line;
    int column;
    char current_char;
} Lexer;

// Function prototypes
Lexer* init_lexer(char* source);
void free_lexer(Lexer* lexer);
Token* get_next_token(Lexer* lexer);
void free_token(Token* token);
const char* token_type_to_string(TokenType type);

#endif