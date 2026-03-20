#include "semantic.h"

static void analyze_node(SemanticAnalyzer* analyzer, ASTNode* node);
static DataType get_expression_type(SemanticAnalyzer* analyzer, ASTNode* expr);
static void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* stmt);

SemanticAnalyzer* create_semantic_analyzer(void) {
    SemanticAnalyzer* analyzer = safe_malloc(sizeof(SemanticAnalyzer));
    analyzer->symbols = create_symbol_table(211);
    analyzer->error_count = 0;
    analyzer->in_loop = 0;
    analyzer->current_function_return_type = TYPE_VOID;
    analyzer->has_return = 0;
    return analyzer;
}

void free_semantic_analyzer(SemanticAnalyzer* analyzer) {
    if (analyzer) {
        if (analyzer->symbols) {
            free_symbol_table(analyzer->symbols);
        }
        safe_free((void**)&analyzer);
    }
}

int get_semantic_error_count(SemanticAnalyzer* analyzer) {
    return analyzer->error_count;
}

static void semantic_error(SemanticAnalyzer* analyzer, ASTNode* node, 
                          const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "Semantic error at line %d, column %d: ", 
            node->line, node->column);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
    analyzer->error_count++;
}

static DataType get_expression_type(SemanticAnalyzer* analyzer, ASTNode* expr) {
    if (!expr) return TYPE_UNKNOWN;
    
    switch (expr->type) {
        case AST_INTEGER:
            return TYPE_INT;
            
        case AST_IDENTIFIER: {
            Symbol* sym = lookup_symbol(analyzer->symbols, expr->data.identifier);
            if (!sym) {
                semantic_error(analyzer, expr, "Undefined variable '%s'", 
                             expr->data.identifier);
                return TYPE_UNKNOWN;
            }
            return sym->type;
        }
        
        case AST_BINARY_OP: {
            DataType left = get_expression_type(analyzer, expr->data.binary.left);
            DataType right = get_expression_type(analyzer, expr->data.binary.right);
            
            if (left == TYPE_UNKNOWN || right == TYPE_UNKNOWN) {
                return TYPE_UNKNOWN;
            }
            
            // All operations return int for now
            return TYPE_INT;
        }
        case AST_CHAR:
    return TYPE_CHAR;
    
case AST_STRING:
    return TYPE_STRING;
        
        default:
            return TYPE_UNKNOWN;
    }
}

static void analyze_statement(SemanticAnalyzer* analyzer, ASTNode* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case AST_VAR_DECL: {
            printf("DEBUG: Declaring variable %s at scope level %d\n", 
                   stmt->data.var_decl.name, analyzer->symbols->current_scope);
            
            if (!declare_symbol(analyzer->symbols, stmt->data.var_decl.name,
                               SYM_VARIABLE, TYPE_INT, stmt->line)) {
                semantic_error(analyzer, stmt, "Variable '%s' already declared in this scope",
                             stmt->data.var_decl.name);
            }
            
            if (stmt->data.var_decl.initializer) {
                DataType init_type = get_expression_type(analyzer, 
                                                         stmt->data.var_decl.initializer);
                if (init_type != TYPE_INT) {
                    semantic_error(analyzer, stmt, "Variable initializer must be int");
                }
            }
            break;
        }
        
        case AST_ASSIGNMENT: {
            Symbol* sym = lookup_symbol(analyzer->symbols, stmt->data.assignment.name);
            if (!sym) {
                semantic_error(analyzer, stmt, "Variable '%s' not declared",
                             stmt->data.assignment.name);
            } else {
                DataType value_type = get_expression_type(analyzer, 
                                                          stmt->data.assignment.value);
                if (value_type != TYPE_INT) {
                    semantic_error(analyzer, stmt, "Assignment value must be int");
                }
            }
            break;
        }
        
        case AST_RETURN: {
            analyzer->has_return = 1;
            
            if (stmt->data.return_value) {
                DataType return_type = get_expression_type(analyzer, 
                                                           stmt->data.return_value);
                if (return_type != TYPE_INT) {
                    semantic_error(analyzer, stmt, "Return value must be int");
                }
            }
            break;
        }
        
        case AST_IF: {
            DataType cond_type = get_expression_type(analyzer, 
                                                     stmt->data.if_stmt.condition);
            if (cond_type != TYPE_INT) {
                semantic_error(analyzer, stmt, "If condition must be int");
            }
            
            analyze_statement(analyzer, stmt->data.if_stmt.then_branch);
            if (stmt->data.if_stmt.else_branch) {
                analyze_statement(analyzer, stmt->data.if_stmt.else_branch);
            }
            break;
        }
        
        case AST_WHILE: {
            DataType cond_type = get_expression_type(analyzer, 
                                                     stmt->data.while_stmt.condition);
            if (cond_type != TYPE_INT) {
                semantic_error(analyzer, stmt, "While condition must be int");
            }
            
            analyzer->in_loop++;
            analyze_statement(analyzer, stmt->data.while_stmt.body);
            analyzer->in_loop--;
            break;
        }
        
        case AST_BLOCK: {
            printf("DEBUG: Entering block scope\n");
            enter_scope(analyzer->symbols);
            
            for (int i = 0; i < stmt->data.block.statement_count; i++) {
                analyze_statement(analyzer, stmt->data.block.statements[i]);
            }
            
            printf("DEBUG: Exiting block scope\n");
            exit_scope(analyzer->symbols);
            break;
        }
        
        default:
            break;
    }
}

static void analyze_node(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++) {
                analyze_node(analyzer, node->data.program.functions[i]);
            }
            break;
            
        case AST_FUNCTION: {
            printf("DEBUG: Declaring function %s\n", node->data.function.name);
            declare_symbol(analyzer->symbols, node->data.function.name,
                          SYM_FUNCTION, TYPE_INT, node->line);
            
            printf("DEBUG: Entering function scope for %s (current scope: %d)\n", 
                   node->data.function.name, analyzer->symbols->current_scope);
            enter_scope(analyzer->symbols);
            
            if (node->data.function.body) {
                analyze_node(analyzer, node->data.function.body);
            }
            
            printf("DEBUG: Exiting function scope (current scope: %d)\n", 
                   analyzer->symbols->current_scope);
            exit_scope(analyzer->symbols);
            break;
        }
        
        default:
            analyze_statement(analyzer, node);
            break;
    }
}

void analyze_program(SemanticAnalyzer* analyzer, ASTNode* program) {
    printf("\n--- Starting Semantic Analysis ---\n");
    analyze_node(analyzer, program);
    printf("--- Semantic Analysis Complete ---\n\n");
    
    if (analyzer->error_count > 0) {
        fprintf(stderr, "\nSemantic analysis found %d error(s)\n", 
                analyzer->error_count);
    } else {
        printf("Semantic analysis completed successfully\n");
    }
}
