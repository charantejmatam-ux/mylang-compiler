#include "parser.h"

static void advance(Parser* parser) {
    free_token(parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token    = get_next_token(parser->lexer);
}

static int match(Parser* parser, TokenType type) {
    if (parser->current_token->type == type) { advance(parser); return 1; }
    return 0;
}

static int expect(Parser* parser, TokenType type, const char* error_msg) {
    if (parser->current_token->type == type) { advance(parser); return 1; }
    error_at(parser->current_token->line, parser->current_token->column,
             "Expected %s, got %s: %s",
             token_type_to_string(type),
             token_type_to_string(parser->current_token->type),
             error_msg);
    parser->error_count++;
    return 0;
}

static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_assignment(Parser* parser);
static ASTNode* parse_logical_or(Parser* parser);
static ASTNode* parse_logical_and(Parser* parser);
static ASTNode* parse_equality(Parser* parser);
static ASTNode* parse_comparison(Parser* parser);
static ASTNode* parse_term(Parser* parser);
static ASTNode* parse_factor(Parser* parser);
static ASTNode* parse_primary(Parser* parser);

Parser* init_parser(Lexer* lexer) {
    Parser* parser        = safe_malloc(sizeof(Parser));
    parser->lexer         = lexer;
    parser->error_count   = 0;
    parser->current_token = get_next_token(lexer);
    parser->peek_token    = get_next_token(lexer);
    return parser;
}

void free_parser(Parser* parser) {
    if (parser) {
        free_token(parser->current_token);
        free_token(parser->peek_token);
        safe_free((void**)&parser);
    }
}

int get_parser_error_count(Parser* parser) { return parser->error_count; }

/* ─────────────────────────────────────────
   PROGRAM
   Parses zero or more:  func name(p1, p2, ...) { body }
───────────────────────────────────────── */

ASTNode* parse_program(Parser* parser) {
    printf("DEBUG: Starting parse_program\n");
    ASTNode* program = create_program_node();

    while (parser->current_token->type != TOKEN_EOF) {

        if (parser->current_token->type == TOKEN_FUNC) {
            advance(parser); /* consume 'func' */

            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                error_at(parser->current_token->line,
                         parser->current_token->column,
                         "Expected function name after 'func'");
                parser->error_count++;
                advance(parser);
                continue;
            }

            char* func_name = str_dup(parser->current_token->lexeme);
            advance(parser);

            /* ── parse parameter list ── */
            expect(parser, TOKEN_LPAREN, "Expected '(' after function name");

            char*  params[64];
            int    param_count = 0;

            while (parser->current_token->type != TOKEN_RPAREN &&
                   parser->current_token->type != TOKEN_EOF) {

                if (parser->current_token->type == TOKEN_IDENTIFIER) {
                    params[param_count++] = str_dup(parser->current_token->lexeme);
                    advance(parser);
                } else {
                    error_at(parser->current_token->line,
                             parser->current_token->column,
                             "Expected parameter name");
                    parser->error_count++;
                    advance(parser);
                }

                /* consume comma between params */
                if (parser->current_token->type == TOKEN_COMMA)
                    advance(parser);
            }

            expect(parser, TOKEN_RPAREN, "Expected ')' after parameters");
            expect(parser, TOKEN_LBRACE, "Expected '{' before function body");

            /* ── parse body ── */
            ASTNode* body = create_block_node();
            while (parser->current_token->type != TOKEN_RBRACE &&
                   parser->current_token->type != TOKEN_EOF) {
                ASTNode* stmt = parse_statement(parser);
                if (stmt) add_statement(body, stmt);
            }
            expect(parser, TOKEN_RBRACE, "Expected '}' after function body");

            ASTNode* function = create_function_node(func_name, params,
                                                     param_count, body);
            add_function(program, function);

            /* free temporary param name copies */
            for (int i = 0; i < param_count; i++)
                free(params[i]);
            safe_free((void**)&func_name);

        } else {
            error_at(parser->current_token->line, parser->current_token->column,
                     "Unexpected token at top level: %s",
                     token_type_to_string(parser->current_token->type));
            parser->error_count++;
            advance(parser);
        }
    }

    return program;
}

/* ─────────────────────────────────────────
   STATEMENT
───────────────────────────────────────── */

static ASTNode* parse_statement(Parser* parser) {
    printf("DEBUG: parse_statement line %d token=%s (%s)\n",
           parser->current_token->line,
           token_type_to_string(parser->current_token->type),
           parser->current_token->lexeme);

    ASTNode* stmt = NULL;
    int line = parser->current_token->line;
    int col  = parser->current_token->column;

    switch (parser->current_token->type) {

        /* var x = expr */
        case TOKEN_VAR: {
            advance(parser);
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                char* var_name = str_dup(parser->current_token->lexeme);
                int vl = parser->current_token->line;
                int vc = parser->current_token->column;
                advance(parser);
                ASTNode* init = NULL;
                if (parser->current_token->type == TOKEN_ASSIGN) {
                    advance(parser);
                    init = parse_expression(parser);
                }
                if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
                stmt = create_var_decl_node(var_name, init, vl, vc);
                safe_free((void**)&var_name);
            } else {
                error_at(parser->current_token->line,
                         parser->current_token->column,
                         "Expected identifier after 'var'");
                parser->error_count++;
            }
            break;
        }

        /* string v = 'value' */
        case TOKEN_STRING_TYPE: {
            advance(parser);
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                char* var_name = str_dup(parser->current_token->lexeme);
                int vl = parser->current_token->line;
                int vc = parser->current_token->column;
                advance(parser);
                ASTNode* init = NULL;
                if (parser->current_token->type == TOKEN_ASSIGN) {
                    advance(parser);
                    if (parser->current_token->type == TOKEN_STRING) {
                        init = create_string_node(parser->current_token->lexeme, vl, vc);
                        advance(parser);
                    } else {
                        init = parse_expression(parser);
                    }
                }
                if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
                stmt = create_var_decl_node(var_name, init, vl, vc);
                safe_free((void**)&var_name);
            } else {
                error_at(parser->current_token->line,
                         parser->current_token->column,
                         "Expected identifier after 'string'");
                parser->error_count++;
            }
            break;
        }

        /* return expr */
        case TOKEN_RETURN: {
            advance(parser);
            ASTNode* value = parse_expression(parser);
            if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
            stmt = create_return_node(value, line, col);
            break;
        }

        /* if condition stmt [else stmt] */
        case TOKEN_IF: {
            advance(parser);
            ASTNode* condition   = parse_expression(parser);
            ASTNode* then_branch = parse_statement(parser);
            ASTNode* else_branch = NULL;
            if (parser->current_token->type == TOKEN_ELSE) {
                advance(parser);
                else_branch = parse_statement(parser);
            }
            stmt = create_if_node(condition, then_branch, else_branch, line, col);
            break;
        }

        /* while condition stmt */
        case TOKEN_WHILE: {
            advance(parser);
            ASTNode* condition = parse_expression(parser);
            ASTNode* body      = parse_statement(parser);
            stmt = create_while_node(condition, body, line, col);
            break;
        }

        /* { stmts } */
        case TOKEN_LBRACE: {
            advance(parser);
            ASTNode* block = create_block_node();
            while (parser->current_token->type != TOKEN_RBRACE &&
                   parser->current_token->type != TOKEN_EOF) {
                ASTNode* inner = parse_statement(parser);
                if (inner) add_statement(block, inner);
            }
            expect(parser, TOKEN_RBRACE, "Expected '}' after block");
            stmt = block;
            break;
        }

        /*
         * identifier — either  x = expr  (assignment)
         *                   or  foo(...)  (function call / expression)
         *
         * Peek at next token to decide without consuming the identifier.
         */
        case TOKEN_IDENTIFIER: {
            ASTNode* expr = NULL;
            if (parser->peek_token->type == TOKEN_ASSIGN) {
                expr = parse_assignment(parser);
            } else {
                expr = parse_expression(parser);
            }
            if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
            stmt = expr;
            break;
        }

        case TOKEN_SEMICOLON:
            advance(parser);
            break;

        default:
            error_at(parser->current_token->line, parser->current_token->column,
                     "Unexpected token in statement: %s",
                     token_type_to_string(parser->current_token->type));
            parser->error_count++;
            advance(parser);
            break;
    }

    return stmt;
}

/* ─────────────────────────────────────────
   ASSIGNMENT
───────────────────────────────────────── */

static ASTNode* parse_assignment(Parser* parser) {
    int line = parser->current_token->line;
    int col  = parser->current_token->column;

    if (parser->current_token->type == TOKEN_IDENTIFIER) {
        char* name = str_dup(parser->current_token->lexeme);
        advance(parser);
        if (match(parser, TOKEN_ASSIGN)) {
            ASTNode* value = parse_expression(parser);
            return create_assignment_node(name, value, line, col);
        }
        safe_free((void**)&name);
    }
    return NULL;
}

/* ─────────────────────────────────────────
   EXPRESSION (precedence climbing)
───────────────────────────────────────── */

static ASTNode* parse_expression(Parser* parser) { return parse_logical_or(parser); }

static ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* expr = parse_logical_and(parser);
    while (parser->current_token->type == TOKEN_OR) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_logical_and(parser), l, c);
    }
    return expr;
}

static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* expr = parse_equality(parser);
    while (parser->current_token->type == TOKEN_AND) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_equality(parser), l, c);
    }
    return expr;
}

static ASTNode* parse_equality(Parser* parser) {
    ASTNode* expr = parse_comparison(parser);
    while (parser->current_token->type == TOKEN_EQUAL ||
           parser->current_token->type == TOKEN_NOT_EQUAL) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_comparison(parser), l, c);
    }
    return expr;
}

static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* expr = parse_term(parser);
    while (parser->current_token->type == TOKEN_LESS         ||
           parser->current_token->type == TOKEN_LESS_EQUAL   ||
           parser->current_token->type == TOKEN_GREATER      ||
           parser->current_token->type == TOKEN_GREATER_EQUAL) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_term(parser), l, c);
    }
    return expr;
}

static ASTNode* parse_term(Parser* parser) {
    ASTNode* expr = parse_factor(parser);
    while (parser->current_token->type == TOKEN_PLUS ||
           parser->current_token->type == TOKEN_MINUS) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_factor(parser), l, c);
    }
    return expr;
}

static ASTNode* parse_factor(Parser* parser) {
    ASTNode* expr = parse_primary(parser);
    while (parser->current_token->type == TOKEN_STAR ||
           parser->current_token->type == TOKEN_SLASH) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        expr = create_binary_op_node(op, expr, parse_primary(parser), l, c);
    }
    return expr;
}

/* ─────────────────────────────────────────
   PRIMARY
───────────────────────────────────────── */

static ASTNode* parse_primary(Parser* parser) {
    ASTNode* expr = NULL;
    int line = parser->current_token->line;
    int col  = parser->current_token->column;

    switch (parser->current_token->type) {

        case TOKEN_NUMBER: {
            int value = atoi(parser->current_token->lexeme);
            advance(parser);
            expr = create_integer_node(value, line, col);
            break;
        }

        case TOKEN_CHAR: {
            char value = parser->current_token->lexeme[0];
            advance(parser);
            expr = create_char_node(value, line, col);
            break;
        }

        case TOKEN_STRING: {
            char* str = str_dup(parser->current_token->lexeme);
            advance(parser);
            expr = create_string_node(str, line, col);
            safe_free((void**)&str);
            break;
        }

        case TOKEN_IDENTIFIER: {
            char* name = str_dup(parser->current_token->lexeme);
            advance(parser);

            if (parser->current_token->type == TOKEN_LPAREN) {
                advance(parser); /* consume '(' */

                ASTNode** args = NULL;
                int arg_count  = 0;

                if (parser->current_token->type != TOKEN_RPAREN) {
                    args    = safe_malloc(sizeof(ASTNode*));
                    args[0] = parse_expression(parser);
                    arg_count = 1;

                    while (parser->current_token->type == TOKEN_COMMA) {
                        advance(parser);
                        arg_count++;
                        args = safe_realloc(args, arg_count * sizeof(ASTNode*));
                        args[arg_count - 1] = parse_expression(parser);
                    }
                }

                expect(parser, TOKEN_RPAREN, "Expected ')' after arguments");
                expr = create_function_call_node(name, args, arg_count, line, col);
            } else {
                expr = create_identifier_node(name, line, col);
            }

            safe_free((void**)&name);
            break;
        }

        case TOKEN_LPAREN: {
            advance(parser);
            expr = parse_expression(parser);
            expect(parser, TOKEN_RPAREN, "Expected ')' after expression");
            break;
        }

        default:
            error_at(parser->current_token->line, parser->current_token->column,
                     "Unexpected token in primary: %s",
                     token_type_to_string(parser->current_token->type));
            parser->error_count++;
            advance(parser);
            break;
    }

    return expr;
}