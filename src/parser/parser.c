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

static int expect(Parser* parser, TokenType type, const char* msg) {
    if (parser->current_token->type == type) { advance(parser); return 1; }
    error_at(parser->current_token->line, parser->current_token->column,
             "Expected %s, got %s: %s",
             token_type_to_string(type),
             token_type_to_string(parser->current_token->type), msg);
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
    Parser* p        = safe_malloc(sizeof(Parser));
    p->lexer         = lexer;
    p->error_count   = 0;
    p->current_token = get_next_token(lexer);
    p->peek_token    = get_next_token(lexer);
    return p;
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
───────────────────────────────────────── */

ASTNode* parse_program(Parser* parser) {
    printf("DEBUG: Starting parse_program\n");
    ASTNode* program = create_program_node();

    while (parser->current_token->type != TOKEN_EOF) {
        if (parser->current_token->type == TOKEN_FUNC) {
            advance(parser);

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

            expect(parser, TOKEN_LPAREN, "Expected '(' after function name");

            char* params[64]; int param_count = 0;
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
                if (parser->current_token->type == TOKEN_COMMA) advance(parser);
            }
            expect(parser, TOKEN_RPAREN, "Expected ')' after parameters");
            expect(parser, TOKEN_LBRACE, "Expected '{' before function body");

            ASTNode* body = create_block_node();
            while (parser->current_token->type != TOKEN_RBRACE &&
                   parser->current_token->type != TOKEN_EOF) {
                ASTNode* stmt = parse_statement(parser);
                if (stmt) add_statement(body, stmt);
            }
            expect(parser, TOKEN_RBRACE, "Expected '}' after function body");

            ASTNode* function = create_function_node(func_name, params, param_count, body);
            add_function(program, function);
            for (int i = 0; i < param_count; i++) free(params[i]);
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
    ASTNode* stmt = NULL;
    int line = parser->current_token->line;
    int col  = parser->current_token->column;

    switch (parser->current_token->type) {

        /* ── var x = expr ── */
        case TOKEN_VAR: {
            advance(parser);
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                char* name = str_dup(parser->current_token->lexeme);
                int vl = parser->current_token->line;
                int vc = parser->current_token->column;
                advance(parser);
                ASTNode* init = NULL;
                if (parser->current_token->type == TOKEN_ASSIGN) {
                    advance(parser); init = parse_expression(parser);
                }
                if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
                stmt = create_var_decl_node(name, init, vl, vc);
                safe_free((void**)&name);
            } else {
                error_at(parser->current_token->line, parser->current_token->column,
                         "Expected identifier after 'var'");
                parser->error_count++;
            }
            break;
        }

        /* ── string v = 'value' ── */
        case TOKEN_STRING_TYPE: {
            advance(parser);
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                char* name = str_dup(parser->current_token->lexeme);
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
                stmt = create_var_decl_node(name, init, vl, vc);
                safe_free((void**)&name);
            }
            break;
        }

        /* ── list nums = [1, 2, 3] ── */
        case TOKEN_LIST: {
            advance(parser);  /* consume 'list' */

            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                error_at(parser->current_token->line, parser->current_token->column,
                         "Expected variable name after 'list'");
                parser->error_count++;
                break;
            }

            char* name = str_dup(parser->current_token->lexeme);
            int vl = parser->current_token->line;
            int vc = parser->current_token->column;
            advance(parser);  /* consume variable name */

            ASTNode* init = NULL;

            if (parser->current_token->type == TOKEN_ASSIGN) {
                advance(parser);  /* consume = */

                expect(parser, TOKEN_LBRACKET,
                       "Expected '[' after '=' in list declaration");

                ASTNode** elems = NULL;
                int count = 0;

                while (parser->current_token->type != TOKEN_RBRACKET &&
                       parser->current_token->type != TOKEN_EOF) {
                    count++;
                    elems = safe_realloc(elems, count * sizeof(ASTNode*));
                    elems[count - 1] = parse_expression(parser);
                    if (parser->current_token->type == TOKEN_COMMA)
                        advance(parser);
                }

                expect(parser, TOKEN_RBRACKET, "Expected ']' to close list");
                init = create_list_literal_node(elems, count, vl, vc);
            }

            if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
            stmt = create_var_decl_node(name, init, vl, vc);
            safe_free((void**)&name);
            break;
        }

        /* ── return expr ── */
        case TOKEN_RETURN: {
            advance(parser);
            ASTNode* val = parse_expression(parser);
            if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
            stmt = create_return_node(val, line, col);
            break;
        }

        /* ── if condition stmt [else stmt] ── */
        case TOKEN_IF: {
            advance(parser);
            ASTNode* cond = parse_expression(parser);
            ASTNode* then = parse_statement(parser);
            ASTNode* els  = NULL;
            if (parser->current_token->type == TOKEN_ELSE) {
                advance(parser); els = parse_statement(parser);
            }
            stmt = create_if_node(cond, then, els, line, col);
            break;
        }

        /* ── while condition { body } ── */
        case TOKEN_WHILE: {
            advance(parser);
            ASTNode* cond = parse_expression(parser);
            ASTNode* body = parse_statement(parser);
            stmt = create_while_node(cond, body, line, col);
            break;
        }

        /* ── for (init; condition; update) { body } ── */
        case TOKEN_FOR: {
            advance(parser);
            expect(parser, TOKEN_LPAREN, "Expected '(' after 'for'");

            ASTNode* init = NULL;
            if (parser->current_token->type == TOKEN_VAR) {
                advance(parser);
                if (parser->current_token->type == TOKEN_IDENTIFIER) {
                    char* name = str_dup(parser->current_token->lexeme);
                    int vl = parser->current_token->line;
                    int vc = parser->current_token->column;
                    advance(parser);
                    ASTNode* iv = NULL;
                    if (parser->current_token->type == TOKEN_ASSIGN) {
                        advance(parser); iv = parse_expression(parser);
                    }
                    init = create_var_decl_node(name, iv, vl, vc);
                    safe_free((void**)&name);
                }
            } else if (parser->current_token->type == TOKEN_IDENTIFIER &&
                       parser->peek_token->type   == TOKEN_ASSIGN) {
                init = parse_assignment(parser);
            }
            expect(parser, TOKEN_SEMICOLON, "Expected ';' after for-init");

            ASTNode* cond = NULL;
            if (parser->current_token->type != TOKEN_SEMICOLON)
                cond = parse_expression(parser);
            expect(parser, TOKEN_SEMICOLON, "Expected ';' after for-condition");

            ASTNode* update = NULL;
            if (parser->current_token->type != TOKEN_RPAREN) {
                if (parser->current_token->type == TOKEN_IDENTIFIER &&
                    parser->peek_token->type   == TOKEN_ASSIGN) {
                    update = parse_assignment(parser);
                } else {
                    update = parse_expression(parser);
                }
            }
            expect(parser, TOKEN_RPAREN, "Expected ')' after for-update");

            ASTNode* body = parse_statement(parser);
            stmt = create_for_node(init, cond, update, body, line, col);
            break;
        }

        /* ── { stmts } ── */
        case TOKEN_LBRACE: {
            advance(parser);
            ASTNode* block = create_block_node();
            while (parser->current_token->type != TOKEN_RBRACE &&
                   parser->current_token->type != TOKEN_EOF) {
                ASTNode* s = parse_statement(parser);
                if (s) add_statement(block, s);
            }
            expect(parser, TOKEN_RBRACE, "Expected '}'");
            stmt = block;
            break;
        }

        /*
         * ── identifier statement ──
         *
         * Three cases based on what follows the identifier:
         *   x = 5          peek is TOKEN_ASSIGN     → simple assignment
         *   nums[1] = 99   peek is TOKEN_LBRACKET   → list index assignment
         *   output(x)      anything else             → expression
         */
        case TOKEN_IDENTIFIER: {
            if (parser->peek_token->type == TOKEN_ASSIGN) {
                /* simple variable assignment */
                stmt = parse_assignment(parser);

            } else if (parser->peek_token->type == TOKEN_LBRACKET) {
                /* list index assignment:  nums[1] = 99 */
                char* name = str_dup(parser->current_token->lexeme);
                int ln = parser->current_token->line;
                int cn = parser->current_token->column;
                advance(parser);   /* consume list name */

                advance(parser);   /* consume [ */
                ASTNode* idx = parse_expression(parser);
                expect(parser, TOKEN_RBRACKET, "Expected ']' after index");

                expect(parser, TOKEN_ASSIGN,
                       "Expected '=' after list index in assignment");

                ASTNode* val = parse_expression(parser);
                stmt = create_list_assign_node(name, idx, val, ln, cn);
                safe_free((void**)&name);

            } else {
                /* expression statement: function call, etc. */
                stmt = parse_expression(parser);
            }

            if (parser->current_token->type == TOKEN_SEMICOLON) advance(parser);
            break;
        }

        case TOKEN_SEMICOLON: advance(parser); break;

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
            ASTNode* val = parse_expression(parser);
            return create_assignment_node(name, val, line, col);
        }
        safe_free((void**)&name);
    }
    return NULL;
}

/* ─────────────────────────────────────────
   EXPRESSIONS
───────────────────────────────────────── */

static ASTNode* parse_expression(Parser* parser) { return parse_logical_or(parser); }

static ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* e = parse_logical_and(parser);
    while (parser->current_token->type == TOKEN_OR) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_logical_and(parser), l, c);
    }
    return e;
}

static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* e = parse_equality(parser);
    while (parser->current_token->type == TOKEN_AND) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_equality(parser), l, c);
    }
    return e;
}

static ASTNode* parse_equality(Parser* parser) {
    ASTNode* e = parse_comparison(parser);
    while (parser->current_token->type == TOKEN_EQUAL ||
           parser->current_token->type == TOKEN_NOT_EQUAL) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_comparison(parser), l, c);
    }
    return e;
}

static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* e = parse_term(parser);
    while (parser->current_token->type == TOKEN_LESS        ||
           parser->current_token->type == TOKEN_LESS_EQUAL  ||
           parser->current_token->type == TOKEN_GREATER     ||
           parser->current_token->type == TOKEN_GREATER_EQUAL) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_term(parser), l, c);
    }
    return e;
}

static ASTNode* parse_term(Parser* parser) {
    ASTNode* e = parse_factor(parser);
    while (parser->current_token->type == TOKEN_PLUS ||
           parser->current_token->type == TOKEN_MINUS) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_factor(parser), l, c);
    }
    return e;
}

static ASTNode* parse_factor(Parser* parser) {
    ASTNode* e = parse_primary(parser);
    while (parser->current_token->type == TOKEN_STAR ||
           parser->current_token->type == TOKEN_SLASH) {
        TokenType op = parser->current_token->type;
        int l = parser->current_token->line, c = parser->current_token->column;
        advance(parser);
        e = create_binary_op_node(op, e, parse_primary(parser), l, c);
    }
    return e;
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
            int v = atoi(parser->current_token->lexeme);
            advance(parser);
            expr = create_integer_node(v, line, col);
            break;
        }

        case TOKEN_CHAR: {
            char v = parser->current_token->lexeme[0];
            advance(parser);
            expr = create_char_node(v, line, col);
            break;
        }

        case TOKEN_STRING: {
            char* s = str_dup(parser->current_token->lexeme);
            advance(parser);
            expr = create_string_node(s, line, col);
            safe_free((void**)&s);
            break;
        }

        case TOKEN_IDENTIFIER: {
            char* name = str_dup(parser->current_token->lexeme);
            advance(parser);

            if (parser->current_token->type == TOKEN_LPAREN) {
                /* function call: name(args...) */
                advance(parser);
                ASTNode** args = NULL; int argc = 0;
                if (parser->current_token->type != TOKEN_RPAREN) {
                    args    = safe_malloc(sizeof(ASTNode*));
                    args[0] = parse_expression(parser);
                    argc    = 1;
                    while (parser->current_token->type == TOKEN_COMMA) {
                        advance(parser); argc++;
                        args = safe_realloc(args, argc * sizeof(ASTNode*));
                        args[argc-1] = parse_expression(parser);
                    }
                }
                expect(parser, TOKEN_RPAREN, "Expected ')' after arguments");
                expr = create_function_call_node(name, args, argc, line, col);

            } else if (parser->current_token->type == TOKEN_LBRACKET) {
                /* index read: nums[0] */
                ASTNode* obj = create_identifier_node(name, line, col);
                advance(parser);   /* consume [ */
                ASTNode* idx = parse_expression(parser);
                expect(parser, TOKEN_RBRACKET, "Expected ']' after index");
                expr = create_index_node(obj, idx, line, col);

            } else {
                expr = create_identifier_node(name, line, col);
            }

            safe_free((void**)&name);
            break;
        }

        /* bare list literal as expression:  [1, 2, 3] */
        case TOKEN_LBRACKET: {
            advance(parser);  /* consume [ */
            ASTNode** elems = NULL; int count = 0;
            while (parser->current_token->type != TOKEN_RBRACKET &&
                   parser->current_token->type != TOKEN_EOF) {
                count++;
                elems = safe_realloc(elems, count * sizeof(ASTNode*));
                elems[count-1] = parse_expression(parser);
                if (parser->current_token->type == TOKEN_COMMA) advance(parser);
            }
            expect(parser, TOKEN_RBRACKET, "Expected ']'");
            expr = create_list_literal_node(elems, count, line, col);
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
                     "Unexpected token in expression: %s",
                     token_type_to_string(parser->current_token->type));
            parser->error_count++;
            advance(parser);
            break;
    }
    return expr;
}