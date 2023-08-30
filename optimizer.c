#include "optimizer.h"
#include "expr.h"

/*
 *  fold adjecent
 *
 *  A(n) :: { Incr(n) | Decr(n) | Right(n) | Left(n) }
 *
 *  [A(n1) A(n2)] -> [A(n1 + n2)]
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
        return expr_clone(expr);
    }
}

/*
 *  eliminate negation
 *
 *  A(n), B(n) :: { Incr(n) | Decr(n) | Right(n) | Left(n) }
 *
 *  [A(n) B(n)] = []
 *
 *  [A(n1) B(n2)] ? n1 == n2 -> []
 *  [A(n1) B(n2)] ? n1 < n2 -> [B(n2 - n1)]
 *  [A(n1) B(n2)] ? n1 > n2 -> [A(n1 - n2)]
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
        return expr_clone(expr);
    }
}

/*
 *  eliminate overflow
 *
 *  A(n) :: { Incr(n) | Decr(n) | Right(n) | Left(n) }
 *
 *  n > 255
 *
 *  A(n) -> A(n % 256)
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
        return expr_clone(expr);
    }
}

/*
 *  replace zeroing loops
 *
 *  A(n) :: { Incr(n) | Decr(n) }
 *
 *  n % 2 == 1
 *
 *  Loop[A(n)] -> Zero
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
        return expr_clone(expr);
    }
}

/*
 *  replace copying loops
 *
 *  O(n), I(n) :: { Left(n) | Right(n) }
 *
 *  O != I
 *
 *  [Loop[O(n) Incr(1) I(n) Decr(1)]] -> [Copy(n) Zero]
 *
 */

ExprVec optimize_replace_copying_loops(const ExprVec* original)
{
    ExprVec result;
    expr_vec_construct(&result);
    for (size_t i = 0; i < original->length; ++i) {
        const Expr* expr = &original->data[i];
        if (expr->type == ExprType_Loop) {
            const ExprVec* loop = &expr->exprs;
            if (loop->length == 4
                && ((loop->data[0].type == ExprType_Left
                     && loop->data[2].type == ExprType_Right)
                    || (loop->data[0].type == ExprType_Right
                        && loop->data[2].type == ExprType_Left))
                && loop->data[1].type == ExprType_Incr
                && loop->data[3].type == ExprType_Decr
                && loop->data[1].value == loop->data[3].value) {
                if (loop->data[0].type == ExprType_Right) {
                    expr_vec_push(
                        &result,
                        (Expr) {
                            .type = ExprType_Add,
                            .value = loop->data[0].value,
                        }
                    );
                } else {
                    expr_vec_push(
                        &result,
                        (Expr) {
                            .type = ExprType_Add,
                            .value = -loop->data[0].value,
                        }
                    );
                }
                expr_vec_push(&result, (Expr) { .type = ExprType_Zero });
            } else {
                expr_vec_push(
                    &result,
                    (Expr) {
                        .type = ExprType_Loop,
                        .exprs = optimize_replace_copying_loops(&expr->exprs),
                    }
                );
            }
        } else {
            expr_vec_push(&result, expr_clone(expr));
        }
    }
    return result;
}
