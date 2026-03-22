#ifndef AST_H
#define AST_H

#include "../utils/utils.h"
#include "../lexer/lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BINARY_OP,
    AST_UNARY_OP,    /* !expr, -expr          */
    AST_INTEGER,
    AST_BOOL,        /* yes=1  no=0           */
    AST_CHAR,
    AST_STRING,
    AST_IDENTIFIER,
    AST_FUNCTION_CALL,
    /* list support */
    AST_LIST_LITERAL,
    AST_INDEX,
    AST_LIST_ASSIGN
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct { TokenType op; ASTNode* left; ASTNode* right; } BinaryOp;
typedef struct { TokenType op; ASTNode* operand; }              UnaryOp;
typedef struct { char* name; ASTNode** arguments; int arg_count; } FunctionCall;
typedef struct { ASTNode* condition; ASTNode* then_branch; ASTNode* else_branch; } IfStmt;
typedef struct { ASTNode* condition; ASTNode* body; } WhileStmt;
typedef struct { ASTNode* init; ASTNode* condition; ASTNode* update; ASTNode* body; } ForStmt;
typedef struct { char* name; ASTNode* initializer; } VarDecl;
typedef struct { char* name; ASTNode* value; }        Assignment;
typedef struct { char* name; ASTNode* body; char** params; int param_count; } Function;
typedef struct { ASTNode** statements; int statement_count; } Block;
typedef struct { ASTNode** elements; int count; } ListLiteral;
typedef struct { ASTNode* object; ASTNode* index; } IndexExpr;
typedef struct { char* name; ASTNode* index; ASTNode* value; } ListAssign;

struct ASTNode {
    ASTNodeType type;
    int line, column;
    union {
        struct { ASTNode** functions; int function_count; } program;
        Function     function;
        Block        block;
        VarDecl      var_decl;
        Assignment   assignment;
        ASTNode*     return_value;
        IfStmt       if_stmt;
        WhileStmt    while_stmt;
        ForStmt      for_stmt;
        BinaryOp     binary;
        UnaryOp      unary;
        int          int_value;
        int          bool_value;   /* 1 = yes/true, 0 = no/false */
        char*        identifier;
        FunctionCall func_call;
        char         char_value;
        char*        string_value;
        ListLiteral  list_literal;
        IndexExpr    index_expr;
        ListAssign   list_assign;
    } data;
};

/* existing creators */
ASTNode* create_program_node(void);
void     add_function(ASTNode* program, ASTNode* function);
ASTNode* create_function_node(char* name, char** params, int param_count, ASTNode* body);
ASTNode* create_block_node(void);
void     add_statement(ASTNode* block, ASTNode* stmt);
ASTNode* create_var_decl_node(char* name, ASTNode* init, int line, int col);
ASTNode* create_assignment_node(char* name, ASTNode* value, int line, int col);
ASTNode* create_return_node(ASTNode* value, int line, int col);
ASTNode* create_if_node(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line, int col);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body, int line, int col);
ASTNode* create_for_node(ASTNode* init, ASTNode* cond, ASTNode* update, ASTNode* body, int line, int col);
ASTNode* create_binary_op_node(TokenType op, ASTNode* left, ASTNode* right, int line, int col);
ASTNode* create_unary_op_node(TokenType op, ASTNode* operand, int line, int col);  /* NEW */
ASTNode* create_integer_node(int value, int line, int col);
ASTNode* create_bool_node(int value, int line, int col);                            /* NEW */
ASTNode* create_identifier_node(char* name, int line, int col);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, int line, int col);
ASTNode* create_string_node(char* value, int line, int col);
ASTNode* create_char_node(char value, int line, int col);
ASTNode* create_list_literal_node(ASTNode** elems, int count, int line, int col);
ASTNode* create_index_node(ASTNode* object, ASTNode* index, int line, int col);
ASTNode* create_list_assign_node(char* name, ASTNode* index, ASTNode* value, int line, int col);

void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif