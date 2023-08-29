#include "emitter.h"
#include "expr.h"
#include "optimizer.h"
#include "parser.h"
#include "print.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char** argv)
{
    const char* text = "++++++++++[>+<-]";
    printf("\ntext:%s\n\"%s\"%s\n", color_bright_green, text, color_reset);
    Parser parser = parser_create(lexer_from_string(text, strlen(text)));

    // Parser parser = parser_create(lexer_from_args_or_stdin(argc, argv));

    char* ast_string = malloc(sizeof(char) * 33768);
    ast_string[0] = '\0';

    ExprVec ast = parser_parse(&parser);
    {
        expr_vec_stringify(&ast, ast_string, 0);
        printf("\nparsed:\n%s\n", ast_string);
    }

    ExprVec previous_ast;
    bool first = true;
    int pass_counter = 1;
    while (first || !expr_vec_equal(&ast, &previous_ast)) {
        printf(
            "\n%soptimization pass %d:%s\n",
            color_bold,
            pass_counter,
            color_reset
        );
        pass_counter += 1;

        if (!first) {
            expr_vec_free(&previous_ast);
        }
        previous_ast = ast;
        ast = optimize_fold_adjecent(&ast);
        printf("%sfold_adjecent:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = optimize_eliminate_negation(&ast);
        printf("%seliminate_negation:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = optimize_eliminate_overflow(&ast);
        printf("%seliminate_overflow:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = optimize_replace_zeroing_loops(&ast);
        printf("%sreplace_zeroing_loops:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        if (first) {
            first = false;
        }
    }

    ast_string[0] = '\0';
    expr_vec_stringify(&ast, ast_string, 0);
    printf("\n%sfinal:%s\n%s\n", color_bold, color_reset, ast_string);

    size_t code_size = 33678;
    void* code = mmap(
        NULL,
        code_size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    if (code == NULL) {
        fprintf(stderr, "panic: could not mmap\n");
        exit(1);
    }

    Emitter emitter = emitter_create(code);
    emitter_emit_program(&emitter, &ast);

    uint8_t* memory = malloc(30000);
    memset(memory, 0, 30000);

    printf("\n%scode:%s\n", color_bold, color_reset);

    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            uint8_t v = ((uint8_t*)code)[y * 8 + x];
            if (v == 0) {
                fputs(color_gray, stdout);
            }
            printf("%02x ", v);
            fputs(color_reset, stdout);
        }
        printf("\n");
    }

    printf("\n%sresult:%s\n", color_bold, color_reset);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    void (*runnable)(uint8_t* memory) = (void (*)(uint8_t* memory))code;
#pragma GCC diagnostic pop

    runnable(memory);

    printf("\n%smemory:%s\n", color_bold, color_reset);

    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            uint8_t v = memory[y * 8 + x];
            if (v == 0) {
                fputs(color_gray, stdout);
            }
            printf("%02x ", v);
            fputs(color_reset, stdout);
        }
        printf("\n");
    }

    free(memory);
    munmap(code, code_size);
    expr_vec_free(&previous_ast);
    expr_vec_free(&ast);
    free(ast_string);
}
