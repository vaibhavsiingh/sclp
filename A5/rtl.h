#ifndef RTL_H
#define RTL_H

#include <stdio.h>
#include "ast.h"

typedef enum
{
    RTL_LABEL,
    RTL_OP0,
    RTL_OP1,
    RTL_OP2,
    RTL_OP3,
    RTL_OP2_COMMA,
    RTL_CALL,
    RTL_GOTO,
    RTL_BGTZ
} Rtl_Kind;

typedef struct Rtl Rtl;
typedef struct Rtl_Seq Rtl_Seq;
typedef void (*Rtl_Print_Fn)(Rtl *rtl, FILE *file);

struct Rtl
{
    Rtl_Kind kind;
    Rtl_Print_Fn print;
    Rtl *next;
};

struct Rtl_Seq
{
    Rtl *head;
    Rtl *tail;
};

typedef struct
{
    Rtl base;
    char *label;
} Rtl_Label;

typedef struct
{
    Rtl base;
    char *op;
} Rtl_Op0;

typedef struct
{
    Rtl base;
    char *op;
    char *src;
} Rtl_Op1;

typedef struct
{
    Rtl base;
    char *op;
    char *dst;
    char *src;
    char *comment;
} Rtl_Op2;

typedef struct
{
    Rtl base;
    char *op;
    char *dst;
    char *src1;
    char *src2;
    char *comment;
} Rtl_Op3;

typedef struct
{
    Rtl base;
    char *op;
    char *src1;
    char *src2;
} Rtl_Op2Comma;

typedef struct
{
    Rtl base;
    char *dst;
    char *name;
} Rtl_Call;

typedef struct
{
    Rtl base;
    char *label;
} Rtl_Goto;

typedef struct
{
    Rtl base;
    char *reg;
    char *label;
} Rtl_Bgtz;

Rtl_Seq *rtl_seq_create(void);
void rtl_seq_append(Rtl_Seq *seq, Rtl *instr);
void rtl_seq_print(const Rtl_Seq *seq, FILE *out);

Rtl *rtl_make_label(const char *label);
Rtl *rtl_make_op0(const char *op);
Rtl *rtl_make_op1(const char *op, const char *src);
Rtl *rtl_make_op2(const char *op, const char *dst, const char *src, const char *comment);
Rtl *rtl_make_op3(const char *op, const char *dst, const char *src1, const char *src2, const char *comment);
Rtl *rtl_make_op2_comma(const char *op, const char *src1, const char *src2);
Rtl *rtl_make_call(const char *dst, const char *name);
Rtl *rtl_make_goto(const char *label);
Rtl *rtl_make_bgtz(const char *reg, const char *label);

/* Generate RTL for a checked AST and write it to the provided stream. */
void rtl_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes RTL to a file path. Returns 1 on success. */
int rtl_generate_to_path(Ast *root, const char *path);

/* Reset virtual register and label counters for deterministic output. */
void rtl_reset_counters(void);

/* Internal helpers used by spim.c */
typedef struct
{
    char *name;
    Rtl_Seq *seq;
} Proc_Rtl_Block;

Proc_Rtl_Block *rtl_collect_proc_blocks(Ast *root, int *out_count);
void rtl_free_proc_blocks(Proc_Rtl_Block *blocks, int block_count);
Data_Type lookup_symbol_type_safe(const char *name);

#endif
