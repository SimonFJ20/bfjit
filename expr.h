#ifndef EXPR_H
#define EXPR_H

#include <stdbool.h>
#include <stddef.h>

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

void expr_vec_construct(ExprVec* vec);
void expr_vec_destroy(ExprVec* vec);
void expr_vec_free(ExprVec* vec);
void expr_vec_push(ExprVec* vec, Expr expr);
Expr expr_vec_pop(ExprVec* vec);
bool expr_vec_equal(const ExprVec* self, const ExprVec* other);

struct Expr {
    ExprType type;
    union {
        int value;
        ExprVec exprs;
    };
};

void expr_free(Expr* expr);
const char* expr_bracket_color(int depth);
void expr_stringify_concat_value(Expr* expr, char* acc, int depth);
void expr_vec_stringify(ExprVec* vec, char* acc, int depth);
void expr_stringify(Expr* expr, char* acc, int depth);
bool expr_equal(const Expr* self, const Expr* other);

#endif
