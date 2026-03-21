#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "symbol_table/symbol_table.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"
#include "utils/utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.my>\n", argv[0]);
        return 1;
    }

    printf("Simple C Compiler\n");
    printf("=================\n\n");

    /* ── read source ── */
    printf("Reading source file: %s\n", argv[1]);
    char* source = read_file(argv[1]);
    if (!source) {
        fprintf(stderr, "Error: Cannot read file '%s'\n", argv[1]);
        return 1;
    }

    /* ── Phase 1: Lexical analysis ── */
    printf("[Phase 1] Lexical Analysis...\n");
    Lexer* lexer = init_lexer(source);

    printf("\n--- TOKEN OUTPUT ---\n");
    Token* tok;
    int token_count = 0;
    do {
        tok = get_next_token(lexer);
        TokenType type = tok->type;
        printf("Token %2d: Type=%-15s Lexeme='%s' Line=%d Col=%d\n",
               token_count++,
               token_type_to_string(type),
               tok->lexeme ? tok->lexeme : "",
               tok->line, tok->column);
        free_token(tok);
        if (type == TOKEN_EOF) break;
    } while (1);
    printf("--- END TOKENS ---\n\n");

    /* Reset lexer for parsing pass */
    free_lexer(lexer);
    lexer = init_lexer(source);

    /* ── Phase 2: Parsing ── */
    printf("[Phase 2] Parsing...\n");
    Parser* parser = init_parser(lexer);
    ASTNode* ast   = parse_program(parser);

    /* Collect parse errors from both error_at() and parser->error_count */
    int parse_errors = get_parser_error_count(parser) + get_error_count();

    if (parse_errors > 0) {
        fprintf(stderr, "\n── Parsing failed ──\n");
        fprintf(stderr,
            "Fix the syntax errors above before the compiler can continue.\n");
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }

    printf("Parsing completed successfully\n");
    clear_errors();   /* reset for semantic phase */

    /* ── Phase 3: Semantic analysis ── */
    printf("\n[Phase 3] Semantic Analysis...\n");
    SemanticAnalyzer* semantic = create_semantic_analyzer();
    analyze_program(semantic, ast);

    int sem_errors = get_semantic_error_count(semantic) + get_error_count();

    if (sem_errors > 0) {
        fprintf(stderr, "\n── Semantic analysis failed ──\n");
        fprintf(stderr,
            "Fix the %d error(s) above before the compiler can generate code.\n",
            sem_errors);
        free_semantic_analyzer(semantic);
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return 1;
    }

    printf("\nSymbol Table after semantic analysis:\n");
    print_symbol_table(semantic->symbols);
    clear_errors();   /* reset for codegen phase */

    /* ── Phase 4: Code generation ── */
    printf("\n[Phase 4] Code Generation...\n");
    CodeGenerator* generator = create_code_generator("output/output.asm");
    generator->symbols = semantic->symbols;
    generate_code(generator, ast);

    /* cleanup */
    free_code_generator(generator);
    free_semantic_analyzer(semantic);
    free_ast(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(source);

    printf("\nCompilation successful!\n");
    printf("Output: output/output.asm\n");
    return 0;
}