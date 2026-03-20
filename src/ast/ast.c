#include "ast.h"

ASTNode* create_program_node(void) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->data.program.functions = NULL;
    node->data.program.function_count = 0;
    return node;
}

void add_function(ASTNode* program, ASTNode* function) {
    if (!program || program->type != AST_PROGRAM) return;
    
    program->data.program.function_count++;
    program->data.program.functions = safe_realloc(
        program->data.program.functions,
        program->data.program.function_count * sizeof(ASTNode*)
    );
    program->data.program.functions[program->data.program.function_count - 1] = function;
}

ASTNode* create_function_node(char* name, ASTNode* body) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_FUNCTION;
    node->data.function.name = str_dup(name);
    node->data.function.body = body;
    return node;
}

ASTNode* create_block_node(void) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_BLOCK;
    node->data.block.statements = NULL;
    node->data.block.statement_count = 0;
    return node;
}

void add_statement(ASTNode* block, ASTNode* stmt) {
    if (!block || block->type != AST_BLOCK) return;
    
    block->data.block.statement_count++;
    block->data.block.statements = safe_realloc(
        block->data.block.statements,
        block->data.block.statement_count * sizeof(ASTNode*)
    );
    block->data.block.statements[block->data.block.statement_count - 1] = stmt;
}

ASTNode* create_var_decl_node(char* name, ASTNode* init, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->line = line;
    node->column = col;
    node->data.var_decl.name = str_dup(name);
    node->data.var_decl.initializer = init;
    return node;
}
ASTNode* create_char_node(char value, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_CHAR;
    node->line = line;
    node->column = col;
    node->data.char_value = value;
    return node;
}

ASTNode* create_string_node(char* value, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_STRING;
    node->line = line;
    node->column = col;
    node->data.string_value = str_dup(value);
    return node;
}
ASTNode* create_assignment_node(char* name, ASTNode* value, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_ASSIGNMENT;
    node->line = line;
    node->column = col;
    node->data.assignment.name = str_dup(name);
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_return_node(ASTNode* value, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_RETURN;
    node->line = line;
    node->column = col;
    node->data.return_value = value;
    return node;
}

ASTNode* create_if_node(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_IF;
    node->line = line;
    node->column = col;
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* create_while_node(ASTNode* cond, ASTNode* body, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_WHILE;
    node->line = line;
    node->column = col;
    node->data.while_stmt.condition = cond;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* create_binary_op_node(TokenType op, ASTNode* left, ASTNode* right, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->line = line;
    node->column = col;
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

ASTNode* create_integer_node(int value, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_INTEGER;
    node->line = line;
    node->column = col;
    node->data.int_value = value;
    return node;
}

ASTNode* create_identifier_node(char* name, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->line = line;
    node->column = col;
    node->data.identifier = str_dup(name);
    return node;
}

ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, int line, int col) {
    ASTNode* node = safe_calloc(1, sizeof(ASTNode));
    node->type = AST_FUNCTION_CALL;
    node->line = line;
    node->column = col;
    node->data.func_call.name = str_dup(name);
    node->data.func_call.arguments = args;
    node->data.func_call.arg_count = arg_count;
    return node;
}

static void free_ast_node(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.function_count; i++) {
                free_ast(node->data.program.functions[i]);
            }
            safe_free((void**)&node->data.program.functions);
            break;
            
        case AST_FUNCTION:
            safe_free((void**)&node->data.function.name);
            free_ast(node->data.function.body);
            break;
            
        case AST_BLOCK:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                free_ast(node->data.block.statements[i]);
            }
            safe_free((void**)&node->data.block.statements);
            break;
            
        case AST_VAR_DECL:
            safe_free((void**)&node->data.var_decl.name);
            free_ast(node->data.var_decl.initializer);
            break;
            
        case AST_ASSIGNMENT:
            safe_free((void**)&node->data.assignment.name);
            free_ast(node->data.assignment.value);
            break;
            
        case AST_RETURN:
            free_ast(node->data.return_value);
            break;
            
        case AST_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_branch);
            free_ast(node->data.if_stmt.else_branch);
            break;
            
        case AST_WHILE:
            free_ast(node->data.while_stmt.condition);
            free_ast(node->data.while_stmt.body);
            break;
            
        case AST_BINARY_OP:
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right);
            break;
            
        case AST_IDENTIFIER:
            safe_free((void**)&node->data.identifier);
            break;
            
        case AST_FUNCTION_CALL:
            safe_free((void**)&node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++) {
                free_ast(node->data.func_call.arguments[i]);
            }
            safe_free((void**)&node->data.func_call.arguments);
            break;
          case AST_CHAR:
    // Nothing to free (char_value is not a pointer)
    {break;}
    
case AST_STRING:
    {safe_free((void**)&node->data.string_value);
    break;}  
        default:
            break;
    }
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast_node(node);
    safe_free((void**)&node);
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.program.function_count; i++) {
                print_ast(node->data.program.functions[i], indent + 1);
            }
            break;
            
        case AST_FUNCTION:
            printf("Function: %s\n", node->data.function.name);
            print_ast(node->data.function.body, indent + 1);
            break;
            
        case AST_BLOCK:
            printf("Block\n");
            for (int i = 0; i < node->data.block.statement_count; i++) {
                print_ast(node->data.block.statements[i], indent + 1);
            }
            break;
            
case AST_VAR_DECL:
    printf("VarDecl: %s", node->data.var_decl.name);
    if (node->data.var_decl.initializer) {
        printf(" = \n");
        print_ast(node->data.var_decl.initializer, indent + 1);
    } else {
        printf("\n");
    }
    break;
        case AST_ASSIGNMENT:
            printf("Assign: %s =\n", node->data.assignment.name);
            print_ast(node->data.assignment.value, indent + 1);
            break;
            
        case AST_RETURN:
            printf("Return\n");
            print_ast(node->data.return_value, indent + 1);
            break;
            
        case AST_IF:
            printf("If\n");
            print_ast(node->data.if_stmt.condition, indent + 1);
            print_ast(node->data.if_stmt.then_branch, indent + 1);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent);
                printf("Else\n");
                print_ast(node->data.if_stmt.else_branch, indent + 1);
            }
            break;
            
        case AST_WHILE:
            printf("While\n");
            print_ast(node->data.while_stmt.condition, indent + 1);
            print_ast(node->data.while_stmt.body, indent + 1);
            break;
            
        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", token_type_to_string(node->data.binary.op));
            print_ast(node->data.binary.left, indent + 1);
            print_ast(node->data.binary.right, indent + 1);
            break;
            
        case AST_INTEGER:
            printf("Integer: %d\n", node->data.int_value);
            break;
            
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier);
            break;
        case AST_CHAR:
    printf("Char: '%c'\n", node->data.char_value);
    break;
    
case AST_STRING:
    printf("String: \"%s\"\n", node->data.string_value);
    break;    
        case AST_FUNCTION_CALL:
            printf("Call: %s\n", node->data.func_call.name);
            for (int i = 0; i < node->data.func_call.arg_count; i++) {
                print_ast(node->data.func_call.arguments[i], indent + 1);
            }
            break;
    }
}
