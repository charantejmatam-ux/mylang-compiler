#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "symbol_table/symbol_table.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"
#include "utils/utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }
    
    printf("Simple C Compiler\n");
    printf("=================\n\n");
    
    // Read source file
    printf("Reading source file: %s\n", argv[1]);
    char* source = read_file(argv[1]);
    if (!source) {
        fprintf(stderr, "Error: Cannot read file '%s'\n", argv[1]);
        return 1;
    }
    
    // Lexical analysis
    printf("[Phase 1] Lexical Analysis...\n");
    Lexer* lexer = init_lexer(source);

    // DEBUG: Print all tokens
    printf("\n--- TOKEN OUTPUT ---\n");
    Token* debug_token;
    int token_count = 0;
    do {
        debug_token = get_next_token(lexer);
        TokenType type = debug_token->type;
        printf("Token %2d: Type=%-15s Lexeme='%s' Line=%d Col=%d\n", 
               token_count++,
               token_type_to_string(type),
               debug_token->lexeme ? debug_token->lexeme : "",
               debug_token->line,
               debug_token->column);
        free_token(debug_token);
        
        if (type == TOKEN_EOF) break;
    } while (1);
    printf("--- END TOKENS ---\n\n");

    // Reset lexer
    free_lexer(lexer);
    lexer = init_lexer(source);

    // Parsing
    printf("[Phase 2] Parsing...\n");
    Parser* parser = init_parser(lexer);
    ASTNode* ast = parse_program(parser);
    
    if (get_parser_error_count(parser) > 0) {
        fprintf(stderr, "\nParsing failed with %d errors\n", 
                get_parser_error_count(parser));
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }
    
    printf("Parsing completed successfully\n");
    
    // Semantic analysis
    printf("\n[Phase 3] Semantic Analysis...\n");
    SemanticAnalyzer* semantic = create_semantic_analyzer();
    analyze_program(semantic, ast);
    
    if (get_semantic_error_count(semantic) > 0) {
        fprintf(stderr, "\nCompilation failed due to semantic errors\n");
        free_semantic_analyzer(semantic);
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }
    
    // Print symbol table for debugging
    printf("\nSymbol Table after semantic analysis:\n");
    print_symbol_table(semantic->symbols);
    
    // Code generation
    printf("\n[Phase 4] Code Generation...\n");
    CodeGenerator* generator = create_code_generator("output/output.asm");
    
    // Share the symbol table (don't transfer ownership)
    generator->symbols = semantic->symbols;
    
    generate_code(generator, ast);
    
    // Cleanup - order matters!
    free_code_generator(generator);        // This does NOT free the symbol table
    free_semantic_analyzer(semantic);      // This frees the symbol table
    free_ast(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(source);
    
    printf("\nCompilation successful!\n");
    printf("Output: output/output.asm\n");
    
    return 0;
}
