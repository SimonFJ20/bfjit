#include "optimizer.h"
#include "expr.h"

/*
 *  fold adjecent
 *
 *  A(N) :: { Incr(N) | Decr(N) | Right(N) | Left(N) }
 *
 *  [A(N1) A(N2)] -> [A(N1 + N2)]
 *
 */

ExprVec optimize_fold_adjecent(const ExprVec* vec)
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
            .exprs = optimize_fold_adjecent(&expr->exprs),
        };
    } else {
        return *expr;
    }
}

/*
 *  eliminate negation
 *
 *  A(N), B(N) :: { Incr(N) | Decr(N) | Right(N) | Left(N) }
 *
 *  [A(N) B(N)] = []
 *
 *  [A(N1) B(N2)] ? N1 == N2 -> []
 *  [A(N1) B(N2)] ? N1 < N2 -> [B(N2 - N1)]
 *  [A(N1) B(N2)] ? N1 > N2 -> [A(N1 - N2)]
 *
 */

ExprVec optimize_eliminate_negation(const ExprVec* vec)
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
            .exprs = optimize_eliminate_negation(&expr->exprs),
        };
    } else {
        return *expr;
    }
}

/*
 *  eliminate overflow
 *
 *  A(N) :: { Incr(N) | Decr(N) | Right(N) | Left(N) }
 *
 *  N > 255
 *
 *  A(N) -> A(N % 256)
 *
 */

ExprVec optimize_eliminate_overflow(const ExprVec* vec)
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
            .exprs = optimize_eliminate_overflow(&expr->exprs),
        };
    } else if (expr->value > 255) {
        return (Expr) { .type = expr->type, .value = expr->value % 256 };
    } else {
        return *expr;
    }
}

/*
 *  replace zeroing loops
 *
 *  A(N) :: { Incr(N) | Decr(N) }
 *
 *  N % 2 == 1
 *
 *  Loop[A(N)] -> Zero
 *
 */

ExprVec optimize_replace_zeroing_loops(const ExprVec* vec)
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
                .exprs = optimize_replace_zeroing_loops(&expr->exprs),
            };
        }
    } else {
        return *expr;
    }
}
