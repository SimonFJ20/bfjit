#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

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

typedef struct {
    const char* text;
    size_t index;
    size_t length;
} Lexer;

Lexer lexer_create(const char* text, size_t length)
{
    return (Lexer) { .text = text, .index = 0, .length = length };
}

void lexer_step(Lexer* lexer) { lexer->index += 1; }

Token lexer_next(Lexer* lexer)
{
    if (lexer->index >= lexer->length) {
        return Token_Eof;
    }
    switch (lexer->text[lexer->index]) {
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

typedef enum {
    ExprType_Error,
    ExprType_Incr,
    ExprType_Decr,
    ExprType_Left,
    ExprType_Right,
    ExprType_Output,
    ExprType_Input,
    ExprType_Loop,
    ExprType_Zero,
} ExprType;

typedef struct Expr Expr;

typedef struct ExprVec {
    Expr* data;
    size_t capacity;
    size_t length;
} ExprVec;

struct Expr {
    ExprType type;
    union {
        int value;
        ExprVec exprs;
    };
};

void expr_vec_construct(ExprVec* vec)
{
    *vec = (ExprVec) {
        .data = malloc(sizeof(Expr) * 8),
        .capacity = 8,
        .length = 0,
    };
}
void expr_vec_destroy(ExprVec* vec) { free(vec->data); }

void expr_free(Expr* expr);
void expr_vec_free(ExprVec* vec)
{
    for (size_t i = 0; i < vec->length; ++i) {
        expr_free(&vec->data[i]);
    }
    expr_vec_destroy(vec);
}

void expr_vec_push(ExprVec* vec, Expr expr)
{
    if (vec->length + 1 > vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(Expr) * vec->capacity);
    }
    vec->data[vec->length] = expr;
    vec->length += 1;
}
Expr expr_vec_pop(ExprVec* vec)
{
    vec->length -= 1;
    return vec->data[vec->length];
}

void expr_free(Expr* expr)
{
    switch (expr->type) {
        case ExprType_Loop:
            expr_vec_free(&expr->exprs);
            break;
        default:
            break;
    }
}

const char* color_reset = "\x1b[0m";
const char* color_bold = "\x1b[1m";

const char* color_black = "\x1b[30m";
const char* color_red = "\x1b[31m";
const char* color_green = "\x1b[32m";
const char* color_yellow = "\x1b[33m";
const char* color_blue = "\x1b[34m";
const char* color_magenta = "\x1b[35m";
const char* color_cyan = "\x1b[36m";
const char* color_bright_gray = "\x1b[37m";

const char* color_gray = "\x1b[90m";
const char* color_bright_red = "\x1b[91m";
const char* color_bright_green = "\x1b[92m";
const char* color_bright_yellow = "\x1b[93m";
const char* color_bright_blue = "\x1b[94m";
const char* color_bright_magenta = "\x1b[95m";
const char* color_bright_cyan = "\x1b[96m";
const char* color_white = "\x1b[97m";

const char* expr_bracket_color(int depth)
{
    switch (depth % 3) {
        case 0:
            return color_bright_yellow;
        case 1:
            return color_magenta;
        case 2:
            return color_cyan;
    }
    return NULL;
}

void expr_stringify_concat_value(Expr* expr, char* acc, int depth)
{
    strcat(acc, color_bold);
    strcat(acc, expr_bracket_color(depth));
    strcat(acc, "(");
    strcat(acc, color_reset);
    char value[16] = { 0 };
    snprintf(value, 16, "%d", expr->value);
    strcat(acc, value);
    strcat(acc, color_bold);
    strcat(acc, expr_bracket_color(depth));
    strcat(acc, ")");
    strcat(acc, color_reset);
}

void expr_stringify(Expr* expr, char* acc, int depth);

void expr_vec_stringify(ExprVec* vec, char* acc, int depth)
{
    strcat(acc, color_bold);
    strcat(acc, expr_bracket_color(depth));
    strcat(acc, "[");
    strcat(acc, color_reset);
    for (size_t i = 0; i < vec->length; ++i) {
        if (i != 0) {
            strcat(acc, " ");
        }
        expr_stringify(&vec->data[i], acc, depth + 1);
    }
    strcat(acc, color_bold);
    strcat(acc, expr_bracket_color(depth));
    strcat(acc, "]");
    strcat(acc, color_reset);
}

void expr_stringify(Expr* expr, char* acc, int depth)
{
    switch (expr->type) {
        case ExprType_Error:
            strcat(acc, color_bright_red);
            strcat(acc, "Error");
            strcat(acc, color_reset);
            break;
        case ExprType_Incr:
            strcat(acc, color_yellow);
            strcat(acc, "Incr");
            strcat(acc, color_reset);
            expr_stringify_concat_value(expr, acc, depth);
            break;
        case ExprType_Decr:
            strcat(acc, color_yellow);
            strcat(acc, "Decr");
            strcat(acc, color_reset);
            expr_stringify_concat_value(expr, acc, depth);
            break;
        case ExprType_Left:
            strcat(acc, color_green);
            strcat(acc, "Left");
            strcat(acc, color_reset);
            expr_stringify_concat_value(expr, acc, depth);
            break;
        case ExprType_Right:
            strcat(acc, color_green);
            strcat(acc, "Right");
            strcat(acc, color_reset);
            expr_stringify_concat_value(expr, acc, depth);
            break;
        case ExprType_Output:
            strcat(acc, color_bright_gray);
            strcat(acc, "Output");
            strcat(acc, color_reset);
            break;
        case ExprType_Input:
            strcat(acc, color_bright_gray);
            strcat(acc, "Input");
            strcat(acc, color_reset);
            break;
        case ExprType_Loop:
            strcat(acc, color_bright_red);
            strcat(acc, "Loop");
            expr_vec_stringify(&expr->exprs, acc, depth);
            strcat(acc, color_reset);
            break;
        case ExprType_Zero:
            strcat(acc, color_yellow);
            strcat(acc, "Zero");
            strcat(acc, color_reset);
            break;
    }
}

bool expr_equal(const Expr* self, const Expr* other);

bool expr_vec_equal(const ExprVec* self, const ExprVec* other)
{
    if (self->length != other->length) {
        return false;
    }
    for (size_t i = 0; i < self->length; ++i) {
        if (!expr_equal(&self->data[i], &other->data[i])) {
            return false;
        }
    }
    return true;
}

bool expr_equal(const Expr* self, const Expr* other)
{
    if (self->type != other->type) {
        return false;
    }
    switch (self->type) {
        case ExprType_Incr:
        case ExprType_Decr:
        case ExprType_Left:
        case ExprType_Right:
            if (self->value != other->value) {
                return false;
            }
            break;
        case ExprType_Loop:
            if (!expr_vec_equal(&self->exprs, &other->exprs)) {
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

Parser parser_create(const char* text, size_t length)
{
    Lexer lexer = lexer_create(text, length);
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

Expr expr_optimize_fold_adjecent(const Expr* expr);
ExprVec expr_vec_optimize_fold_adjecent(const ExprVec* vec)
{
    ExprVec exprs;
    expr_vec_construct(&exprs);
    if (vec->length == 0) {
        return exprs;
    }
    Expr a = expr_optimize_fold_adjecent(&vec->data[0]);
    for (size_t i = 1; i < vec->length; ++i) {
        Expr b = expr_optimize_fold_adjecent(&vec->data[i]);
        switch (a.type) {
            case ExprType_Incr:
            case ExprType_Decr:
            case ExprType_Left:
            case ExprType_Right:
                if (a.type != b.type) {
                    expr_vec_push(&exprs, a);
                    a = b;
                } else {
                    a.value += b.value;
                }
                break;
            default:
                expr_vec_push(&exprs, a);
                a = b;
        }
    }
    expr_vec_push(&exprs, a);
    return exprs;
}
Expr expr_optimize_fold_adjecent(const Expr* expr)
{
    if (expr->type == ExprType_Loop) {
        return (Expr) {
            .type = ExprType_Loop,
            .exprs = expr_vec_optimize_fold_adjecent(&expr->exprs),
        };
    } else {
        return *expr;
    }
}

Expr expr_optimize_eliminate_negation(const Expr* expr);
ExprVec expr_vec_optimize_eliminate_negation(const ExprVec* vec)
{
    ExprVec exprs;
    expr_vec_construct(&exprs);
    if (vec->length == 0) {
        return exprs;
    }
    expr_vec_push(&exprs, expr_optimize_eliminate_negation(&vec->data[0]));
    for (size_t i = 1; i < vec->length; ++i) {
        expr_vec_push(&exprs, expr_optimize_eliminate_negation(&vec->data[i]));
        Expr* a = &exprs.data[i - 1];
        Expr* b = &exprs.data[i];
        if (a->type == ExprType_Incr && b->type == ExprType_Decr) {
            if (a->value > b->value) {
                a->value -= b->value;
                expr_vec_pop(&exprs);
            } else if (a->value < b->value) {
                *a = (Expr) { .type = ExprType_Decr,
                              .value = b->value - a->value };
                expr_vec_pop(&exprs);
            } else {
                expr_vec_pop(&exprs);
                expr_vec_pop(&exprs);
            }
        } else if (a->type == ExprType_Decr && b->type == ExprType_Incr) {
            if (a->value > b->value) {
                a->value -= b->value;
                expr_vec_pop(&exprs);
            } else if (a->value < b->value) {
                *a = (Expr) { .type = ExprType_Incr,
                              .value = b->value - a->value };
                expr_vec_pop(&exprs);
            } else {
                expr_vec_pop(&exprs);
                expr_vec_pop(&exprs);
            }
        }
        if (a->type == ExprType_Left && b->type == ExprType_Right) {
            if (a->value > b->value) {
                a->value -= b->value;
                expr_vec_pop(&exprs);
            } else if (a->value < b->value) {
                *a = (Expr) { .type = ExprType_Right,
                              .value = b->value - a->value };
                expr_vec_pop(&exprs);
            } else {
                expr_vec_pop(&exprs);
                expr_vec_pop(&exprs);
            }
        } else if (a->type == ExprType_Right && b->type == ExprType_Left) {
            if (a->value > b->value) {
                a->value -= b->value;
                expr_vec_pop(&exprs);
            } else if (a->value < b->value) {
                *a = (Expr) { .type = ExprType_Left,
                              .value = b->value - a->value };
                expr_vec_pop(&exprs);
            } else {
                expr_vec_pop(&exprs);
                expr_vec_pop(&exprs);
            }
        }
    }
    return exprs;
}
Expr expr_optimize_eliminate_negation(const Expr* expr)
{
    if (expr->type == ExprType_Loop) {
        return (Expr) {
            .type = ExprType_Loop,
            .exprs = expr_vec_optimize_eliminate_negation(&expr->exprs),
        };
    } else {
        return *expr;
    }
}

Expr expr_optimize_eliminate_overflow(const Expr* expr);
ExprVec expr_vec_optimize_eliminate_overflow(const ExprVec* vec)
{
    ExprVec exprs;
    expr_vec_construct(&exprs);
    for (size_t i = 0; i < vec->length; ++i) {
        expr_vec_push(&exprs, expr_optimize_eliminate_overflow(&vec->data[i]));
    }
    return exprs;
}
Expr expr_optimize_eliminate_overflow(const Expr* expr)
{
    if (expr->type == ExprType_Loop) {
        return (Expr) {
            .type = ExprType_Loop,
            .exprs = expr_vec_optimize_eliminate_overflow(&expr->exprs),
        };
    } else if (expr->value > 255) {
        return (Expr) { .type = expr->type, .value = expr->value % 256 };
    } else {
        return *expr;
    }
}

Expr expr_optimize_replace_zeroing_loops(const Expr* expr);
ExprVec expr_vec_optimize_replace_zeroing_loops(const ExprVec* vec)
{
    ExprVec exprs;
    expr_vec_construct(&exprs);
    for (size_t i = 0; i < vec->length; ++i) {
        expr_vec_push(
            &exprs, expr_optimize_replace_zeroing_loops(&vec->data[i])
        );
    }
    return exprs;
}
Expr expr_optimize_replace_zeroing_loops(const Expr* expr)
{
    if (expr->type == ExprType_Loop) {
        if (expr->exprs.length == 1
            && (expr->exprs.data[0].type == ExprType_Incr
                || expr->exprs.data[0].type == ExprType_Decr)
            && expr->exprs.data[0].value % 2 != 0) {
            return (Expr) { .type = ExprType_Zero };
        } else {
            return (Expr) {
                .type = ExprType_Loop,
                .exprs = expr_vec_optimize_replace_zeroing_loops(&expr->exprs),
            };
        }
    } else {
        return *expr;
    }
}

typedef struct {
    uint8_t* code;
    size_t pos;
    int loop_counter;
    bool cmp_flags_set;
} Emitter;

Emitter emitter_create(uint8_t* code_address)
{
    return (Emitter) {
        .code = code_address,
        .pos = 0,
        .loop_counter = 0,
        .cmp_flags_set = false,
    };
}

void emitter_push_u8(Emitter* emitter, uint8_t value)
{
    emitter->code[emitter->pos] = value;
    emitter->pos += 1;
}

void emitter_push_u32(Emitter* emitter, uint32_t value)
{
    emitter->code[emitter->pos] = value & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 8) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 16) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = value >> 24;
    emitter->pos += 1;
}

void emitter_push_u64(Emitter* emitter, uint64_t value)
{
    emitter->code[emitter->pos] = value & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 8) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 16) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 24) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 32) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 40) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 48) & 0xFF;
    emitter->pos += 1;
    emitter->code[emitter->pos] = (value >> 56) & 0xFF;
    emitter->pos += 1;
}

uint8_t get_char(void) { return (uint8_t)fgetc(stdin); }
void put_char(uint8_t v) { fputc(v, stdout); }

void emitter_emit_expr(Emitter* emitter, Expr* expr)
{
    emitter->cmp_flags_set = false;
    switch (expr->type) {
        case ExprType_Error:
            fprintf(stderr, "panic: emitter: program contained errors\n");
            exit(1);
            break;
        case ExprType_Incr:
            // add BYTE [rbx], 1
            emitter_push_u8(emitter, 0x80);
            emitter_push_u8(emitter, 0x03);
            emitter_push_u8(emitter, expr->value);
            emitter->cmp_flags_set = true;
            break;
        case ExprType_Decr:
            // sub BYTE [rbx], 1
            emitter_push_u8(emitter, 0x80);
            emitter_push_u8(emitter, 0x2b);
            emitter_push_u8(emitter, expr->value);
            emitter->cmp_flags_set = true;
            break;
        case ExprType_Left:
            // sub rbx, 1
            emitter_push_u8(emitter, 0x48);
            emitter_push_u8(emitter, 0x83);
            emitter_push_u8(emitter, 0xeb);
            emitter_push_u8(emitter, expr->value);
            break;
        case ExprType_Right:
            // add rbx, 1
            emitter_push_u8(emitter, 0x48);
            emitter_push_u8(emitter, 0x83);
            emitter_push_u8(emitter, 0xc3);
            emitter_push_u8(emitter, expr->value);
            break;
        case ExprType_Output:
            // movzx edi, BYTE [rbx]
            emitter_push_u8(emitter, 0x0f);
            emitter_push_u8(emitter, 0xb6);
            emitter_push_u8(emitter, 0x3b);
            // movabs rax, <put_char>
            emitter_push_u8(emitter, 0x48);
            emitter_push_u8(emitter, 0xb8);
            emitter_push_u64(emitter, (uint64_t)put_char);
            // call rax
            emitter_push_u8(emitter, 0xff);
            emitter_push_u8(emitter, 0xd0);
            break;
        case ExprType_Input:
            // movabs rax, <put_char>
            emitter_push_u8(emitter, 0x48);
            emitter_push_u8(emitter, 0xb8);
            emitter_push_u64(emitter, (uint64_t)get_char);
            // call rax
            emitter_push_u8(emitter, 0xff);
            emitter_push_u8(emitter, 0xd0);
            // mov BYTE [rbx], al
            emitter_push_u8(emitter, 0x88);
            emitter_push_u8(emitter, 0x03);
            break;
        case ExprType_Loop:
            fprintf(stderr, "panic: emitter: unexpected loop\n");
            exit(1);
            break;
        case ExprType_Zero:
            // mov BYTE [rbx], 0
            emitter_push_u8(emitter, 0xc6);
            emitter_push_u8(emitter, 0x03);
            emitter_push_u8(emitter, 0x00);
            emitter->cmp_flags_set = true;
            break;
    }
}

void emitter_emit_expr_vec(Emitter* emitter, ExprVec* vec);

void emitter_emit_loop(Emitter* emitter, Expr* expr)
{
    int64_t start_loc = (int64_t)&emitter->code[emitter->pos];
    emitter_emit_expr_vec(emitter, &expr->exprs);
    if (!emitter->cmp_flags_set) {
        // cmp BYTE [rbx], 0
        emitter_push_u8(emitter, 0x80);
        emitter_push_u8(emitter, 0x3b);
        emitter_push_u8(emitter, 0x00);
    }

    int64_t current_loc = (int64_t)&emitter->code[emitter->pos];
    int32_t relative_address = -(int32_t)(current_loc - start_loc);
    if (relative_address >= -127) {
        // jne <current_loc: rel8>
        emitter_push_u8(emitter, 0x75);
        emitter_push_u8(emitter, (uint8_t)relative_address - 2);
    } else {
        // jne <current_loc: rel32>
        emitter_push_u8(emitter, 0x0f);
        emitter_push_u8(emitter, 0x85);
        emitter_push_u32(emitter, (uint32_t)relative_address - 6);
    }
}

void emitter_emit_expr_vec(Emitter* emitter, ExprVec* vec)
{
    for (size_t i = 0; i < vec->length; ++i) {
        Expr* expr = &vec->data[i];
        if (expr->type == ExprType_Loop) {
            emitter_emit_loop(emitter, expr);
        } else {
            emitter_emit_expr(emitter, expr);
        }
    }
}

void emitter_emit_program(Emitter* emitter, ExprVec* program)
{
    // push rbp:
    emitter_push_u8(emitter, 0x55);
    // mov rbp, rsp
    emitter_push_u8(emitter, 0x48);
    emitter_push_u8(emitter, 0x89);
    emitter_push_u8(emitter, 0xe5);
    // push rbx:
    emitter_push_u8(emitter, 0x53);
    // mov rbx, rdi
    emitter_push_u8(emitter, 0x48);
    emitter_push_u8(emitter, 0x89);
    emitter_push_u8(emitter, 0xfb);

    emitter_emit_expr_vec(emitter, program);

    // pop rbx
    emitter_push_u8(emitter, 0x5b);
    // pop rbx
    emitter_push_u8(emitter, 0x5d);
    // ret
    emitter_push_u8(emitter, 0xc3);
}

int main(void)
{
    // const char* text
    //     = "+++[>[-]++++++++++++++++++++++++++++++++++++++++++++++++++"
    //       "++++++++++++++++++"
    //       "++++.---.++++++++++++++++++++.[-]++++++++++.<-]";
    const char* text = ">++[<+++++++++++++>-]<[[>+>+<<-]>[<+>-]++++++++"
                       "[>++++++++<-]>.[-]<<>++++++++++[>++++++++++[>++"
                       "++++++++[>++++++++++[>++++++++++[>++++++++++[>+"
                       "+++++++++[-]<-]<-]<-]<-]<-]<-]<-]++++++++++.";

    printf("\ntext:%s\n\"%s\"%s\n", color_bright_green, text, color_reset);

    char* ast_string = malloc(sizeof(char) * 33768);
    ast_string[0] = '\0';

    Parser parser = parser_create(text, strlen(text));
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
        ast = expr_vec_optimize_fold_adjecent(&ast);
        printf("%sfold_adjecent:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = expr_vec_optimize_eliminate_negation(&ast);
        printf("%seliminate_negation:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = expr_vec_optimize_eliminate_overflow(&ast);
        printf("%seliminate_overflow:%s\n", color_bold, color_reset);
        if (!expr_vec_equal(&ast, &previous_ast)) {
            ast_string[0] = '\0';
            expr_vec_stringify(&ast, ast_string, 0);
            puts(ast_string);
        }

        expr_vec_free(&previous_ast);
        previous_ast = ast;
        ast = expr_vec_optimize_replace_zeroing_loops(&ast);
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
    uint8_t* code = mmap(
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

    void (*runnable)(uint8_t* memory) = (void (*)(uint8_t* memory))(void*)code;

    printf("\n%scode:%s\n", color_bold, color_reset);

    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            uint8_t v = code[y * 8 + x];
            if (v == 0) {
                fputs(color_gray, stdout);
            }
            printf("%02x ", v);
            fputs(color_reset, stdout);
        }
        printf("\n");
    }

    printf("\n%sresult:%s\n", color_bold, color_reset);

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
