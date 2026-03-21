#include "semantic.h"

static void analyze_node(SemanticAnalyzer* analyzer, ASTNode* node);
static DataType get_expression_type(SemanticAnalyzer* analyzer, ASTNode* expr);
static void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* stmt);

SemanticAnalyzer* create_semantic_analyzer(void) {
    SemanticAnalyzer* a = safe_malloc(sizeof(SemanticAnalyzer));
    a->symbols = create_symbol_table(211);
    a->error_count = 0;
    a->in_loop = 0;
    a->current_function_return_type = TYPE_VOID;
    a->has_return = 0;
    return a;
}

void free_semantic_analyzer(SemanticAnalyzer* a) {
    if (a) { free_symbol_table(a->symbols); safe_free((void**)&a); }
}

int get_semantic_error_count(SemanticAnalyzer* a) { return a->error_count; }

static void semantic_error(SemanticAnalyzer* a, ASTNode* node,
                           const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    fprintf(stderr, "Semantic error at line %d, col %d: ", node->line, node->column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    a->error_count++;
}

static DataType get_expression_type(SemanticAnalyzer* a, ASTNode* expr) {
    if (!expr) return TYPE_UNKNOWN;
    switch (expr->type) {
        case AST_INTEGER:  return TYPE_INT;
        case AST_CHAR:     return TYPE_CHAR;
        case AST_STRING:   return TYPE_STRING;

        case AST_IDENTIFIER: {
            Symbol* s = lookup_symbol(a->symbols, expr->data.identifier);
            if (!s) {
                semantic_error(a, expr, "Undefined variable '%s'",
                               expr->data.identifier);
                return TYPE_UNKNOWN;
            }
            return s->type;
        }

        case AST_BINARY_OP: {
            DataType l = get_expression_type(a, expr->data.binary.left);
            DataType r = get_expression_type(a, expr->data.binary.right);
            if (l == TYPE_UNKNOWN || r == TYPE_UNKNOWN) return TYPE_UNKNOWN;
            return TYPE_INT;
        }

        case AST_FUNCTION_CALL:
            for (int i = 0; i < expr->data.func_call.arg_count; i++)
                get_expression_type(a, expr->data.func_call.arguments[i]);
            return TYPE_INT;

        /* list literal → TYPE_LIST */
        case AST_LIST_LITERAL:
            for (int i = 0; i < expr->data.list_literal.count; i++)
                get_expression_type(a, expr->data.list_literal.elements[i]);
            return TYPE_LIST;

        /* index read nums[0] → TYPE_INT (elements are int) */
        case AST_INDEX: {
            Symbol* s = NULL;
            if (expr->data.index_expr.object->type == AST_IDENTIFIER) {
                s = lookup_symbol(a->symbols,
                                  expr->data.index_expr.object->data.identifier);
                if (!s) {
                    semantic_error(a, expr, "Undefined list '%s'",
                                   expr->data.index_expr.object->data.identifier);
                    return TYPE_UNKNOWN;
                }
            }
            get_expression_type(a, expr->data.index_expr.index);
            return TYPE_INT;
        }

        default: return TYPE_UNKNOWN;
    }
}

static void analyze_statement(SemanticAnalyzer* a, ASTNode* stmt) {
    if (!stmt) return;
    switch (stmt->type) {

        case AST_VAR_DECL: {
            DataType dtype = TYPE_INT;
            if (stmt->data.var_decl.initializer) {
                dtype = get_expression_type(a, stmt->data.var_decl.initializer);
                /* if init is a list literal, declare as TYPE_LIST */
                if (stmt->data.var_decl.initializer->type == AST_LIST_LITERAL)
                    dtype = TYPE_LIST;
            }
            if (!declare_symbol(a->symbols, stmt->data.var_decl.name,
                                SYM_VARIABLE, dtype, stmt->line))
                semantic_error(a, stmt, "Variable '%s' already declared",
                               stmt->data.var_decl.name);
            break;
        }

        case AST_FUNCTION_CALL: {
            Symbol* s = lookup_symbol(a->symbols, stmt->data.func_call.name);
            if (!s)
                printf("WARNING: Function '%s' not declared (allowed)\n",
                       stmt->data.func_call.name);
            for (int i = 0; i < stmt->data.func_call.arg_count; i++)
                get_expression_type(a, stmt->data.func_call.arguments[i]);
            break;
        }

        case AST_ASSIGNMENT: {
            Symbol* s = lookup_symbol(a->symbols, stmt->data.assignment.name);
            if (!s)
                semantic_error(a, stmt, "Variable '%s' not declared",
                               stmt->data.assignment.name);
            get_expression_type(a, stmt->data.assignment.value);
            break;
        }

        /* list index assignment: nums[1] = 99 */
        case AST_LIST_ASSIGN: {
            Symbol* s = lookup_symbol(a->symbols, stmt->data.list_assign.name);
            if (!s)
                semantic_error(a, stmt, "List '%s' not declared",
                               stmt->data.list_assign.name);
            get_expression_type(a, stmt->data.list_assign.index);
            get_expression_type(a, stmt->data.list_assign.value);
            break;
        }

        case AST_RETURN: {
            a->has_return = 1;
            if (stmt->data.return_value)
                get_expression_type(a, stmt->data.return_value);
            break;
        }

        case AST_IF: {
            get_expression_type(a, stmt->data.if_stmt.condition);
            analyze_statement(a, stmt->data.if_stmt.then_branch);
            if (stmt->data.if_stmt.else_branch)
                analyze_statement(a, stmt->data.if_stmt.else_branch);
            break;
        }

        case AST_WHILE: {
            get_expression_type(a, stmt->data.while_stmt.condition);
            a->in_loop++;
            analyze_statement(a, stmt->data.while_stmt.body);
            a->in_loop--;
            break;
        }

        case AST_FOR: {
            enter_scope(a->symbols);
            if (stmt->data.for_stmt.init)
                analyze_statement(a, stmt->data.for_stmt.init);
            if (stmt->data.for_stmt.condition)
                get_expression_type(a, stmt->data.for_stmt.condition);
            if (stmt->data.for_stmt.update)
                analyze_statement(a, stmt->data.for_stmt.update);
            a->in_loop++;
            analyze_statement(a, stmt->data.for_stmt.body);
            a->in_loop--;
            exit_scope(a->symbols);
            break;
        }

        case AST_BLOCK: {
            enter_scope(a->symbols);
            for (int i = 0; i < stmt->data.block.statement_count; i++)
                analyze_statement(a, stmt->data.block.statements[i]);
            exit_scope(a->symbols);
            break;
        }

        default: break;
    }
}

static void analyze_node(SemanticAnalyzer* a, ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++)
                analyze_node(a, node->data.program.functions[i]);
            break;
        case AST_FUNCTION:
            declare_symbol(a->symbols, node->data.function.name,
                           SYM_FUNCTION, TYPE_INT, node->line);
            enter_scope(a->symbols);
            for (int i = 0; i < node->data.function.param_count; i++)
                declare_symbol(a->symbols, node->data.function.params[i],
                               SYM_VARIABLE, TYPE_INT, node->line);
            if (node->data.function.body)
                analyze_node(a, node->data.function.body);
            exit_scope(a->symbols);
            break;
        default:
            analyze_statement(a, node);
            break;
    }
}

void analyze_program(SemanticAnalyzer* a, ASTNode* program) {
    printf("\n--- Starting Semantic Analysis ---\n");
    analyze_node(a, program);
    printf("--- Semantic Analysis Complete ---\n\n");
    if (a->error_count > 0)
        fprintf(stderr, "Semantic analysis found %d error(s)\n", a->error_count);
    else
        printf("Semantic analysis completed successfully\n");
}