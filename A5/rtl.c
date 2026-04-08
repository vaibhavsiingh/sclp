#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rtl.h"
#include "tac.h"
#include "symbol_table.h"

typedef struct StringMap
{
    char *literal;
    char *label;
    struct StringMap *next;
} StringMap;

static StringMap *string_map_head = NULL;
static int next_string_id = 0;

static void *checked_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Out of memory in RTL generator\n");
        exit(1);
    }
    return ptr;
}

static char *xstrdup(const char *s)
{
    size_t n;
    char *p;

    if (!s)
        s = "";

    n = strlen(s) + 1;
    p = (char *)checked_malloc(n);
    memcpy(p, s, n);
    return p;
}

static void trim(char *s)
{
    size_t len;
    size_t i = 0;

    if (!s)
        return;

    while (s[i] && isspace((unsigned char)s[i]))
        i++;
    if (i > 0)
        memmove(s, s + i, strlen(s + i) + 1);

    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
    {
        s[len - 1] = '\0';
        len--;
    }
}

static int is_integer_literal(const char *s)
{
    size_t i = 0;

    if (!s || !*s)
        return 0;

    if (s[i] == '+' || s[i] == '-')
        i++;

    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static int is_float_literal(const char *s)
{
    size_t i = 0;
    int seen_digit = 0;
    int seen_dot = 0;

    if (!s || !*s)
        return 0;

    if (s[i] == '+' || s[i] == '-')
        i++;

    for (; s[i] != '\0'; i++)
    {
        if (isdigit((unsigned char)s[i]))
        {
            seen_digit = 1;
            continue;
        }

        if (s[i] == '.')
        {
            if (seen_dot)
                return 0;
            seen_dot = 1;
            continue;
        }

        return 0;
    }

    return seen_digit && seen_dot;
}

static int is_float_reg(const char *reg)
{
    return reg && reg[0] == 'f';
}

static int is_string_literal(const char *s)
{
    size_t len;

    if (!s)
        return 0;

    len = strlen(s);
    return len >= 2 && s[0] == '"' && s[len - 1] == '"';
}

static const char *intern_string_label(const char *literal)
{
    // returns appropriate label for string literals
    StringMap *cur = string_map_head;
    char name[32];
    StringMap *node;

    while (cur)
    {
        if (strcmp(cur->literal, literal) == 0)
            return cur->label;
        cur = cur->next;
    }

    node = (StringMap *)checked_malloc(sizeof(StringMap));
    snprintf(name, sizeof(name), "_str_%d", next_string_id++);
    node->literal = xstrdup(literal);
    node->label = xstrdup(name);
    node->next = string_map_head;
    string_map_head = node;

    return node->label;
}

static void free_string_map(void)
{
    StringMap *cur = string_map_head;

    while (cur)
    {
        StringMap *next = cur->next;
        free(cur->literal);
        free(cur->label);
        free(cur);
        cur = next;
    }

    string_map_head = NULL;
}

static void rtl_print_label(Rtl *rtl, FILE *out)
{
    Rtl_Label *ins = (Rtl_Label *)rtl;
    fprintf(out, "\n%s:\n", ins->label);
}

static void rtl_print_op0(Rtl *rtl, FILE *out)
{
    Rtl_Op0 *ins = (Rtl_Op0 *)rtl;
    fprintf(out, "    %s\n", ins->op);
}

static void rtl_print_op2(Rtl *rtl, FILE *out)
{
    Rtl_Op2 *ins = (Rtl_Op2 *)rtl;
    char op_buf[32];

    snprintf(op_buf, sizeof(op_buf), "%s:", ins->op ? ins->op : "");
    fprintf(out, "    %-7s %s <- %s", op_buf, ins->dst, ins->src);
    // if (ins->comment && *ins->comment)
    //     fprintf(out, "        ;; %s", ins->comment);
    fprintf(out, "\n");
}

static void rtl_print_op3(Rtl *rtl, FILE *out)
{
    Rtl_Op3 *ins = (Rtl_Op3 *)rtl;

    fprintf(out, "    %s:    %s <- %s, %s", ins->op, ins->dst, ins->src1, ins->src2);
    if (ins->comment && *ins->comment)
        fprintf(out, "        ;; %s", ins->comment);
    fprintf(out, "\n");
}

static void rtl_print_op2_comma(Rtl *rtl, FILE *out)
{
    Rtl_Op2Comma *ins = (Rtl_Op2Comma *)rtl;
    fprintf(out, "    %s:    %s, %s\n", ins->op, ins->src1, ins->src2);
}

static void rtl_print_goto(Rtl *rtl, FILE *out)
{
    Rtl_Goto *ins = (Rtl_Goto *)rtl;
    fprintf(out, "    goto:   %s\n", ins->label);
}

static void rtl_print_bgtz(Rtl *rtl, FILE *out)
{
    Rtl_Bgtz *ins = (Rtl_Bgtz *)rtl;
    fprintf(out, "    bgtz:   %s, %s\n", ins->reg, ins->label);
}

static Rtl *rtl_alloc(size_t size, Rtl_Kind kind, Rtl_Print_Fn print_fn)
{
    Rtl *instr = (Rtl *)checked_malloc(size);
    instr->kind = kind;
    instr->print = print_fn;
    instr->next = NULL;
    return instr;
}

Rtl_Seq *rtl_seq_create(void)
{
    Rtl_Seq *seq = (Rtl_Seq *)checked_malloc(sizeof(Rtl_Seq));
    seq->head = NULL;
    seq->tail = NULL;
    return seq;
}

void rtl_seq_append(Rtl_Seq *seq, Rtl *instr)
{
    if (!seq || !instr)
        return;

    instr->next = NULL;
    if (!seq->head)
        seq->head = instr;
    else
        seq->tail->next = instr;
    seq->tail = instr;
}

void rtl_seq_print(const Rtl_Seq *seq, FILE *out)
{
    Rtl *cur;

    if (!seq || !out)
        return;

    for (cur = seq->head; cur; cur = cur->next)
    {
        if (cur->print)
            cur->print(cur, out);
    }
}

Rtl *rtl_make_label(const char *label)
{
    Rtl_Label *ins = (Rtl_Label *)rtl_alloc(sizeof(Rtl_Label), RTL_LABEL, rtl_print_label);
    ins->label = xstrdup(label);
    return (Rtl *)ins;
}

Rtl *rtl_make_op0(const char *op)
{
    Rtl_Op0 *ins = (Rtl_Op0 *)rtl_alloc(sizeof(Rtl_Op0), RTL_OP0, rtl_print_op0);
    ins->op = xstrdup(op);
    return (Rtl *)ins;
}

Rtl *rtl_make_op2(const char *op, const char *dst, const char *src, const char *comment)
{
    Rtl_Op2 *ins = (Rtl_Op2 *)rtl_alloc(sizeof(Rtl_Op2), RTL_OP2, rtl_print_op2);
    ins->op = xstrdup(op);
    ins->dst = xstrdup(dst);
    ins->src = xstrdup(src);
    ins->comment = comment ? xstrdup(comment) : NULL;
    return (Rtl *)ins;
}

Rtl *rtl_make_op3(const char *op, const char *dst, const char *src1, const char *src2, const char *comment)
{
    Rtl_Op3 *ins = (Rtl_Op3 *)rtl_alloc(sizeof(Rtl_Op3), RTL_OP3, rtl_print_op3);
    ins->op = xstrdup(op);
    ins->dst = xstrdup(dst);
    ins->src1 = xstrdup(src1);
    ins->src2 = xstrdup(src2);
    ins->comment = comment ? xstrdup(comment) : NULL;
    return (Rtl *)ins;
}

Rtl *rtl_make_op2_comma(const char *op, const char *src1, const char *src2)
{
    Rtl_Op2Comma *ins = (Rtl_Op2Comma *)rtl_alloc(sizeof(Rtl_Op2Comma), RTL_OP2_COMMA, rtl_print_op2_comma);
    ins->op = xstrdup(op);
    ins->src1 = xstrdup(src1);
    ins->src2 = xstrdup(src2);
    return (Rtl *)ins;
}

Rtl *rtl_make_goto(const char *label)
{
    Rtl_Goto *ins = (Rtl_Goto *)rtl_alloc(sizeof(Rtl_Goto), RTL_GOTO, rtl_print_goto);
    ins->label = xstrdup(label);
    return (Rtl *)ins;
}

Rtl *rtl_make_bgtz(const char *reg, const char *label)
{
    Rtl_Bgtz *ins = (Rtl_Bgtz *)rtl_alloc(sizeof(Rtl_Bgtz), RTL_BGTZ, rtl_print_bgtz);
    ins->reg = xstrdup(reg);
    ins->label = xstrdup(label);
    return (Rtl *)ins;
}

static void rtl_free(Rtl *rtl)
{
    if (!rtl)
        return;

    switch (rtl->kind)
    {
    case RTL_LABEL:
        free(((Rtl_Label *)rtl)->label);
        break;
    case RTL_OP0:
        free(((Rtl_Op0 *)rtl)->op);
        break;
    case RTL_OP2:
    {
        Rtl_Op2 *ins = (Rtl_Op2 *)rtl;
        free(ins->op);
        free(ins->dst);
        free(ins->src);
        free(ins->comment);
        break;
    }
    case RTL_OP3:
    {
        Rtl_Op3 *ins = (Rtl_Op3 *)rtl;
        free(ins->op);
        free(ins->dst);
        free(ins->src1);
        free(ins->src2);
        free(ins->comment);
        break;
    }
    case RTL_OP2_COMMA:
    {
        Rtl_Op2Comma *ins = (Rtl_Op2Comma *)rtl;
        free(ins->op);
        free(ins->src1);
        free(ins->src2);
        break;
    }
    case RTL_GOTO:
        free(((Rtl_Goto *)rtl)->label);
        break;
    case RTL_BGTZ:
    {
        Rtl_Bgtz *ins = (Rtl_Bgtz *)rtl;
        free(ins->reg);
        free(ins->label);
        break;
    }
    default:
        break;
    }

    free(rtl);
}

static void rtl_seq_free(Rtl_Seq *seq)
{
    Rtl *cur;
    Rtl *next;

    if (!seq)
        return;

    cur = seq->head;
    while (cur)
    {
        next = cur->next;
        rtl_free(cur);
        cur = next;
    }

    free(seq);
}

typedef struct TempRegMap
{
    char name[32];
    char reg[8];
    Data_Type type;
    struct TempRegMap *next;
} TempRegMap;

typedef struct
{
    TempRegMap *head;
    char procedure_name[128];
    int has_procedure_name;
} RtlState;

// s* hote h kya regs? check krne honge
static const char *k_int_temp_regs[] = {
    "v0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"};

static const char *k_float_temp_regs[] = {
    "f2", "f4", "f6", "f8", "f10", "f12", "f14", "f16", "f18", "f20", "f22", "f24", "f26", "f28", "f30"};

static int is_temp_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strcmp(s, "extra") == 0)
        return 1;

    if (strncmp(s, "temp", 4) != 0)
        return 0;

    i = 4;
    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static int is_saved_temp_name(const char *s)
{
    size_t i = 0;

    if (!s)
        return 0;

    if (strncmp(s, "stemp", 5) != 0)
        return 0;

    i = 5;
    if (!isdigit((unsigned char)s[i]))
        return 0;

    for (; s[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    return 1;
}

static const char *lookup_temp_reg(RtlState *state, const char *name)
{
    TempRegMap *cur;

    if (!state || !name)
        return NULL;

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return cur->reg;
        cur = cur->next;
    }

    return NULL;
}

static Data_Type lookup_temp_type(RtlState *state, const char *name)
{
    TempRegMap *cur;

    if (!state || !name)
        return INT_TYPE;

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return cur->type;
        cur = cur->next;
    }
    printf("ERROR: type not found for temp: %s\n", name);
    return INT_TYPE;
}

static int reg_in_use(RtlState *state, const char *reg)
{
    TempRegMap *cur;

    if (!state || !reg)
        return 0;

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->reg, reg) == 0)
            return 1;
        cur = cur->next;
    }

    return 0;
}

static int reg_is_excluded(const char *reg, const char *a, const char *b, const char *c)
{
    if (!reg)
        return 0;
    if (a && strcmp(reg, a) == 0)
        return 1;
    if (b && strcmp(reg, b) == 0)
        return 1;
    if (c && strcmp(reg, c) == 0)
        return 1;
    return 0;
}

static Data_Type reg_type_for(Data_Type type)
{
    return (type == FLOAT_TYPE) ? FLOAT_TYPE : INT_TYPE;
}

static const char *pick_free_reg(RtlState *state, Data_Type type, const char *exclude1, const char *exclude2, const char *exclude3)
{
    // returns a register that is not in state and is not eqeual any of the excludes
    const char **pool = k_int_temp_regs;
    size_t count = sizeof(k_int_temp_regs) / sizeof(k_int_temp_regs[0]);
    size_t i;

    if (reg_type_for(type) == FLOAT_TYPE)
    {
        pool = k_float_temp_regs;
        count = sizeof(k_float_temp_regs) / sizeof(k_float_temp_regs[0]);
    }

    for (i = 0; i < count; i++)
    {
        const char *reg = pool[i];
        if (reg_is_excluded(reg, exclude1, exclude2, exclude3))
            continue;
        if (!reg_in_use(state, reg))
            return reg;
    }

    return pool[0];
}

static const char *assign_temp_reg(RtlState *state, const char *name, Data_Type type, const char *exclude1, const char *exclude2, const char *exclude3)
{
    // picks a free reg and add that to the state
    TempRegMap *node;
    const char *reg_name;

    if (!state || !name)
    {
        printf("ERROR: Register asked but either state dne or name dne");
        return "v0";
    }

    if (lookup_temp_reg(state, name))
        return lookup_temp_reg(state, name);

    reg_name = pick_free_reg(state, type, exclude1, exclude2, exclude3);

    node = (TempRegMap *)checked_malloc(sizeof(TempRegMap));
    snprintf(node->name, sizeof(node->name), "%s", name);
    snprintf(node->reg, sizeof(node->reg), "%s", reg_name);
    node->type = reg_type_for(type);
    node->next = state->head;
    state->head = node;
    return node->reg;
}

static void clear_all_temp_regs(RtlState *state)
{
    TempRegMap *cur;

    if (!state)
        return;

    cur = state->head;
    while (cur)
    {
        TempRegMap *next = cur->next;
        free(cur);
        cur = next;
    }

    state->head = NULL;
}

static void remove_temp_reg(RtlState *state, const char *name)
{
    TempRegMap *cur;
    TempRegMap *prev = NULL;

    if (!state || !name)
        return;

    cur = state->head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
        {
            if (prev)
                prev->next = cur->next;
            else
                state->head = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static void release_temp_if_used(RtlState *state, const char *name)
{
    // if a tac temp was used, pass the temp in this function, and this will free the corresponding reg
    if (!state || !is_temp_name(name))
        return;

    remove_temp_reg(state, name);
}

static Data_Type lookup_symbol_type_safe(const char *name)
{
    Symbol_Table_Entry *entry;

    if (!name)
        return INT_TYPE;

    if (is_saved_temp_name(name))
        return get_operand_type((char *)name);

    entry = lookup_symbol((char *)name);
    if (!entry)
    {
        printf("Symbol table entry not found of %s in lookup_symbol_type_safe\n", name);
        return INT_TYPE;
    }
    return entry->type;
}

static Data_Type infer_operand_type(const char *operand, RtlState *state)
{
    printf("%s\n", operand);
    if (!operand)
        return INT_TYPE;
    if (is_integer_literal(operand))
        return INT_TYPE;
    if (is_float_literal(operand))
        return FLOAT_TYPE;
    if (is_string_literal(operand))
        return STRING_TYPE;
    if (is_temp_name(operand))
        return lookup_temp_type(state, operand);
    if (is_saved_temp_name(operand))
        return get_operand_type((char *)operand);

    return lookup_symbol_type_safe(operand);
}

static void emit_load_operand(const char *reg, const char *operand, Rtl_Seq *out)
{
    if (is_string_literal(operand))
    {
        const char *label = intern_string_label(operand);
        rtl_seq_append(out, rtl_make_op2("load_addr", reg, label, NULL));
        return;
    }
    if (is_integer_literal(operand))
        rtl_seq_append(out, rtl_make_op2("iLoad", reg, operand, NULL));
    else if (is_float_literal(operand))
        rtl_seq_append(out, rtl_make_op2("iLoad.d", reg, operand, NULL));
    else if (is_float_reg(reg))
        rtl_seq_append(out, rtl_make_op2("load.d", reg, operand, NULL));
    else
        rtl_seq_append(out, rtl_make_op2("load", reg, operand, NULL));
}

static void emit_store_symbol(const char *symbol, const char *reg, Rtl_Seq *out)
{
    if (is_float_reg(reg))
        rtl_seq_append(out, rtl_make_op2("store.d", symbol, reg, NULL));
    else
        rtl_seq_append(out, rtl_make_op2("store", symbol, reg, NULL));
}

static void emit_move_value(const char *dst_reg, const char *src_reg, Rtl_Seq *out)
{
    if (is_float_reg(dst_reg) || is_float_reg(src_reg))
        rtl_seq_append(out, rtl_make_op2("move.d", dst_reg, src_reg, NULL));
    else
        rtl_seq_append(out, rtl_make_op2("move", dst_reg, src_reg, NULL));
}

static const char *emit_operand_reg(const char *operand, Data_Type expected_type, RtlState *state, Rtl_Seq *out, const char *exclude1, const char *exclude2, const char *exclude3)
{
    const char *mapped = NULL;
    const char *reg = NULL;

    if (is_temp_name(operand))
    {
        mapped = lookup_temp_reg(state, operand);
        if (mapped)
            return mapped;
    }

    reg = pick_free_reg(state, expected_type, exclude1, exclude2, exclude3);
    emit_load_operand(reg, operand, out);
    return reg;
}

static const char *arith_to_rtl(const char *op, int is_float)
{
    if (strcmp(op, "+") == 0)
        return is_float ? "add.d" : "add";
    if (strcmp(op, "-") == 0)
        return is_float ? "sub.d" : "sub";
    if (strcmp(op, "*") == 0)
        return is_float ? "mul.d" : "mul";
    if (strcmp(op, "/") == 0)
        return is_float ? "div.d" : "div";
    if (strcmp(op, "&&") == 0)
        return "and";
    if (strcmp(op, "||") == 0)
        return "or";
    return NULL;
}

static const char *relop_to_rtl(const char *op, int is_float)
{
    if (strcmp(op, "<") == 0)
        return is_float ? "slt.d" : "slt";
    if (strcmp(op, "<=") == 0)
        return is_float ? "sle.d" : "sle";
    if (strcmp(op, ">") == 0)
        return is_float ? "sle.d" : "sgt";
    if (strcmp(op, ">=") == 0)
        return is_float ? "slt.d" : "sge";
    if (strcmp(op, "==") == 0)
        return is_float ? "seq.d" : "seq";
    if (strcmp(op, "!=") == 0)
        return is_float ? "seq.d" : "sne";
    return NULL;
}

static int float_relop_uses_movf(const char *op)
{
    return (strcmp(op, ">") == 0 || strcmp(op, ">=") == 0 || strcmp(op, "!=") == 0);
}

// static void emit_float_relop_assign(const char *dst_reg, const char *lhs_reg,
//                                     const char *op, const char *rhs_reg,
//                                     RtlState *state, Rtl_Seq *out)
// {
//     const char *cmp_op = relop_to_rtl(op, 1);
//     int use_movf = float_relop_uses_movf(op);
//     const char *scratch = pick_free_reg(state, INT_TYPE, dst_reg, lhs_reg, rhs_reg);

//     rtl_seq_append(out, rtl_make_op2_comma(cmp_op, lhs_reg, rhs_reg));
//     rtl_seq_append(out, rtl_make_op2("iLoad", scratch, "1", NULL));
//     rtl_seq_append(out, rtl_make_op2("move", dst_reg, "zero", NULL));
//     rtl_seq_append(out, rtl_make_op3(use_movf ? "movf" : "movt", dst_reg, scratch, "0", NULL));
// }

static void emit_float_relop_assign(const char *dst, const char *lhs, const char *op, const char *rhs, RtlState *state, Rtl_Seq *out)
{
    const char *rtl_op = relop_to_rtl(op, 1);
    int use_movf = float_relop_uses_movf(op);
    const char *extra_reg = assign_temp_reg(state, "extra", INT_TYPE, NULL, NULL, NULL);

    const char *lhs_reg = emit_operand_reg(lhs, FLOAT_TYPE, state, out, extra_reg, NULL, NULL);
    const char *dst_reg;
    if (is_temp_name(dst))
    {
        dst_reg = assign_temp_reg(state, dst, INT_TYPE, lhs_reg, extra_reg, NULL);
    }
    else
    {
        dst_reg = pick_free_reg(state, INT_TYPE, lhs_reg, extra_reg, NULL);
    }
    const char *rhs_reg = emit_operand_reg(rhs, FLOAT_TYPE, state, out, lhs_reg, dst_reg, extra_reg);

    rtl_seq_append(out, rtl_make_op2_comma(rtl_op, lhs_reg, rhs_reg));
    rtl_seq_append(out, rtl_make_op2("iLoad", extra_reg, "1", NULL));
    rtl_seq_append(out, rtl_make_op2("move", dst_reg, "zero", NULL));
    rtl_seq_append(out, rtl_make_op3(use_movf ? "movf" : "movt", dst_reg, extra_reg, "0", NULL));

    release_temp_if_used(state, "extra");
    if (strcmp(dst, lhs) != 0)
        release_temp_if_used(state, lhs);
    if (strcmp(dst, rhs) != 0)
        release_temp_if_used(state, rhs);
}

static void emit_print(const char *value, RtlState *state, Rtl_Seq *out)
{
    char local[384];
    const char *mapped_temp = NULL;

    if (!value)
        return;

    snprintf(local, sizeof(local), "%s", value);
    trim(local);

    if (is_string_literal(local))
    {
        const char *label = intern_string_label(local);
        rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "4", "syscall: print string"));
        rtl_seq_append(out, rtl_make_op2("load_addr", "a0", label, NULL));
        rtl_seq_append(out, rtl_make_op0("write"));
        return;
    }

    Data_Type value_type;
    value_type = infer_operand_type(local, state);
    if (value_type == STRING_TYPE)
    {
        rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "4", "syscall: print string"));
        if (is_temp_name(local) && (mapped_temp = lookup_temp_reg(state, local)) != NULL)
            rtl_seq_append(out, rtl_make_op2("move", "a0", mapped_temp, NULL));
        else
            rtl_seq_append(out, rtl_make_op2("load", "a0", local, NULL));
        rtl_seq_append(out, rtl_make_op0("write"));
        return;
    }
    if (value_type == FLOAT_TYPE)
    {
        rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "3", "syscall: print float"));
        if (is_float_literal(local))
            rtl_seq_append(out, rtl_make_op2("iLoad.d", "f12", local, NULL));
        else if (is_temp_name(local) && (mapped_temp = lookup_temp_reg(state, local)) != NULL)
            rtl_seq_append(out, rtl_make_op2("move.d", "f12", mapped_temp, NULL));
        else
            rtl_seq_append(out, rtl_make_op2("load.d", "f12", local, NULL));
        rtl_seq_append(out, rtl_make_op0("write"));
        return;
    }

    rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "1", "syscall: print integer"));
    if (is_integer_literal(local))
        rtl_seq_append(out, rtl_make_op2("iLoad", "a0", local, NULL));
    else if (is_temp_name(local) && (mapped_temp = lookup_temp_reg(state, local)) != NULL)
        rtl_seq_append(out, rtl_make_op2("move", "a0", mapped_temp, NULL));
    else
        rtl_seq_append(out, rtl_make_op2("load", "a0", local, NULL));
    rtl_seq_append(out, rtl_make_op0("write"));
}

static void emit_copy_assign(const char *dst, const char *rhs, RtlState *state, Rtl_Seq *out)
{
    Data_Type rhs_type = infer_operand_type(rhs, state);
    Data_Type dst_type = infer_operand_type(dst, state);
    printf("%d", (int)rhs_type);
    const char *rhs_reg = emit_operand_reg(rhs, rhs_type, state, out, NULL, NULL, NULL);

    if (is_temp_name(dst))
    {
        Data_Type temp_type = (rhs_type == FLOAT_TYPE || dst_type == FLOAT_TYPE) ? FLOAT_TYPE : rhs_type;
        const char *dst_reg = assign_temp_reg(state, dst, temp_type, rhs_reg, NULL, NULL);

        if (strcmp(dst_reg, rhs_reg) != 0)
            emit_move_value(dst_reg, rhs_reg, out);
        if (strcmp(dst, rhs) != 0)
            release_temp_if_used(state, rhs);
        return;
    }

    emit_store_symbol(dst, rhs_reg, out);
    release_temp_if_used(state, rhs);
    clear_all_temp_regs(state);
}

static void emit_unary_assign(const char *dst, const char *op, const char *rhs, RtlState *state, Rtl_Seq *out)
{
    Data_Type rhs_type = infer_operand_type(rhs, state);
    int is_float_unary = (rhs_type == FLOAT_TYPE && strcmp(op, "-") == 0);
    const char *src_reg = emit_operand_reg(rhs, rhs_type, state, out, NULL, NULL, NULL);
    const char *dst_reg;

    if (is_temp_name(dst))
    {
        dst_reg = assign_temp_reg(state, dst, is_float_unary ? FLOAT_TYPE : INT_TYPE, src_reg, NULL, NULL);
    }
    else
    {
        dst_reg = pick_free_reg(state, is_float_unary ? FLOAT_TYPE : INT_TYPE, src_reg, NULL, NULL);
    }

    if (strcmp(op, "-") == 0)
        rtl_seq_append(out, rtl_make_op2(is_float_unary ? "uminus.d" : "uminus", dst_reg, src_reg, NULL));
    else
        rtl_seq_append(out, rtl_make_op2("not", dst_reg, src_reg, NULL));

    if (strcmp(dst, rhs) != 0)
        release_temp_if_used(state, rhs);

    if (!is_temp_name(dst))
    {
        emit_store_symbol(dst, dst_reg, out);
        clear_all_temp_regs(state);
    }
}

static void emit_binary_assign(const char *dst, const char *lhs, const char *op, const char *rhs, RtlState *state, Rtl_Seq *out)
{
    Data_Type lhs_type = infer_operand_type(lhs, state);
    Data_Type rhs_type = infer_operand_type(rhs, state);
    int is_relop = (strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">") == 0 || strcmp(op, ">=") == 0 || strcmp(op, "==") == 0 || strcmp(op, "!=") == 0);
    int is_logical = (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0);
    int use_float_op = (!is_logical && (lhs_type == FLOAT_TYPE || rhs_type == FLOAT_TYPE));
    Data_Type result_type = is_relop ? BOOL_TYPE : (use_float_op ? FLOAT_TYPE : INT_TYPE);
    const char *rtl_op = arith_to_rtl(op, use_float_op);
    const char *lhs_reg;
    const char *rhs_reg;
    const char *dst_reg;

    if (is_relop && lhs_type == FLOAT_TYPE)
    {
        emit_float_relop_assign(dst, lhs, op, rhs, state, out);
        return;
    }

    if (!rtl_op)
        rtl_op = relop_to_rtl(op, use_float_op);

    if (!rtl_op)
    {
        emit_copy_assign(dst, rhs, state, out);
        return;
    }

    lhs_reg = emit_operand_reg(lhs, lhs_type, state, out, NULL, NULL, NULL);

    if (is_temp_name(dst))
    {
        dst_reg = assign_temp_reg(state, dst, (result_type == FLOAT_TYPE) ? FLOAT_TYPE : INT_TYPE, lhs_reg, NULL, NULL);
    }
    else
    {
        dst_reg = pick_free_reg(state, (result_type == FLOAT_TYPE) ? FLOAT_TYPE : INT_TYPE, lhs_reg, NULL, NULL);
    }

    rhs_reg = emit_operand_reg(rhs, rhs_type, state, out, lhs_reg, dst_reg, NULL);

    if (!is_relop || !use_float_op)
        rtl_seq_append(out, rtl_make_op3(rtl_op, dst_reg, lhs_reg, rhs_reg, NULL));

    if (strcmp(dst, lhs) != 0)
        release_temp_if_used(state, lhs);
    if (strcmp(dst, rhs) != 0)
        release_temp_if_used(state, rhs);

    if (!is_temp_name(dst))
    {
        emit_store_symbol(dst, dst_reg, out);
        clear_all_temp_regs(state);
    }
}

static void emit_read(const char *dst, RtlState *state, Rtl_Seq *out)
{
    Data_Type dst_type = lookup_symbol_type_safe(dst);

    if (dst_type == FLOAT_TYPE)
    {
        rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "6", "syscall: read float"));
        rtl_seq_append(out, rtl_make_op0("read"));
        rtl_seq_append(out, rtl_make_op2("store.d", dst, "f0", NULL));
    }
    else
    {
        rtl_seq_append(out, rtl_make_op2("iLoad", "v0", "5", "syscall: read integer"));
        rtl_seq_append(out, rtl_make_op0("read"));
        rtl_seq_append(out, rtl_make_op2("store", dst, "v0", NULL));
    }

    clear_all_temp_regs(state);
}

static void emit_if_goto(const char *cond, const char *label, RtlState *state, Rtl_Seq *out)
{
    const char *cond_reg = emit_operand_reg(cond, INT_TYPE, state, out, NULL, NULL, NULL);
    rtl_seq_append(out, rtl_make_bgtz(cond_reg, label));
    release_temp_if_used(state, cond);
}

static void ensure_tac(Ast *root)
{
    FILE *fp;

    if (!root || root->tac_code)
        return;

    fp = tmpfile();
    if (!fp)
        fp = fopen("/dev/null", "w");
    if (!fp)
        return;

    tac_generate(root, fp);
    fclose(fp);
}

static void emit_tac_seq(const Tac_Seq *seq, RtlState *state, Rtl_Seq *out)
{
    const Tac *cur;

    if (!seq || !state || !out)
        return;

    for (cur = seq->head; cur; cur = cur->next)
    {
        switch (cur->kind)
        {
        case TAC_PROC_BEGIN:
        {
            const Tac_Proc *ins = (const Tac_Proc *)cur;
            snprintf(state->procedure_name, sizeof(state->procedure_name), "%s", ins->name ? ins->name : "");
            state->has_procedure_name = 1;
            clear_all_temp_regs(state);
            break;
        }
        case TAC_PROC_END:
            clear_all_temp_regs(state);
            break;
        case TAC_LABEL:
        {
            const Tac_Label *ins = (const Tac_Label *)cur;
            rtl_seq_append(out, rtl_make_label(ins->label));
            break;
        }
        case TAC_GOTO:
        {
            const Tac_Goto *ins = (const Tac_Goto *)cur;
            rtl_seq_append(out, rtl_make_goto(ins->label));
            break;
        }
        case TAC_IF_GOTO:
        {
            const Tac_If_Goto *ins = (const Tac_If_Goto *)cur;
            emit_if_goto(ins->cond, ins->label, state, out);
            break;
        }
        case TAC_READ:
        {
            const Tac_Read *ins = (const Tac_Read *)cur;
            emit_read(ins->target, state, out);
            break;
        }
        case TAC_PRINT:
        {
            const Tac_Print *ins = (const Tac_Print *)cur;
            emit_print(ins->value, state, out);
            release_temp_if_used(state, ins->value);
            break;
        }
        case TAC_ASSIGN:
        {
            const Tac_Assign *ins = (const Tac_Assign *)cur;
            emit_copy_assign(ins->dest, ins->src, state, out);
            break;
        }
        case TAC_UNARY:
        {
            const Tac_Unary *ins = (const Tac_Unary *)cur;
            emit_unary_assign(ins->dest, ins->op, ins->operand, state, out);
            break;
        }
        case TAC_BINARY:
        {
            const Tac_Binary *ins = (const Tac_Binary *)cur;
            emit_binary_assign(ins->dest, ins->lhs, ins->op, ins->rhs, state, out);
            break;
        }
        case TAC_RETURN:
            rtl_seq_append(out, rtl_make_op0("return"));
            clear_all_temp_regs(state);
            break;
        default:
            break;
        }
    }
}

static Rtl_Seq *tac_to_rtl_seq(Ast *root, char *procedure_name, size_t procedure_name_size)
{
    Rtl_Seq *seq;
    RtlState state = {NULL, "", 0};

    if (!root)
    {
        if (procedure_name && procedure_name_size > 0)
            procedure_name[0] = '\0';
        return rtl_seq_create();
    }

    ensure_tac(root);

    seq = rtl_seq_create();
    if (root->tac_code)
        emit_tac_seq(root->tac_code, &state, seq);

    if (procedure_name && procedure_name_size > 0)
    {
        if (state.has_procedure_name)
            snprintf(procedure_name, procedure_name_size, "%s", state.procedure_name);
        else
            procedure_name[0] = '\0';
    }

    clear_all_temp_regs(&state);
    return seq;
}

void rtl_reset_counters(void)
{
    next_string_id = 0;
    free_string_map();
}

void rtl_generate(Ast *root, FILE *out)
{
    char procedure_name[128] = "";
    Rtl_Seq *seq;

    if (!out)
        return;

    rtl_reset_counters();
    seq = tac_to_rtl_seq(root, procedure_name, sizeof(procedure_name));

    if (!seq || !seq->head)
    {
        if (seq)
            rtl_seq_free(seq);
        return;
    }

    if (procedure_name[0] != '\0')
    {
        fputs("**PROCEDURE: ", out);
        fputs(procedure_name, out);
        fputs("\n", out);
    }

    fputs("**BEGIN: RTL Statements\n", out);
    rtl_seq_print(seq, out);
    fputs("**END: RTL Statements\n", out);

    rtl_seq_free(seq);
}

int rtl_generate_to_path(Ast *root, const char *path)
{
    FILE *fp;

    if (!path)
        return 0;

    fp = fopen(path, "w");
    if (!fp)
        return 0;

    rtl_generate(root, fp);
    fclose(fp);
    return 1;
}