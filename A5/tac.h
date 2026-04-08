#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include "ast.h"

typedef enum
{
    TAC_ASSIGN,
    TAC_BINARY,
    TAC_UNARY,
    TAC_LABEL,
    TAC_GOTO,
    TAC_IF_GOTO,
    TAC_READ,
    TAC_PRINT,
    TAC_RETURN,
    TAC_PROC_BEGIN,
    TAC_PROC_END,
    TAC_BLANK
} Tac_Kind;

typedef struct Tac Tac;
typedef void (*Tac_Print_Fn)(Tac *tac, FILE *file);

struct Tac
{
    Tac_Kind kind;
    Tac_Print_Fn print;
    Tac *next;
};

struct Tac_Seq
{
    Tac *head;
    Tac *tail;
};

typedef struct
{
    Tac base;
    char *dest;
    char *src;
} Tac_Assign;

typedef struct
{
    Tac base;
    char *dest;
    char *op;
    char *lhs;
    char *rhs;
} Tac_Binary;

typedef struct
{
    Tac base;
    char *dest;
    char *op;
    char *operand;
} Tac_Unary;

typedef struct
{
    Tac base;
    char *label;
} Tac_Label;

typedef struct
{
    Tac base;
    char *label;
} Tac_Goto;

typedef struct
{
    Tac base;
    char *cond;
    char *label;
} Tac_If_Goto;

typedef struct
{
    Tac base;
    char *target;
} Tac_Read;

typedef struct
{
    Tac base;
    char *value;
} Tac_Print;

typedef struct
{
    Tac base;
    char *name;
} Tac_Proc;

typedef struct
{
    Tac base;
} Tac_Return;

typedef struct
{
    Tac base;
} Tac_Blank;

Tac_Seq *tac_seq_create(void);
void tac_seq_append(Tac_Seq *seq, Tac *instr);
void tac_seq_extend(Tac_Seq *seq, const Tac_Seq *other);
void tac_seq_print(const Tac_Seq *seq, FILE *out);

Tac *tac_make_assign(const char *dest, const char *src);
Tac *tac_make_binary(const char *dest, const char *lhs, const char *op, const char *rhs);
Tac *tac_make_unary(const char *dest, const char *op, const char *operand);
Tac *tac_make_label(const char *label);
Tac *tac_make_goto(const char *label);
Tac *tac_make_if_goto(const char *cond, const char *label);
Tac *tac_make_read(const char *target);
Tac *tac_make_print(const char *value);
Tac *tac_make_return(void);
Tac *tac_make_proc_begin(const char *name);
Tac *tac_make_proc_end(const char *name);
Tac *tac_make_blank(void);

/* Generate TAC for a checked AST and write it to the provided stream. */
void tac_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes TAC to a file path. Returns 1 on success. */
int tac_generate_to_path(Ast *root, const char *path);

/* Reset temporary and label counters for deterministic output across runs. */
void tac_reset_counters(void);

Data_Type get_operand_type(char *name);

#endif
