#include "lexer.h"

static void advance(Lexer* lexer) {
    if (lexer->current_char == '\n') { lexer->line++; lexer->column = 1; }
    else lexer->column++;
    lexer->position++;
    lexer->current_char = lexer->source[lexer->position];
}

static char peek(Lexer* lexer) { return lexer->source[lexer->position + 1]; }

static void skip_whitespace(Lexer* lexer) {
    while (lexer->current_char == ' '  || lexer->current_char == '\t' ||
           lexer->current_char == '\n' || lexer->current_char == '\r')
        advance(lexer);
}

static void skip_comment(Lexer* lexer) {
    if (lexer->current_char == '/' && peek(lexer) == '/') {
        while (lexer->current_char != '\n' && lexer->current_char != '\0')
            advance(lexer);
    } else if (lexer->current_char == '/' && peek(lexer) == '*') {
        advance(lexer); advance(lexer);
        while (!(lexer->current_char == '*' && peek(lexer) == '/') &&
               lexer->current_char != '\0')
            advance(lexer);
        if (lexer->current_char == '*') { advance(lexer); advance(lexer); }
    }
}

Lexer* init_lexer(char* source) {
    Lexer* l        = safe_malloc(sizeof(Lexer));
    l->source       = source;
    l->position     = 0;
    l->line         = 1;
    l->column       = 1;
    l->current_char = source[0];
    return l;
}

void free_lexer(Lexer* lexer) { safe_free((void**)&lexer); }

static Token* create_token(Lexer* lexer, TokenType type, char* lexeme) {
    Token* t  = safe_malloc(sizeof(Token));
    t->type   = type;
    t->lexeme = str_dup(lexeme);
    t->line   = lexer->line;
    t->column = lexer->column - (int)strlen(lexeme);
    return t;
}

static Token* read_identifier(Lexer* lexer) {
    char buf[256]; int i = 0;
    while (isalnum(lexer->current_char) || lexer->current_char == '_')
        buf[i++] = lexer->current_char, advance(lexer);
    buf[i] = '\0';

    /* keywords */
    if (strcmp(buf, "func")   == 0) return create_token(lexer, TOKEN_FUNC,        buf);
    if (strcmp(buf, "var")    == 0) return create_token(lexer, TOKEN_VAR,         buf);
    if (strcmp(buf, "string") == 0) return create_token(lexer, TOKEN_STRING_TYPE, buf);
    if (strcmp(buf, "bool")   == 0) return create_token(lexer, TOKEN_BOOL,        buf);
    if (strcmp(buf, "return") == 0) return create_token(lexer, TOKEN_RETURN,      buf);
    if (strcmp(buf, "if")     == 0) return create_token(lexer, TOKEN_IF,          buf);
    if (strcmp(buf, "else")   == 0) return create_token(lexer, TOKEN_ELSE,        buf);
    if (strcmp(buf, "while")  == 0) return create_token(lexer, TOKEN_WHILE,       buf);
    if (strcmp(buf, "for")    == 0) return create_token(lexer, TOKEN_FOR,         buf);
    if (strcmp(buf, "list")   == 0) return create_token(lexer, TOKEN_LIST,        buf);
    /* boolean literals */
    if (strcmp(buf, "yes")    == 0) return create_token(lexer, TOKEN_YES,         buf);
    if (strcmp(buf, "no")     == 0) return create_token(lexer, TOKEN_NO,          buf);

    return create_token(lexer, TOKEN_IDENTIFIER, buf);
}

static Token* read_number(Lexer* lexer) {
    char buf[256]; int i = 0;
    while (isdigit(lexer->current_char)) buf[i++] = lexer->current_char, advance(lexer);
    buf[i] = '\0';
    return create_token(lexer, TOKEN_NUMBER, buf);
}

static Token* read_string_literal(Lexer* lexer, char closing) {
    char buf[1024]; int i = 0;
    advance(lexer);
    while (lexer->current_char != closing && lexer->current_char != '\0') {
        if (lexer->current_char == '\\') {
            advance(lexer);
            switch (lexer->current_char) {
                case 'n':  buf[i++] = '\n'; break;
                case 't':  buf[i++] = '\t'; break;
                case '\\': buf[i++] = '\\'; break;
                default:   buf[i++] = lexer->current_char; break;
            }
        } else buf[i++] = lexer->current_char;
        advance(lexer);
    }
    buf[i] = '\0';
    if (lexer->current_char == closing) advance(lexer);
    return create_token(lexer, TOKEN_STRING, buf);
}

Token* get_next_token(Lexer* lexer) {
    while (lexer->current_char != '\0') {

        if (isspace(lexer->current_char)) { skip_whitespace(lexer); continue; }

        if (lexer->current_char == '/' &&
            (peek(lexer) == '/' || peek(lexer) == '*')) {
            skip_comment(lexer); continue;
        }

        if (lexer->current_char == '"')  return read_string_literal(lexer, '"');
        if (lexer->current_char == '\'') return read_string_literal(lexer, '\'');

        if (isalpha(lexer->current_char) || lexer->current_char == '_')
            return read_identifier(lexer);

        if (isdigit(lexer->current_char)) return read_number(lexer);

        char c = lexer->current_char; advance(lexer);
        switch (c) {
            case '+': return create_token(lexer, TOKEN_PLUS,      "+");
            case '-': return create_token(lexer, TOKEN_MINUS,     "-");
            case '*': return create_token(lexer, TOKEN_STAR,      "*");
            case '/': return create_token(lexer, TOKEN_SLASH,     "/");
            case ';': return create_token(lexer, TOKEN_SEMICOLON, ";");
            case ',': return create_token(lexer, TOKEN_COMMA,     ",");
            case '(': return create_token(lexer, TOKEN_LPAREN,    "(");
            case ')': return create_token(lexer, TOKEN_RPAREN,    ")");
            case '{': return create_token(lexer, TOKEN_LBRACE,    "{");
            case '}': return create_token(lexer, TOKEN_RBRACE,    "}");
            case '[': return create_token(lexer, TOKEN_LBRACKET,  "[");
            case ']': return create_token(lexer, TOKEN_RBRACKET,  "]");
            case '=':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_EQUAL,         "=="); }
                return create_token(lexer, TOKEN_ASSIGN, "=");
            case '!':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_NOT_EQUAL,     "!="); }
                return create_token(lexer, TOKEN_NOT, "!");
            case '<':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_LESS_EQUAL,    "<="); }
                return create_token(lexer, TOKEN_LESS, "<");
            case '>':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_GREATER_EQUAL, ">="); }
                return create_token(lexer, TOKEN_GREATER, ">");
            case '&':
                if (peek(lexer) == '&') { advance(lexer); return create_token(lexer, TOKEN_AND, "&&"); }
                return create_token(lexer, TOKEN_UNKNOWN, "&");
            case '|':
                if (peek(lexer) == '|') { advance(lexer); return create_token(lexer, TOKEN_OR, "||"); }
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
        "EOF", "IDENTIFIER", "NUMBER", "CHAR", "STRING",
        "FUNC", "VAR", "STRING_TYPE", "BOOL", "RETURN",
        "IF", "ELSE", "WHILE", "FOR", "LIST",
        "YES", "NO",
        "PLUS", "MINUS", "STAR", "SLASH",
        "ASSIGN", "EQUAL", "NOT_EQUAL",
        "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL",
        "AND", "OR", "NOT",
        "SEMICOLON", "COMMA", "LPAREN", "RPAREN",
        "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
        "UNKNOWN", "ERROR"
    };
    return names[type];
}