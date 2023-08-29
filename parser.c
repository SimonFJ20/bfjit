#include "parser.h"
#include <stdlib.h>

Lexer lexer_from_string(const char* text, size_t length)
{
    Lexer lexer = (Lexer) {
        .type = LexerType_String,
        .current = text[0],
        .string = {
            .text = text,
            .index = 0,
            .length = length,
        },
    };
    return lexer;
}

Lexer lexer_from_file(FILE* file)
{
    Lexer lexer = (Lexer) {
        .type = LexerType_File,
        .current = 1,
        .file = file,
    };
    lexer_step(&lexer);
    return lexer;
}

Lexer lexer_from_args_or_stdin(int argc, char** argv)
{
    if (argc >= 2) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "panic: could not open file \"%s\"\n", argv[1]);
            exit(1);
        }
        return lexer_from_file(file);
    } else {
        return lexer_from_file(stdin);
    }
}

bool lexer_done(Lexer* lexer)
{
    switch (lexer->type) {
        case LexerType_String:
            return lexer->string.index >= lexer->string.length;
        case LexerType_File:
            return lexer->current == '\0';
    }
    exit(1);
}

void lexer_step(Lexer* lexer)
{
    if (lexer_done(lexer)) {
        return;
    }
    switch (lexer->type) {
        case LexerType_String:
            lexer->string.index += 1;
            lexer->current = lexer->string.text[lexer->string.index];
            break;
        case LexerType_File: {
            int c = fgetc(lexer->file);
            lexer->current = c != EOF ? (char)c : '\0';
        } break;
    }
}

Token lexer_next(Lexer* lexer)
{
    if (lexer_done(lexer)) {
        return Token_Eof;
    }
    switch (lexer->current) {
        case '+':
            return (lexer_step(lexer), Token_Plus);
        case '-':
            return (lexer_step(lexer), Token_Minus);
        case '<':
            return (lexer_step(lexer), Token_LT);
        case '>':
            return (lexer_step(lexer), Token_GT);
        case '.':
            return (lexer_step(lexer), Token_Dot);
        case ',':
            return (lexer_step(lexer), Token_Comma);
        case '[':
            return (lexer_step(lexer), Token_LBracket);
        case ']':
            return (lexer_step(lexer), Token_RBracket);
        default:
            return (lexer_step(lexer), lexer_next(lexer));
    }
}

Parser parser_create(Lexer lexer)
{
    return (Parser) {
        .lexer = lexer,
        .current = lexer_next(&lexer),
    };
}

void parser_step(Parser* parser)
{
    parser->current = lexer_next(&parser->lexer);
}
Expr parser_parse_expr(Parser* parser);

Expr parser_parse_loop(Parser* parser)
{
    parser_step(parser);
    ExprVec exprs;
    expr_vec_construct(&exprs);
    while (parser->current != Token_Eof && parser->current != Token_RBracket) {
        expr_vec_push(&exprs, parser_parse_expr(parser));
    }
    if (parser->current != Token_RBracket) {
        return (Expr) { .type = ExprType_Error };
    }
    parser_step(parser);
    return (Expr) { .type = ExprType_Loop, .exprs = exprs };
}

Expr parser_parse_expr(Parser* parser)
{
    switch (parser->current) {
        case Token_Plus:
            return (
                parser_step(parser),
                (Expr) { .type = ExprType_Incr, .value = 1 }
            );
        case Token_Minus:
            return (
                parser_step(parser),
                (Expr) { .type = ExprType_Decr, .value = 1 }
            );
        case Token_LT:
            return (
                parser_step(parser),
                (Expr) { .type = ExprType_Left, .value = 1 }
            );
        case Token_GT:
            return (
                parser_step(parser),
                (Expr) { .type = ExprType_Right, .value = 1 }
            );
        case Token_Dot:
            return (parser_step(parser), (Expr) { .type = ExprType_Output });
        case Token_Comma:
            return (parser_step(parser), (Expr) { .type = ExprType_Input });
        case Token_LBracket:
            return parser_parse_loop(parser);
        default:
            return (parser_step(parser), (Expr) { .type = ExprType_Error });
    }
}

ExprVec parser_parse(Parser* parser)
{
    ExprVec exprs;
    expr_vec_construct(&exprs);
    while (parser->current != Token_Eof) {
        expr_vec_push(&exprs, parser_parse_expr(parser));
    }
    return exprs;
}
