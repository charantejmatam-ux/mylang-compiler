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
    while (lexer->current_char == ' '  || lexer->current_char == '\t' ||
           lexer->current_char == '\n' || lexer->current_char == '\r') {
        advance(lexer);
    }
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
    Lexer* lexer       = safe_malloc(sizeof(Lexer));
    lexer->source      = source;
    lexer->position    = 0;
    lexer->line        = 1;
    lexer->column      = 1;
    lexer->current_char = source[0];
    return lexer;
}

void free_lexer(Lexer* lexer) {
    safe_free((void**)&lexer);
}

static Token* create_token(Lexer* lexer, TokenType type, char* lexeme) {
    Token* token  = safe_malloc(sizeof(Token));
    token->type   = type;
    token->lexeme = str_dup(lexeme);
    token->line   = lexer->line;
    token->column = lexer->column - (int)strlen(lexeme);
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

    /* Keywords */
    if (strcmp(buffer, "func")   == 0) return create_token(lexer, TOKEN_FUNC,        buffer);
    if (strcmp(buffer, "var")    == 0) return create_token(lexer, TOKEN_VAR,         buffer);
    if (strcmp(buffer, "string") == 0) return create_token(lexer, TOKEN_STRING_TYPE, buffer);
    if (strcmp(buffer, "return") == 0) return create_token(lexer, TOKEN_RETURN,      buffer);
    if (strcmp(buffer, "if")     == 0) return create_token(lexer, TOKEN_IF,          buffer);
    if (strcmp(buffer, "else")   == 0) return create_token(lexer, TOKEN_ELSE,        buffer);
    if (strcmp(buffer, "while")  == 0) return create_token(lexer, TOKEN_WHILE,       buffer);

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

/*
 * read_string — reads a string enclosed in EITHER double quotes or
 * single quotes.  Called when the opening quote has NOT yet been consumed.
 *
 *   "hello world"   → TOKEN_STRING  lexeme = hello world
 *   'charan'        → TOKEN_STRING  lexeme = charan
 *
 * Single-character literals like 'a' are also produced as TOKEN_STRING
 * (not TOKEN_CHAR) when they appear after a 'string' type keyword or in
 * an output() call.  TOKEN_CHAR is only produced for the old bare 'x'
 * form in other expression contexts.
 */
static Token* read_string_literal(Lexer* lexer, char closing_quote) {
    char buffer[1024];
    int i = 0;

    advance(lexer); /* skip opening quote */

    while (lexer->current_char != closing_quote && lexer->current_char != '\0') {
        /* basic escape: \n \t \\ */
        if (lexer->current_char == '\\') {
            advance(lexer);
            switch (lexer->current_char) {
                case 'n':  buffer[i++] = '\n'; break;
                case 't':  buffer[i++] = '\t'; break;
                case '\\': buffer[i++] = '\\'; break;
                case '\'': buffer[i++] = '\''; break;
                case '"':  buffer[i++] = '"';  break;
                default:   buffer[i++] = lexer->current_char; break;
            }
        } else {
            buffer[i++] = lexer->current_char;
        }
        advance(lexer);
    }
    buffer[i] = '\0';

    if (lexer->current_char == closing_quote)
        advance(lexer); /* skip closing quote */

    return create_token(lexer, TOKEN_STRING, buffer);
}

Token* get_next_token(Lexer* lexer) {
    while (lexer->current_char != '\0') {

        /* whitespace */
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }

        /* comments */
        if (lexer->current_char == '/' &&
            (peek(lexer) == '/' || peek(lexer) == '*')) {
            skip_comment(lexer);
            continue;
        }

        /* double-quoted string literal */
        if (lexer->current_char == '"')
            return read_string_literal(lexer, '"');

        /*
         * Single-quote: could be a string literal ('charan') or a single
         * char literal ('a').  We read it as TOKEN_STRING always — the
         * parser/codegen treat single-char TOKEN_STRING correctly.
         */
        if (lexer->current_char == '\'')
            return read_string_literal(lexer, '\'');

        /* identifiers / keywords */
        if (isalpha(lexer->current_char) || lexer->current_char == '_')
            return read_identifier(lexer);

        /* numbers */
        if (isdigit(lexer->current_char))
            return read_number(lexer);

        /* operators & delimiters */
        char c = lexer->current_char;
        advance(lexer);

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

            case '=':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_EQUAL,         "=="); }
                return create_token(lexer, TOKEN_ASSIGN, "=");

            case '!':
                if (lexer->current_char == '=') { advance(lexer); return create_token(lexer, TOKEN_NOT_EQUAL,     "!="); }
                return create_token(lexer, TOKEN_UNKNOWN, "!");

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
        "FUNC", "VAR", "STRING_TYPE", "RETURN", "IF", "ELSE", "WHILE",
        "PLUS", "MINUS", "STAR", "SLASH", "ASSIGN", "EQUAL", "NOT_EQUAL",
        "LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL", "AND", "OR",
        "SEMICOLON", "COMMA", "LPAREN", "RPAREN", "LBRACE", "RBRACE",
        "UNKNOWN", "ERROR"
    };
    return names[type];
}