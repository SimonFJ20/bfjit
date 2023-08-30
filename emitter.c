#include "emitter.h"
#include "expr.h"
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

Emitter emitter_create(uint8_t* code_address)
{
    return (Emitter) {
        .code = code_address,
        .pos = 0,
        .loop_counter = 0,
        .cmp_flags_set = false,
        .rax_contains_copy = false,
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

inline bool is_8(int value) { return value >= -128 && value <= 127; }
inline bool is_16(int value) { return value >= -32768 && value <= 32767; }

void emitter_emit_expr(Emitter* emitter, Expr* expr)
{
    emitter->cmp_flags_set = false;
    switch (expr->type) {
        case ExprType_Error:
            fprintf(stderr, "panic: emitter: program contained errors\n");
            exit(1);
            break;
        case ExprType_Incr:
            // add BYTE [rbx], <value, rel8>
            emitter_push_u8(emitter, 0x80);
            emitter_push_u8(emitter, 0x03);
            emitter_push_u8(emitter, (uint8_t)expr->value);
            emitter->cmp_flags_set = true;
            break;
        case ExprType_Decr:
            // sub BYTE [rbx], <value: rel8>
            emitter_push_u8(emitter, 0x80);
            emitter_push_u8(emitter, 0x2b);
            emitter_push_u8(emitter, (uint8_t)expr->value);
            emitter->cmp_flags_set = true;
            break;
        case ExprType_Left:
            if (is_8(expr->value)) {
                // sub rbx, <value: rel8>
                emitter_push_u8(emitter, 0x48);
                emitter_push_u8(emitter, 0x83);
                emitter_push_u8(emitter, 0xeb);
                emitter_push_u8(emitter, (uint8_t)expr->value);
            } else {
                // sub rbx, <value: rel32>
                emitter_push_u8(emitter, 0x48);
                emitter_push_u8(emitter, 0x81);
                emitter_push_u8(emitter, 0xeb);
                emitter_push_u32(emitter, expr->value);
            }
            break;
        case ExprType_Right:
            if (is_8(expr->value)) {
                // add rbx, <value: rel8>
                emitter_push_u8(emitter, 0x48);
                emitter_push_u8(emitter, 0x83);
                emitter_push_u8(emitter, 0xc3);
                emitter_push_u8(emitter, (uint8_t)expr->value);
            } else {
                // add rbx, <value: rel32>
                emitter_push_u8(emitter, 0x48);
                emitter_push_u8(emitter, 0x81);
                emitter_push_u8(emitter, 0xc3);
                emitter_push_u32(emitter, expr->value);
            }
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
        case ExprType_Add:
            if (!emitter->rax_contains_copy) {
                // movzx rax, BYTE [rbx]
                emitter_push_u8(emitter, 0x48);
                emitter_push_u8(emitter, 0x0f);
                emitter_push_u8(emitter, 0xb6);
                emitter_push_u8(emitter, 0x03);
                emitter->rax_contains_copy = true;
            }
            if (is_8(expr->value)) {
                // add BYTE [rbx + <value: rel8>], al
                emitter_push_u8(emitter, 0x00);
                emitter_push_u8(emitter, 0x43);
                emitter_push_u8(emitter, (uint8_t)expr->value);
            } else {
                // add BYTE [rbx + <value: rel32>], al
                emitter_push_u8(emitter, 0x00);
                emitter_push_u8(emitter, 0x83);
                emitter_push_u32(emitter, expr->value);
            }
            break;
    }
    if (expr->type != ExprType_Add) {
        emitter->rax_contains_copy = false;
    }
}

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
