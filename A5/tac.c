#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tac.h"
#include "symbol_table.h"

static int nextTempNumber = 0;
static int nextSTempNumber = 0;
static int nextLabelNumber = 0;
static FILE *tac_out = NULL;

typedef struct TempDt
{
    char *name;
    char *proc_name;
    Data_Type type;
    struct TempDt *next;
} TempDt;

static TempDt *temp_map_head = NULL;
static TempDt *temp_map_tail = NULL;
static const char *current_proc_context = NULL;

static char *current_return_label = NULL;
static char *current_return_temp = NULL;

static void *checked_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }
    return ptr;
}

static void *checked_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }
    return new_ptr;
}

static char *xstrdup(const char *s)
{
    size_t n;
    char *p;

    if (!s)
    {
        printf("ERROR: input of xstrdup DNE\n");
        s = "";
    }

    n = strlen(s) + 1;
    p = (char *)checked_malloc(n);
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
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

static void register_temp_type(char *name, Data_Type type)
{
    TempDt *node = (TempDt *)checked_malloc(sizeof(TempDt));
    if (!temp_map_tail)
    {
        temp_map_head = node;
        temp_map_tail = node;
    }
    else
    {
        temp_map_tail->next = node;
        temp_map_tail = node;
    }

    // printf("type of %s = %d\n", name, (int)type);

    node->name = xstrdup(name);
    node->proc_name = current_proc_context ? xstrdup(current_proc_context) : NULL;
    node->type = type;
    node->next = NULL;
}

static void free_temp_map(void)
{
    TempDt *cur = temp_map_head;

    while (cur)
    {
        TempDt *next = cur->next;
        free(cur->name);
        if (cur->proc_name)
            free(cur->proc_name);
        free(cur);
        cur = next;
    }

    temp_map_head = NULL;
    temp_map_tail = NULL;
}

static void reset_temp_counters_for_function(void)
{
    nextTempNumber = 0;
    nextSTempNumber = 0;
}

Data_Type get_operand_type(char *name)
{
    TempDt *cur = temp_map_head;

    if (current_proc_context)
    {
        while (cur)
        {
            if (strcmp(cur->name, name) == 0 && cur->proc_name &&
                strcmp(cur->proc_name, current_proc_context) == 0)
                return cur->type;
            cur = cur->next;
        }
    }

    cur = temp_map_head;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return cur->type;
        cur = cur->next;
    }
    Symbol_Table_Entry *entry = lookup_symbol(name);
    if (entry)
        return entry->type;

    printf("ERROR: cannot infer type of %s", name);
    return INT_TYPE;
}

static char *gen_temp(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "temp%d", nextTempNumber++);
    return xstrdup(buf);
}

static char *gen_stemp(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "stemp%d", nextSTempNumber++);
    return xstrdup(buf);
}

static char *gen_label(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "Label%d", nextLabelNumber++);
    return xstrdup(buf);
}

static const char *relop_to_str(Relational_Op op)
{
    switch (op)
    {
    case REL_LT:
        return "<";
    case REL_LE:
        return "<=";
    case REL_GT:
        return ">";
    case REL_GE:
        return ">=";
    case REL_EQ:
        return "==";
    case REL_NE:
        return "!=";
    default:
        return "??";
    }
}

static void tac_print_assign(Tac *tac, FILE *out)
{
    Tac_Assign *ins = (Tac_Assign *)tac;
    if (is_float_literal(ins->src))
        fprintf(out, "%s = %.2f\n", ins->dest, strtod(ins->src, NULL));
    else
        fprintf(out, "%s = %s\n", ins->dest, ins->src);
}

static void tac_print_binary(Tac *tac, FILE *out)
{
    Tac_Binary *ins = (Tac_Binary *)tac;
    fprintf(out, "%s = %s %s %s\n", ins->dest, ins->lhs, ins->op, ins->rhs);
}

static void tac_print_unary(Tac *tac, FILE *out)
{
    Tac_Unary *ins = (Tac_Unary *)tac;
    fprintf(out, "%s = %s%s\n", ins->dest, ins->op, ins->operand);
}

static void tac_print_label(Tac *tac, FILE *out)
{
    Tac_Label *ins = (Tac_Label *)tac;
    fprintf(out, "%s:\n", ins->label);
}

static void tac_print_goto(Tac *tac, FILE *out)
{
    Tac_Goto *ins = (Tac_Goto *)tac;
    fprintf(out, "goto %s\n", ins->label);
}

static void tac_print_if_goto(Tac *tac, FILE *out)
{
    Tac_If_Goto *ins = (Tac_If_Goto *)tac;
    fprintf(out, "if (%s) goto %s\n", ins->cond, ins->label);
}

static void tac_print_read(Tac *tac, FILE *out)
{
    Tac_Read *ins = (Tac_Read *)tac;
    fprintf(out, "read %s\n", ins->target);
}

static void tac_print_print(Tac *tac, FILE *out)
{
    Tac_Print *ins = (Tac_Print *)tac;
    if (is_float_literal(ins->value))
        fprintf(out, "print %.2f\n", strtod(ins->value, NULL));
    else
        fprintf(out, "print %s\n", ins->value);
}

static void tac_print_call(Tac *tac, FILE *out)
{
    Tac_Call *ins = (Tac_Call *)tac;
    if (ins->dest && *ins->dest)
        fprintf(out, "%s = %s(%s)\n", ins->dest, ins->name, ins->args ? ins->args : "");
    else
        fprintf(out, "%s(%s)\n", ins->name, ins->args ? ins->args : "");
}

static void tac_print_return(Tac *tac, FILE *out)
{
    Tac_Return *ins = (Tac_Return *)tac;
    if (ins->value && *ins->value)
        fprintf(out, "return %s\n", ins->value);
    else
        fprintf(out, "return\n");
}

static void tac_print_proc_begin(Tac *tac, FILE *out)
{
    Tac_Proc *ins = (Tac_Proc *)tac;
    // TODO
    fprintf(out, "proc %s begin\n", ins->name);
}

static void tac_print_proc_end(Tac *tac, FILE *out)
{
    Tac_Proc *ins = (Tac_Proc *)tac;
    // TODO
    fprintf(out, "proc %s end\n", ins->name);
}

static void tac_print_blank(Tac *tac, FILE *out)
{
    (void)tac;
    fprintf(out, "\n");
}

static Tac *tac_alloc(size_t size, Tac_Kind kind, Tac_Print_Fn print_fn)
{
    Tac *instr = (Tac *)checked_malloc(size);
    instr->kind = kind;
    instr->print = print_fn;
    instr->next = NULL;
    return instr;
}

Tac *tac_make_assign(const char *dest, const char *src)
{
    Tac_Assign *ins = (Tac_Assign *)tac_alloc(sizeof(Tac_Assign), TAC_ASSIGN, tac_print_assign);
    ins->dest = xstrdup(dest);
    ins->src = xstrdup(src);
    return (Tac *)ins;
}

Tac *tac_make_binary(const char *dest, const char *lhs, const char *op, const char *rhs)
{
    Tac_Binary *ins = (Tac_Binary *)tac_alloc(sizeof(Tac_Binary), TAC_BINARY, tac_print_binary);
    ins->dest = xstrdup(dest);
    ins->lhs = xstrdup(lhs);
    ins->op = xstrdup(op);
    ins->rhs = xstrdup(rhs);
    return (Tac *)ins;
}

Tac *tac_make_unary(const char *dest, const char *op, const char *operand)
{
    Tac_Unary *ins = (Tac_Unary *)tac_alloc(sizeof(Tac_Unary), TAC_UNARY, tac_print_unary);
    ins->dest = xstrdup(dest);
    ins->op = xstrdup(op);
    ins->operand = xstrdup(operand);
    return (Tac *)ins;
}

Tac *tac_make_label(const char *label)
{
    Tac_Label *ins = (Tac_Label *)tac_alloc(sizeof(Tac_Label), TAC_LABEL, tac_print_label);
    ins->label = xstrdup(label);
    return (Tac *)ins;
}

Tac *tac_make_goto(const char *label)
{
    Tac_Goto *ins = (Tac_Goto *)tac_alloc(sizeof(Tac_Goto), TAC_GOTO, tac_print_goto);
    ins->label = xstrdup(label);
    return (Tac *)ins;
}

Tac *tac_make_if_goto(const char *cond, const char *label)
{
    Tac_If_Goto *ins = (Tac_If_Goto *)tac_alloc(sizeof(Tac_If_Goto), TAC_IF_GOTO, tac_print_if_goto);
    ins->cond = xstrdup(cond);
    ins->label = xstrdup(label);
    return (Tac *)ins;
}

Tac *tac_make_read(const char *target)
{
    Tac_Read *ins = (Tac_Read *)tac_alloc(sizeof(Tac_Read), TAC_READ, tac_print_read);
    ins->target = xstrdup(target);
    return (Tac *)ins;
}

Tac *tac_make_print(const char *value)
{
    Tac_Print *ins = (Tac_Print *)tac_alloc(sizeof(Tac_Print), TAC_PRINT, tac_print_print);
    ins->value = xstrdup(value);
    return (Tac *)ins;
}

Tac *tac_make_call(const char *dest, const char *name, const char *args)
{
    Tac_Call *ins = (Tac_Call *)tac_alloc(sizeof(Tac_Call), TAC_CALL, tac_print_call);
    ins->dest = dest ? xstrdup(dest) : NULL;
    ins->name = xstrdup(name ? name : "");
    ins->args = xstrdup(args ? args : "");
    return (Tac *)ins;
}

Tac *tac_make_return(const char *value)
{
    Tac_Return *ins = (Tac_Return *)tac_alloc(sizeof(Tac_Return), TAC_RETURN, tac_print_return);
    ins->value = value ? xstrdup(value) : NULL;
    return (Tac *)ins;
}

Tac *tac_make_proc_begin(const char *name)
{
    Tac_Proc *ins = (Tac_Proc *)tac_alloc(sizeof(Tac_Proc), TAC_PROC_BEGIN, tac_print_proc_begin);
    ins->name = xstrdup(name);
    return (Tac *)ins;
}

Tac *tac_make_proc_end(const char *name)
{
    Tac_Proc *ins = (Tac_Proc *)tac_alloc(sizeof(Tac_Proc), TAC_PROC_END, tac_print_proc_end);
    ins->name = xstrdup(name);
    return (Tac *)ins;
}

Tac *tac_make_blank(void)
{
    Tac_Blank *ins = (Tac_Blank *)tac_alloc(sizeof(Tac_Blank), TAC_BLANK, tac_print_blank);
    return (Tac *)ins;
}

static Tac *tac_clone(const Tac *instr)
{
    if (!instr)
        return NULL;

    switch (instr->kind)
    {
    case TAC_ASSIGN:
    {
        const Tac_Assign *ins = (const Tac_Assign *)instr;
        return tac_make_assign(ins->dest, ins->src);
    }
    case TAC_BINARY:
    {
        const Tac_Binary *ins = (const Tac_Binary *)instr;
        return tac_make_binary(ins->dest, ins->lhs, ins->op, ins->rhs);
    }
    case TAC_UNARY:
    {
        const Tac_Unary *ins = (const Tac_Unary *)instr;
        return tac_make_unary(ins->dest, ins->op, ins->operand);
    }
    case TAC_LABEL:
    {
        const Tac_Label *ins = (const Tac_Label *)instr;
        return tac_make_label(ins->label);
    }
    case TAC_GOTO:
    {
        const Tac_Goto *ins = (const Tac_Goto *)instr;
        return tac_make_goto(ins->label);
    }
    case TAC_IF_GOTO:
    {
        const Tac_If_Goto *ins = (const Tac_If_Goto *)instr;
        return tac_make_if_goto(ins->cond, ins->label);
    }
    case TAC_READ:
    {
        const Tac_Read *ins = (const Tac_Read *)instr;
        return tac_make_read(ins->target);
    }
    case TAC_PRINT:
    {
        const Tac_Print *ins = (const Tac_Print *)instr;
        return tac_make_print(ins->value);
    }
    case TAC_CALL:
    {
        const Tac_Call *ins = (const Tac_Call *)instr;
        return tac_make_call(ins->dest, ins->name, ins->args);
    }
    case TAC_RETURN:
    {
        const Tac_Return *ins = (const Tac_Return *)instr;
        return tac_make_return(ins->value);
    }
    case TAC_PROC_BEGIN:
    {
        const Tac_Proc *ins = (const Tac_Proc *)instr;
        return tac_make_proc_begin(ins->name);
    }
    case TAC_PROC_END:
    {
        const Tac_Proc *ins = (const Tac_Proc *)instr;
        return tac_make_proc_end(ins->name);
    }
    case TAC_BLANK:
        return tac_make_blank();
    default:
        return NULL;
    }
}

Tac_Seq *tac_seq_create(void)
{
    Tac_Seq *seq = (Tac_Seq *)checked_malloc(sizeof(Tac_Seq));
    seq->head = NULL;
    seq->tail = NULL;
    return seq;
}

void tac_seq_append(Tac_Seq *seq, Tac *instr)
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

void tac_seq_extend(Tac_Seq *seq, const Tac_Seq *other)
{
    if (!seq || !other || !other->head)
        return;

    for (const Tac *cur = other->head; cur; cur = cur->next)
        tac_seq_append(seq, tac_clone(cur));
}

void tac_seq_print(const Tac_Seq *seq, FILE *out)
{
    if (!seq || !out)
        return;

    for (Tac *cur = seq->head; cur; cur = cur->next)
    {
        if (cur->print)
            cur->print(cur, out);
    }
}

static void set_node_code(Ast *node, Tac_Seq *code)
{
    if (!code)
        code = tac_seq_create();

    node->tac_code = code;
}

static void set_node_place(Ast *node, char *place)
{
    if (node->tac_place)
        free(node->tac_place);
    node->tac_place = place;
}

static void gen_stmt(Ast *node);
static char *gen_expr(Ast *node);

static char *gen_bool_expr(Ast *node)
{
    if (node->tac_place)
        return xstrdup(node->tac_place);

    switch (node->kind)
    {
    case AST_NOT:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)node;
        char *child_place = gen_expr(u->child);

        char *result = gen_temp();

        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, u->child->tac_code);
        tac_seq_append(code, tac_make_unary(result, "!", child_place));

        set_node_code(node, code);
        set_node_place(node, result);

        register_temp_type(result, BOOL_TYPE);

        free(child_place);
        return xstrdup(node->tac_place);
    }

    case AST_LOGICAL:
    {
        Logical_Ast *l = (Logical_Ast *)node;
        char *lhs_place = gen_expr(l->lhs);
        char *rhs_place = gen_expr(l->rhs);

        char *result = gen_temp();

        const char *op = (l->op == LOGICAL_AND) ? "&&" : "||";
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, l->lhs->tac_code);
        tac_seq_extend(code, l->rhs->tac_code);
        tac_seq_append(code, tac_make_binary(result, lhs_place, op, rhs_place));

        set_node_code(node, code);
        set_node_place(node, result);

        register_temp_type(result, BOOL_TYPE);

        free(lhs_place);
        free(rhs_place);
        return xstrdup(node->tac_place);
    }

    default:
    {
        char *place = gen_expr(node);
        free(place);
        return xstrdup(node->tac_place);
    }
    }
}

static Data_Type resolve_call_return_type(const Call_Ast *call)
{
    Function_Entry *fn;

    if (!call)
        return VOID_TYPE;

    fn = lookup_function(call->name);
    if (fn)
        return fn->return_type;

    return call->base.data_type;
}

static char *join_call_args(char **args, int argc)
{
    size_t total = 0;
    char *buf;
    char *p;

    if (argc <= 0)
        return xstrdup("");

    for (int i = 0; i < argc; i++)
        total += strlen(args[i]);

    if (argc > 1)
        total += (size_t)(argc - 1) * 2;

    buf = (char *)checked_malloc(total + 1);
    p = buf;

    for (int i = 0; i < argc; i++)
    {
        size_t n = strlen(args[i]);
        if (i > 0)
        {
            *p++ = ',';
            *p++ = ' ';
        }
        memcpy(p, args[i], n);
        p += n;
    }
    *p = '\0';
    return buf;
}

static Tac_Seq *gen_call_code(Call_Ast *call, char **out_place, int want_result)
{
    Tac_Seq *code = tac_seq_create();
    Ast_List *arg = call ? call->args : NULL;
    char **arg_places = NULL;
    int argc = 0;
    int cap = 0;

    while (arg)
    {
        char *arg_place = gen_expr(arg->stmt);
        tac_seq_extend(code, arg->stmt->tac_code);
        if (argc == cap)
        {
            cap = cap ? cap * 2 : 4;
            arg_places = (char **)checked_realloc(arg_places, sizeof(*arg_places) * (size_t)cap);
        }
        arg_places[argc++] = arg_place;
        arg = arg->next;
    }

    char *args = join_call_args(arg_places, argc);

    if (want_result)
    {
        char *tmp = gen_temp();
        tac_seq_append(code, tac_make_call(tmp, call ? call->name : "", args));
        if (out_place)
            *out_place = tmp;
    }
    else
    {
        tac_seq_append(code, tac_make_call(NULL, call ? call->name : "", args));
        if (out_place)
            *out_place = NULL;
    }

    for (int i = 0; i < argc; i++)
        free(arg_places[i]);
    free(arg_places);
    free(args);

    return code;
}

const char *ast_kind_to_string(Ast_Kind kind)
{
    switch (kind)
    {
    case AST_ASSIGN:
        return "AST_ASSIGN";
    case AST_NAME:
        return "AST_NAME";
    case AST_NUMBER:
        return "AST_NUMBER";

    case AST_PLUS:
        return "AST_PLUS";
    case AST_MINUS:
        return "AST_MINUS";
    case AST_MULT:
        return "AST_MULT";
    case AST_DIV:
        return "AST_DIV";
    case AST_UMINUS:
        return "AST_UMINUS";
    case AST_NOT:
        return "AST_NOT";

    case AST_RELATIONAL:
        return "AST_RELATIONAL";
    case AST_LOGICAL:
        return "AST_LOGICAL";

    case AST_IF:
        return "AST_IF";
    case AST_IF_ELSE_STMT:
        return "AST_IF_ELSE_STMT";
    case AST_WHILE:
        return "AST_WHILE";
    case AST_DO_WHILE:
        return "AST_DO_WHILE";

    case AST_CALL:
        return "AST_CALL";
    case AST_RETURN:
        return "AST_RETURN";
    case AST_READ:
        return "AST_READ";
    case AST_PRINT:
        return "AST_PRINT";
    case AST_SEQUENCE:
        return "AST_SEQUENCE";

    case AST_PROCEDURE:
        return "AST_PROCEDURE";
    case AST_PROGRAM:
        return "AST_PROGRAM";

    default:
        return "UNKNOWN_AST_KIND";
    }
}

static char *gen_expr(Ast *node)
{
    if (!node)
        return xstrdup("0");

    if (node->tac_place)
        return xstrdup(node->tac_place);

    switch (node->kind)
    {
    case AST_NAME:
    {
        Name_Ast *n = (Name_Ast *)node;
        Tac_Seq *code = tac_seq_create();
        set_node_code(node, code);
        if (!n->entry || !n->entry->name)
            set_node_place(node, xstrdup("<undef>"));
        else
            set_node_place(node, xstrdup(n->entry->name));
        return xstrdup(node->tac_place);
    }

    case AST_NUMBER:
    {
        Number_Ast *n = (Number_Ast *)node;
        Tac_Seq *code = tac_seq_create();
        set_node_code(node, code);
        set_node_place(node, xstrdup(n->value ? n->value : "0"));
        return xstrdup(node->tac_place);
    }

    case AST_PLUS:
    case AST_MINUS:
    case AST_MULT:
    case AST_DIV:
    {
        Binary_Expr_Ast *b = (Binary_Expr_Ast *)node;
        char *lhs = gen_expr(b->lhs);
        char *rhs = gen_expr(b->rhs);

        char *tmp = gen_temp();

        Tac_Seq *code = tac_seq_create();
        const char *op = "+";

        if (node->kind == AST_MINUS)
            op = "-";
        else if (node->kind == AST_MULT)
            op = "*";
        else if (node->kind == AST_DIV)
            op = "/";

        tac_seq_extend(code, b->lhs->tac_code);
        tac_seq_extend(code, b->rhs->tac_code);
        tac_seq_append(code, tac_make_binary(tmp, lhs, op, rhs));

        set_node_code(node, code);
        set_node_place(node, tmp);

        register_temp_type(tmp, b->lhs->data_type);

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    case AST_UMINUS:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)node;

        char *tmp = gen_temp();

        char *child = gen_expr(u->child);
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, u->child->tac_code);
        tac_seq_append(code, tac_make_unary(tmp, "-", child));

        set_node_code(node, code);
        set_node_place(node, tmp);

        register_temp_type(tmp, u->child->data_type);

        free(child);
        return xstrdup(node->tac_place);
    }

    case AST_RELATIONAL:
    {
        Relational_Ast *r = (Relational_Ast *)node;
        char *lhs = gen_expr(r->lhs);
        char *rhs = gen_expr(r->rhs);

        char *tmp = gen_temp();

        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, r->lhs->tac_code);
        tac_seq_extend(code, r->rhs->tac_code);
        tac_seq_append(code, tac_make_binary(tmp, lhs, relop_to_str(r->op), rhs));

        set_node_code(node, code);
        set_node_place(node, tmp);

        register_temp_type(tmp, BOOL_TYPE);

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    case AST_LOGICAL:
    case AST_NOT:
        return gen_bool_expr(node);

    case AST_IF:
    {
        If_Ast *i = (If_Ast *)node;
        char *l1 = gen_label();
        char *l2 = gen_label();
        char *t2 = gen_stemp();
        char *cond_place = gen_expr(i->cond);
        char *then_place = gen_expr(i->then_part);
        char *else_place = gen_expr(i->else_part);

        char *t1 = gen_temp();

        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, i->cond->tac_code);
        tac_seq_append(code, tac_make_unary(t1, "!", cond_place));
        tac_seq_append(code, tac_make_if_goto(t1, l1));
        tac_seq_extend(code, i->then_part->tac_code);
        tac_seq_append(code, tac_make_assign(t2, then_place));
        tac_seq_append(code, tac_make_goto(l2));
        tac_seq_append(code, tac_make_label(l1));
        tac_seq_extend(code, i->else_part->tac_code);
        tac_seq_append(code, tac_make_assign(t2, else_place));
        tac_seq_append(code, tac_make_label(l2));

        set_node_code(node, code);
        set_node_place(node, t2);

        register_temp_type(t1, BOOL_TYPE);
        // register_temp_type(t2, get_operand_type(t2));
        register_temp_type(t2, i->then_part->data_type);

        free(cond_place);
        free(then_place);
        free(else_place);
        free(t1);
        free(l1);
        free(l2);
        return xstrdup(node->tac_place);
    }

    case AST_CALL:
    {
        Call_Ast *c = (Call_Ast *)node;
        Data_Type ret_type = resolve_call_return_type(c);
        int want_result = (ret_type != VOID_TYPE);
        char *result_place = NULL;
        Tac_Seq *code = gen_call_code(c, &result_place, want_result);

        set_node_code(node, code);
        if (want_result && result_place)
        {
            set_node_place(node, result_place);
            register_temp_type(result_place, ret_type);
        }
        else
        {
            set_node_place(node, xstrdup("0"));
        }

        return xstrdup(node->tac_place);
    }

    case AST_ASSIGN:
    {
        Assignment_Ast *a = (Assignment_Ast *)node;
        char *lhs = gen_expr(a->lhs);
        char *rhs = gen_expr(a->rhs);
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, a->lhs->tac_code);
        tac_seq_extend(code, a->rhs->tac_code);
        tac_seq_append(code, tac_make_assign(lhs, rhs));

        set_node_code(node, code);
        set_node_place(node, xstrdup(lhs));

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    default:
    {
        Tac_Seq *code = tac_seq_create();
        set_node_code(node, code);
        set_node_place(node, xstrdup("0"));
        return xstrdup(node->tac_place);
    }
    }
}

static void gen_if_else_stmt(If_Else_Stmt_Ast *node, Ast *base)
{
    char *cond_place = gen_bool_expr(node->cond);

    Tac_Seq *code = tac_seq_create();

    gen_stmt(node->then_part);

    char *neg_cond = gen_temp();

    char *end_label = gen_label();
    char *else_label = gen_label();

    if (node->else_part)
        gen_stmt(node->else_part);

    tac_seq_extend(code, node->cond->tac_code);
    tac_seq_append(code, tac_make_unary(neg_cond, "!", cond_place));
    tac_seq_append(code, tac_make_if_goto(neg_cond, else_label));
    tac_seq_extend(code, node->then_part ? node->then_part->tac_code : NULL);
    tac_seq_append(code, tac_make_goto(end_label));
    tac_seq_append(code, tac_make_label(else_label));
    tac_seq_extend(code, node->else_part ? node->else_part->tac_code : NULL);
    tac_seq_append(code, tac_make_label(end_label));

    set_node_code(base, code);

    register_temp_type(neg_cond, BOOL_TYPE);

    free(cond_place);
    free(neg_cond);
    free(else_label);
    free(end_label);
}

static void gen_while_stmt(While_Ast *node, Ast *base)
{
    char *cond_place = NULL;
    char *neg_cond = NULL;
    Tac_Seq *code = tac_seq_create();

    if (node->is_do_form)
    {
        gen_stmt(node->body);
        cond_place = gen_bool_expr(node->cond);
        char *start_label = gen_label();

        tac_seq_append(code, tac_make_label(start_label));
        tac_seq_extend(code, node->body ? node->body->tac_code : NULL);
        tac_seq_extend(code, node->cond->tac_code);
        tac_seq_append(code, tac_make_if_goto(cond_place, start_label));

        free(start_label);
    }
    else
    {
        cond_place = gen_bool_expr(node->cond);
        gen_stmt(node->body);
        char *start_label = gen_label();
        char *end_label = gen_label();

        neg_cond = gen_temp();

        tac_seq_append(code, tac_make_label(start_label));
        tac_seq_extend(code, node->cond->tac_code);
        tac_seq_append(code, tac_make_unary(neg_cond, "!", cond_place));
        tac_seq_append(code, tac_make_if_goto(neg_cond, end_label));
        tac_seq_extend(code, node->body ? node->body->tac_code : NULL);
        tac_seq_append(code, tac_make_goto(start_label));
        tac_seq_append(code, tac_make_label(end_label));

        register_temp_type(neg_cond, BOOL_TYPE);

        free(start_label);
        free(end_label);
    }

    set_node_code(base, code);

    if (cond_place)
        free(cond_place);
    if (neg_cond)
        free(neg_cond);
}

static void gen_stmt(Ast *node)
{
    if (!node)
        return;

    if (node->tac_code)
        return;

    switch (node->kind)
    {
    case AST_SEQUENCE:
    {
        Sequence_Ast *s = (Sequence_Ast *)node;
        Ast_List *cur = s->statements;
        Tac_Seq *code = tac_seq_create();

        while (cur)
        {
            gen_stmt(cur->stmt);
            tac_seq_extend(code, cur->stmt->tac_code);
            cur = cur->next;
        }

        set_node_code(node, code);
        return;
    }

    case AST_ASSIGN:
    {
        Assignment_Ast *a = (Assignment_Ast *)node;
        char *lhs = gen_expr(a->lhs);
        char *rhs = gen_expr(a->rhs);
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, a->lhs->tac_code);
        tac_seq_extend(code, a->rhs->tac_code);
        tac_seq_append(code, tac_make_assign(lhs, rhs));

        set_node_code(node, code);
        free(lhs);
        free(rhs);
        return;
    }

    case AST_READ:
    {
        Read_Ast *r = (Read_Ast *)node;
        char *target = gen_expr(r->var);
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, r->var->tac_code);
        tac_seq_append(code, tac_make_read(target));

        set_node_code(node, code);
        free(target);
        return;
    }

    case AST_PRINT:
    {
        Print_Ast *p = (Print_Ast *)node;
        char *value = gen_expr(p->expr);
        Tac_Seq *code = tac_seq_create();

        tac_seq_extend(code, p->expr->tac_code);
        tac_seq_append(code, tac_make_print(value));

        set_node_code(node, code);
        free(value);
        return;
    }

    case AST_CALL:
    {
        Call_Ast *c = (Call_Ast *)node;
        Tac_Seq *code = gen_call_code(c, NULL, 0);
        set_node_code(node, code);
        return;
    }

    case AST_RETURN:
    {
        Return_Ast *r = (Return_Ast *)node;
        Tac_Seq *code = tac_seq_create();

        if (r->expr)
        {
            char *value = gen_expr(r->expr);
            tac_seq_extend(code, r->expr->tac_code);
            if (current_return_temp)
                tac_seq_append(code, tac_make_assign(current_return_temp, value));
            free(value);
        }

        if (current_return_label)
            tac_seq_append(code, tac_make_goto(current_return_label));
        else
            tac_seq_append(code, tac_make_return(current_return_temp));
        set_node_code(node, code);
        return;
    }

    case AST_IF_ELSE_STMT:
        gen_if_else_stmt((If_Else_Stmt_Ast *)node, node);
        return;

    case AST_WHILE:
    case AST_DO_WHILE:
        gen_while_stmt((While_Ast *)node, node);
        return;

    default:
    {
        char *value = gen_expr(node);
        if (!node->tac_code)
            set_node_code(node, tac_seq_create());
        free(value);
        return;
    }
    }
}

static int is_empty_sequence(const Ast *node)
{
    const Sequence_Ast *seq;

    if (!node || node->kind != AST_SEQUENCE)
        return 0;

    seq = (const Sequence_Ast *)node;
    return seq->statements == NULL;
}

static int contains_return_stmt(const Ast *node)
{
    if (!node)
        return 0;

    switch (node->kind)
    {
    case AST_RETURN:
        return 1;
    case AST_SEQUENCE:
    {
        const Sequence_Ast *seq = (const Sequence_Ast *)node;
        Ast_List *cur = seq->statements;
        while (cur)
        {
            if (contains_return_stmt(cur->stmt))
                return 1;
            cur = cur->next;
        }
        return 0;
    }
    case AST_IF_ELSE_STMT:
    {
        const If_Else_Stmt_Ast *stmt = (const If_Else_Stmt_Ast *)node;
        if (contains_return_stmt(stmt->then_part))
            return 1;
        if (stmt->else_part && contains_return_stmt(stmt->else_part))
            return 1;
        return 0;
    }
    case AST_WHILE:
    case AST_DO_WHILE:
    {
        const While_Ast *wh = (const While_Ast *)node;
        return contains_return_stmt(wh->body);
    }
    default:
        return 0;
    }
}

static void emit_procedure_code(Tac_Seq *code, Procedure_Ast *pr, int add_blank)
{
    int has_return = 0;
    int needs_return_label = 0;

    if (!code || !pr)
        return;

    if (pr->return_type == VOID_TYPE && is_empty_sequence(pr->body))
        return;

    reset_temp_counters_for_function();

    current_return_label = NULL;
    current_return_temp = NULL;

    has_return = contains_return_stmt(pr->body);
    needs_return_label = (pr->return_type != VOID_TYPE) || has_return;

    if (needs_return_label)
        current_return_label = gen_label();

    if (pr->return_type != VOID_TYPE)
    {
        current_return_temp = gen_stemp();
        register_temp_type(current_return_temp, pr->return_type);
    }

    tac_set_current_proc(pr->name ? pr->name : "<anon>");
    gen_stmt(pr->body);

    tac_seq_append(code, tac_make_proc_begin(pr->name ? pr->name : "<anon>"));
    tac_seq_extend(code, pr->body ? pr->body->tac_code : NULL);

    if (needs_return_label)
    {
        tac_seq_append(code, tac_make_label(current_return_label));
        tac_seq_append(code, tac_make_return(current_return_temp));
    }

    tac_seq_append(code, tac_make_proc_end(pr->name ? pr->name : "<anon>"));
    if (add_blank)
        tac_seq_append(code, tac_make_blank());

    if (current_return_label)
    {
        free(current_return_label);
        current_return_label = NULL;
    }

    if (current_return_temp)
    {
        free(current_return_temp);
        current_return_temp = NULL;
    }

    tac_set_current_proc(NULL);
}

static void gen_program(Ast *root)
{
    if (!root)
        return;

    if (root->kind == AST_PROGRAM)
    {
        Program_Ast *p = (Program_Ast *)root;
        Ast_List *cur = p->procedures;
        Tac_Seq *code = tac_seq_create();

        while (cur)
        {
            Ast *proc = cur->stmt;

            if (proc && proc->kind == AST_PROCEDURE)
            {
                emit_procedure_code(code, (Procedure_Ast *)proc, 1);
            }
            else
            {
                gen_stmt(proc);
                tac_seq_extend(code, proc->tac_code);
            }

            cur = cur->next;
        }

        set_node_code(root, code);
        return;
    }

    if (root->kind == AST_PROCEDURE)
    {
        Procedure_Ast *pr = (Procedure_Ast *)root;
        Tac_Seq *code = tac_seq_create();

        emit_procedure_code(code, pr, 0);
        set_node_code(root, code);
        return;
    }

    gen_stmt(root);
}

void tac_reset_counters(void)
{
    free_temp_map();
    reset_temp_counters_for_function();
    nextLabelNumber = 0;
    tac_set_current_proc(NULL);
}

void tac_set_current_proc(const char *name)
{
    current_proc_context = name;
}

static void emit_code(const Tac_Seq *code)
{
    if (code && tac_out)
        tac_seq_print(code, tac_out);
}

void tac_generate(Ast *root, FILE *out)
{
    if (!out)
        return;

    tac_out = out;
    tac_reset_counters();
    gen_program(root);
    emit_code(root ? root->tac_code : NULL);
    tac_out = NULL;
}

int tac_generate_to_path(Ast *root, const char *path)
{
    FILE *fp;

    if (!path)
        return 0;

    fp = fopen(path, "w");
    if (!fp)
        return 0;

    tac_generate(root, fp);
    fclose(fp);
    return 1;
}