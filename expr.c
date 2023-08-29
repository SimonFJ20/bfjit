#include "expr.h"
#include "print.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void expr_vec_construct(ExprVec* vec)
{
    *vec = (ExprVec) {
        .data = malloc(sizeof(Expr) * 8),
        .capacity = 8,
        .length = 0,
    };
}

void expr_vec_destroy(ExprVec* vec) { free(vec->data); }

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
