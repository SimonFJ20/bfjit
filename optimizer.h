#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "expr.h"

ExprVec optimize_fold_adjecent(const ExprVec* vec);
Expr expr_optimize_fold_adjecent(const Expr* expr);

ExprVec optimize_eliminate_negation(const ExprVec* vec);
Expr expr_optimize_eliminate_negation(const Expr* expr);

ExprVec optimize_eliminate_overflow(const ExprVec* vec);
Expr expr_optimize_eliminate_overflow(const Expr* expr);

ExprVec optimize_replace_zeroing_loops(const ExprVec* vec);
Expr expr_optimize_replace_zeroing_loops(const Expr* expr);

#endif
