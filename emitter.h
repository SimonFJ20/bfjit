#ifndef EMITTER_H
#define EMITTER_H

#include "expr.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* code;
    size_t pos;
    int loop_counter;
    bool cmp_flags_set;
    bool rax_contains_copy;
} Emitter;

Emitter emitter_create(uint8_t* code_address);
void emitter_push_u8(Emitter* emitter, uint8_t value);
void emitter_push_u32(Emitter* emitter, uint32_t value);
void emitter_push_u64(Emitter* emitter, uint64_t value);
void emitter_emit_expr(Emitter* emitter, Expr* expr);
void emitter_emit_loop(Emitter* emitter, Expr* expr);
void emitter_emit_expr_vec(Emitter* emitter, ExprVec* vec);
void emitter_emit_program(Emitter* emitter, ExprVec* program);

#endif
