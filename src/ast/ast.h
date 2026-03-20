#ifndef AST_H
#define AST_H

#include "../utils/utils.h"
#include "../lexer/lexer.h"

// AST Node Types
typedef enum {
    // Program structure
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
     
    // Statements
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    
    // Expressions
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_INTEGER,
    AST_CHAR,          // Add this
    AST_STRING,  
    AST_IDENTIFIER,
    AST_FUNCTION_CALL
} ASTNodeType;

// Forward declaration
typedef struct ASTNode ASTNode;

// Binary operation structure
typedef struct {
    TokenType op;
    ASTNode* left;
    ASTNode* right;
} BinaryOp;

// Function call structure
typedef struct {
    char* name;
    ASTNode** arguments;
    int arg_count;
} FunctionCall;

// If statement structure
typedef struct {
    ASTNode* condition;
    ASTNode* then_branch;
    ASTNode* else_branch;
} IfStmt;

// While statement structure
typedef struct {
    ASTNode* condition;
    ASTNode* body;
} WhileStmt;

// Variable declaration structure
typedef struct {
    char* name;
    ASTNode* initializer;
} VarDecl;

// Assignment structure
typedef struct {
    char* name;
    ASTNode* value;
} Assignment;

// Function structure
typedef struct {
    char* name;
    ASTNode* body;
} Function;

// Block structure
typedef struct {
    ASTNode** statements;
    int statement_count;
} Block;

// AST Node
struct ASTNode {
    ASTNodeType type;
    int line;
    int column;
    
    union {
        // Program
        struct {
            ASTNode** functions;
            int function_count;
        } program;
        
        // Function
        Function function;
        
        // Block
        Block block;
        
        // Statements
        VarDecl var_decl;
        Assignment assignment;
        ASTNode* return_value;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        
        // Expressions
        BinaryOp binary;
        int int_value;
        char* identifier;
        FunctionCall func_call;
         char char_value;      // Add this
        char* string_value;
    } data;
};

// Function prototypes
ASTNode* create_program_node(void);
void add_function(ASTNode* program, ASTNode* function);

ASTNode* create_function_node(char* name, ASTNode* body);
ASTNode* create_block_node(void);
void add_statement(ASTNode* block, ASTNode* stmt);

ASTNode* create_var_decl_node(char* name, ASTNode* init, int line, int col);
ASTNode* create_assignment_node(char* name, ASTNode* value, int line, int col);
ASTNode* create_return_node(ASTNode* value, int line, int col);
ASTNode* create_if_node(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch, int line, int col);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body, int line, int col);

ASTNode* create_binary_op_node(TokenType op, ASTNode* left, ASTNode* right, int line, int col);
ASTNode* create_integer_node(int value, int line, int col);
ASTNode* create_identifier_node(char* name, int line, int col);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, int line, int col);

void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif
