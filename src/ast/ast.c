#include "ast.h"

ASTNode* create_program_node(void) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_PROGRAM; return n;
}

void add_function(ASTNode* program, ASTNode* function) {
    if (!program || program->type != AST_PROGRAM) return;
    program->data.program.function_count++;
    program->data.program.functions = safe_realloc(
        program->data.program.functions,
        program->data.program.function_count * sizeof(ASTNode*));
    program->data.program.functions[program->data.program.function_count-1] = function;
}

ASTNode* create_function_node(char* name, char** params, int param_count, ASTNode* body) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_FUNCTION;
    n->data.function.name        = str_dup(name);
    n->data.function.body        = body;
    n->data.function.param_count = param_count;
    if (param_count > 0 && params) {
        n->data.function.params = safe_malloc(param_count * sizeof(char*));
        for (int i = 0; i < param_count; i++)
            n->data.function.params[i] = str_dup(params[i]);
    }
    return n;
}

ASTNode* create_block_node(void) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_BLOCK; return n;
}

void add_statement(ASTNode* block, ASTNode* stmt) {
    if (!block || block->type != AST_BLOCK) return;
    block->data.block.statement_count++;
    block->data.block.statements = safe_realloc(
        block->data.block.statements,
        block->data.block.statement_count * sizeof(ASTNode*));
    block->data.block.statements[block->data.block.statement_count-1] = stmt;
}

ASTNode* create_var_decl_node(char* name, ASTNode* init, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_VAR_DECL; n->line = line; n->column = col;
    n->data.var_decl.name        = str_dup(name);
    n->data.var_decl.initializer = init;
    return n;
}

ASTNode* create_assignment_node(char* name, ASTNode* value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_ASSIGNMENT; n->line = line; n->column = col;
    n->data.assignment.name  = str_dup(name);
    n->data.assignment.value = value;
    return n;
}

ASTNode* create_return_node(ASTNode* value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_RETURN; n->line = line; n->column = col;
    n->data.return_value = value; return n;
}

ASTNode* create_if_node(ASTNode* cond, ASTNode* then_branch,
                        ASTNode* else_branch, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_IF; n->line = line; n->column = col;
    n->data.if_stmt.condition   = cond;
    n->data.if_stmt.then_branch = then_branch;
    n->data.if_stmt.else_branch = else_branch;
    return n;
}

ASTNode* create_while_node(ASTNode* cond, ASTNode* body, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_WHILE; n->line = line; n->column = col;
    n->data.while_stmt.condition = cond;
    n->data.while_stmt.body      = body;
    return n;
}

ASTNode* create_for_node(ASTNode* init, ASTNode* cond,
                         ASTNode* update, ASTNode* body, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_FOR; n->line = line; n->column = col;
    n->data.for_stmt.init      = init;
    n->data.for_stmt.condition = cond;
    n->data.for_stmt.update    = update;
    n->data.for_stmt.body      = body;
    return n;
}

ASTNode* create_binary_op_node(TokenType op, ASTNode* left, ASTNode* right,
                               int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_BINARY_OP; n->line = line; n->column = col;
    n->data.binary.op    = op;
    n->data.binary.left  = left;
    n->data.binary.right = right;
    return n;
}

/* NEW: unary op  !expr */
ASTNode* create_unary_op_node(TokenType op, ASTNode* operand, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_UNARY_OP; n->line = line; n->column = col;
    n->data.unary.op      = op;
    n->data.unary.operand = operand;
    return n;
}

ASTNode* create_integer_node(int value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_INTEGER; n->line = line; n->column = col;
    n->data.int_value = value; return n;
}

/* NEW: bool node — stores 1 for yes, 0 for no */
ASTNode* create_bool_node(int value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_BOOL; n->line = line; n->column = col;
    n->data.bool_value = value;   /* 1 = yes, 0 = no */
    return n;
}

ASTNode* create_char_node(char value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_CHAR; n->line = line; n->column = col;
    n->data.char_value = value; return n;
}

ASTNode* create_string_node(char* value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_STRING; n->line = line; n->column = col;
    n->data.string_value = str_dup(value); return n;
}

ASTNode* create_identifier_node(char* name, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_IDENTIFIER; n->line = line; n->column = col;
    n->data.identifier = str_dup(name); return n;
}

ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count,
                                   int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_FUNCTION_CALL; n->line = line; n->column = col;
    n->data.func_call.name      = str_dup(name);
    n->data.func_call.arguments = args;
    n->data.func_call.arg_count = arg_count;
    return n;
}

ASTNode* create_list_literal_node(ASTNode** elems, int count, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_LIST_LITERAL; n->line = line; n->column = col;
    n->data.list_literal.elements = elems;
    n->data.list_literal.count    = count;
    return n;
}

ASTNode* create_index_node(ASTNode* object, ASTNode* index, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_INDEX; n->line = line; n->column = col;
    n->data.index_expr.object = object;
    n->data.index_expr.index  = index;
    return n;
}

ASTNode* create_list_assign_node(char* name, ASTNode* index,
                                 ASTNode* value, int line, int col) {
    ASTNode* n = safe_calloc(1, sizeof(ASTNode));
    n->type = AST_LIST_ASSIGN; n->line = line; n->column = col;
    n->data.list_assign.name  = str_dup(name);
    n->data.list_assign.index = index;
    n->data.list_assign.value = value;
    return n;
}

/* ── free ── */
static void free_ast_node(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++)
                free_ast(node->data.program.functions[i]);
            safe_free((void**)&node->data.program.functions); break;
        case AST_FUNCTION:
            safe_free((void**)&node->data.function.name);
            for (int i = 0; i < node->data.function.param_count; i++)
                safe_free((void**)&node->data.function.params[i]);
            safe_free((void**)&node->data.function.params);
            free_ast(node->data.function.body); break;
        case AST_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++)
                free_ast(node->data.block.statements[i]);
            safe_free((void**)&node->data.block.statements); break;
        case AST_VAR_DECL:
            safe_free((void**)&node->data.var_decl.name);
            free_ast(node->data.var_decl.initializer); break;
        case AST_ASSIGNMENT:
            safe_free((void**)&node->data.assignment.name);
            free_ast(node->data.assignment.value); break;
        case AST_RETURN:   free_ast(node->data.return_value); break;
        case AST_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_branch);
            free_ast(node->data.if_stmt.else_branch); break;
        case AST_WHILE:
            free_ast(node->data.while_stmt.condition);
            free_ast(node->data.while_stmt.body); break;
        case AST_FOR:
            free_ast(node->data.for_stmt.init);
            free_ast(node->data.for_stmt.condition);
            free_ast(node->data.for_stmt.update);
            free_ast(node->data.for_stmt.body); break;
        case AST_BINARY_OP:
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right); break;
        case AST_UNARY_OP:
            free_ast(node->data.unary.operand); break;
        case AST_IDENTIFIER:   safe_free((void**)&node->data.identifier);    break;
        case AST_STRING:       safe_free((void**)&node->data.string_value);  break;
        case AST_FUNCTION_CALL:
            safe_free((void**)&node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++)
                free_ast(node->data.func_call.arguments[i]);
            safe_free((void**)&node->data.func_call.arguments); break;
        case AST_LIST_LITERAL:
            for (int i = 0; i < node->data.list_literal.count; i++)
                free_ast(node->data.list_literal.elements[i]);
            safe_free((void**)&node->data.list_literal.elements); break;
        case AST_INDEX:
            free_ast(node->data.index_expr.object);
            free_ast(node->data.index_expr.index); break;
        case AST_LIST_ASSIGN:
            safe_free((void**)&node->data.list_assign.name);
            free_ast(node->data.list_assign.index);
            free_ast(node->data.list_assign.value); break;
        default: break;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast_node(node);
    safe_free((void**)&node);
}

/* ── print ── */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    print_indent(indent);
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.program.function_count; i++)
                print_ast(node->data.program.functions[i], indent+1); break;
        case AST_FUNCTION:
            printf("Function: %s (%d params)\n",
                   node->data.function.name, node->data.function.param_count);
            for (int i = 0; i < node->data.function.param_count; i++) {
                print_indent(indent+1);
                printf("Param: %s\n", node->data.function.params[i]);
            }
            print_ast(node->data.function.body, indent+1); break;
        case AST_BLOCK:
            printf("Block\n");
            for (int i = 0; i < node->data.block.statement_count; i++)
                print_ast(node->data.block.statements[i], indent+1); break;
        case AST_VAR_DECL:
            printf("VarDecl: %s\n", node->data.var_decl.name);
            if (node->data.var_decl.initializer)
                print_ast(node->data.var_decl.initializer, indent+1); break;
        case AST_ASSIGNMENT:
            printf("Assign: %s =\n", node->data.assignment.name);
            print_ast(node->data.assignment.value, indent+1); break;
        case AST_RETURN:
            printf("Return\n");
            print_ast(node->data.return_value, indent+1); break;
        case AST_IF:
            printf("If\n");
            print_ast(node->data.if_stmt.condition,  indent+1);
            print_ast(node->data.if_stmt.then_branch, indent+1);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent); printf("Else\n");
                print_ast(node->data.if_stmt.else_branch, indent+1);
            }
            break;
        case AST_WHILE:
            printf("While\n");
            print_ast(node->data.while_stmt.condition, indent+1);
            print_ast(node->data.while_stmt.body,      indent+1); break;
        case AST_FOR:
            printf("For\n");
            print_ast(node->data.for_stmt.init,      indent+1);
            print_ast(node->data.for_stmt.condition, indent+1);
            print_ast(node->data.for_stmt.update,    indent+1);
            print_ast(node->data.for_stmt.body,      indent+1); break;
        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", token_type_to_string(node->data.binary.op));
            print_ast(node->data.binary.left,  indent+1);
            print_ast(node->data.binary.right, indent+1); break;
        case AST_UNARY_OP:
            printf("UnaryOp: %s\n", token_type_to_string(node->data.unary.op));
            print_ast(node->data.unary.operand, indent+1); break;
        case AST_INTEGER:
            printf("Integer: %d\n", node->data.int_value); break;
        case AST_BOOL:
            printf("Bool: %s\n", node->data.bool_value ? "yes" : "no"); break;
        case AST_IDENTIFIER:
            printf("Ident: %s\n", node->data.identifier); break;
        case AST_CHAR:
            printf("Char: '%c'\n", node->data.char_value); break;
        case AST_STRING:
            printf("String: \"%s\"\n", node->data.string_value); break;
        case AST_FUNCTION_CALL:
            printf("Call: %s\n", node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++)
                print_ast(node->data.func_call.arguments[i], indent+1); break;
        case AST_LIST_LITERAL:
            printf("ListLiteral (%d)\n", node->data.list_literal.count);
            for (int i = 0; i < node->data.list_literal.count; i++)
                print_ast(node->data.list_literal.elements[i], indent+1); break;
        case AST_INDEX:
            printf("Index\n");
            print_ast(node->data.index_expr.object, indent+1);
            print_ast(node->data.index_expr.index,  indent+1); break;
        case AST_LIST_ASSIGN:
            printf("ListAssign: %s[]\n", node->data.list_assign.name);
            print_ast(node->data.list_assign.index, indent+1);
            print_ast(node->data.list_assign.value, indent+1); break;
        default: printf("(unknown %d)\n", node->type); break;
    }
}