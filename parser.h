#ifndef PARSER_H
#define PARSER_H

#include "expr.h"
#include <stdio.h>

typedef enum {
    Token_Eof,
    Token_Plus,
    Token_Minus,
    Token_LT,
    Token_GT,
    Token_Dot,
    Token_Comma,
    Token_LBracket,
    Token_RBracket
} Token;

typedef enum {
    LexerType_String,
    LexerType_File,
} LexerType;

typedef struct {
    LexerType type;
    char current;
    union {
        struct {
            const char* text;
            size_t index;
            size_t length;
        } string;
        FILE* file;
    };
} Lexer;

Lexer lexer_from_string(const char* text, size_t length);
Lexer lexer_from_file(FILE* file);
Lexer lexer_from_args_or_stdin(int argc, char** argv);
bool lexer_done(Lexer* lexer);
void lexer_step(Lexer* lexer);
Token lexer_next(Lexer* lexer);

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

Parser parser_create(Lexer lexer);
void parser_step(Parser* parser);
Expr parser_parse_loop(Parser* parser);
Expr parser_parse_expr(Parser* parser);
ExprVec parser_parse(Parser* parser);

#endif
