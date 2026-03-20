#include "lexer.h"

static void advance(Lexer* lexer) {
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    lexer->position++;
    lexer->current_char = lexer->source[lexer->position];
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->position + 1];
}

static void skip_whitespace(Lexer* lexer) {
    while (lexer->current_char == ' ' || lexer->current_char == '\t' || 
           lexer->current_char == '\n' || lexer->current_char == '\r') {
        advance(lexer);
    }
}

static void skip_comment(Lexer* lexer) {
    // Single line comment
    if (lexer->current_char == '/' && peek(lexer) == '/') {
        while (lexer->current_char != '\n' && lexer->current_char != '\0') {
            advance(lexer);
        }
    }
    // Multi-line comment
    else if (lexer->current_char == '/' && peek(lexer) == '*') {
        advance(lexer); // Skip '/'
        advance(lexer); // Skip '*'
        
        while (!(lexer->current_char == '*' && peek(lexer) == '/') && 
                lexer->current_char != '\0') {
            advance(lexer);
        }
        
        if (lexer->current_char == '*') {
            advance(lexer); // Skip '*'
            advance(lexer); // Skip '/'
        }
    }
}

Lexer* init_lexer(char* source) {
    Lexer* lexer = safe_malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = source[0];
    return lexer;
}

void free_lexer(Lexer* lexer) {
    safe_free((void**)&lexer);
}

static Token* create_token(Lexer* lexer, TokenType type, char* lexeme) {
    Token* token = safe_malloc(sizeof(Token));
    token->type = type;
    token->lexeme = str_dup(lexeme);
    token->line = lexer->line;
    token->column = lexer->column - strlen(lexeme);
    return token;
}

static Token* read_identifier(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    while (isalnum(lexer->current_char) || lexer->current_char == '_') {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }
    buffer[i] = '\0';
    
    // Check for keywords - YOUR LANGUAGE KEYWORDS
    if (strcmp(buffer, "func") == 0)
        return create_token(lexer, TOKEN_FUNC, buffer);
    if (strcmp(buffer, "var") == 0)
        return create_token(lexer, TOKEN_VAR, buffer);
    if (strcmp(buffer, "return") == 0)
        return create_token(lexer, TOKEN_RETURN, buffer);
    if (strcmp(buffer, "if") == 0)
        return create_token(lexer, TOKEN_IF, buffer);
    if (strcmp(buffer, "else") == 0)
        return create_token(lexer, TOKEN_ELSE, buffer);
    if (strcmp(buffer, "while") == 0)
        return create_token(lexer, TOKEN_WHILE, buffer);
    
    return create_token(lexer, TOKEN_IDENTIFIER, buffer);
}

static Token* read_number(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    while (isdigit(lexer->current_char)) {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }
    buffer[i] = '\0';
    
    return create_token(lexer, TOKEN_NUMBER, buffer);
}
static Token* read_string(Lexer* lexer) {
    char buffer[1024];
    int i = 0;

    advance(lexer); // skip opening "

    while (lexer->current_char != '"' && lexer->current_char != '\0') {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    buffer[i] = '\0';

    if (lexer->current_char == '"')
        advance(lexer); // skip closing "

    return create_token(lexer, TOKEN_STRING, buffer);
}
Token* get_next_token(Lexer* lexer) {
 while (lexer->current_char != '\0') {
        // Skip whitespace
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }
        
        // Handle character literals - MOVE THIS HERE
        if (lexer->current_char == '\'') {
            advance(lexer); // skip opening '
            char c = lexer->current_char;
            advance(lexer);
            if (lexer->current_char == '\'') {
                advance(lexer); // skip closing '
                char buf[2] = {c, '\0'};
                return create_token(lexer, TOKEN_CHAR, buf);
            }
        }
        // String literal
if (lexer->current_char == '"') {
    return read_string(lexer);
}
        // Identifiers
        if (isalpha(lexer->current_char) || lexer->current_char == '_') {
            return read_identifier(lexer);
        }
        
        // Numbers
        if (isdigit(lexer->current_char)) {
            return read_number(lexer);
        }
        
        // Operators and delimiters
        char c = lexer->current_char;
        advance(lexer);
        
        switch (c) {
            case '+': return create_token(lexer, TOKEN_PLUS, "+");
            case '-': return create_token(lexer, TOKEN_MINUS, "-");
            case '*': return create_token(lexer, TOKEN_STAR, "*");
            case '/': return create_token(lexer, TOKEN_SLASH, "/");
            case ';': return create_token(lexer, TOKEN_SEMICOLON, ";");
            case ',': return create_token(lexer, TOKEN_COMMA, ",");
            case '(': return create_token(lexer, TOKEN_LPAREN, "(");  // CORRECT
            case ')': return create_token(lexer, TOKEN_RPAREN, ")");  // CORRECT
            case '{': return create_token(lexer, TOKEN_LBRACE, "{");  // CORRECT
            case '}': return create_token(lexer, TOKEN_RBRACE, "}");  // CORRECT
            
            case '=':
                if (lexer->current_char == '=') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_EQUAL, "==");
                }
                return create_token(lexer, TOKEN_ASSIGN, "=");
                
            case '!':
                if (lexer->current_char == '=') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_NOT_EQUAL, "!=");
                }
                return create_token(lexer, TOKEN_UNKNOWN, "!");
                
            case '<':
                if (lexer->current_char == '=') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_LESS_EQUAL, "<=");
                }
                return create_token(lexer, TOKEN_LESS, "<");
                
            case '>':
                if (lexer->current_char == '=') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_GREATER_EQUAL, ">=");
                }
                return create_token(lexer, TOKEN_GREATER, ">");
                
            case '&':
                if (peek(lexer) == '&') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_AND, "&&");
                }
                return create_token(lexer, TOKEN_UNKNOWN, "&");
                
            case '|':
                if (peek(lexer) == '|') {
                    advance(lexer);
                    return create_token(lexer, TOKEN_OR, "||");
                }
                return create_token(lexer, TOKEN_UNKNOWN, "|");
                
            default:
                return create_token(lexer, TOKEN_UNKNOWN, (char[]){c, '\0'});
        }
    }
    
    return create_token(lexer, TOKEN_EOF, "EOF");
}

void free_token(Token* token) {
    if (token) {
        safe_free((void**)&token->lexeme);
        safe_free((void**)&token);
    }
}

const char* token_type_to_string(TokenType type) {
    static const char* names[] = {
       "EOF", "IDENTIFIER", "NUMBER",
        "CHAR", "STRING", 
        "FUNC", "VAR", "RETURN", "IF", "ELSE", "WHILE",
        "PLUS", "MINUS", "STAR", "SLASH", "ASSIGN", "EQUAL", "NOT_EQUAL",
        "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL", "AND", "OR",
        "SEMICOLON", "COMMA", "LPAREN", "RPAREN", "LBRACE", "RBRACE",
        "UNKNOWN", "ERROR"
    };
    return names[type];
}
