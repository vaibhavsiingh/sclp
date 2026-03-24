#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "tac.h"
#include "symbol_table.h"

static int nextTempNumber = 0;
static int nextSTempNumber = 0;
static int nextLabelNumber = 0;
static FILE *tac_out = NULL;

static char *xstrdup(const char *s)
{
    size_t n;
    char *p;

    if (!s)
        s = "";

    n = strlen(s) + 1;
    p = (char *)malloc(n);
    if (!p)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }
    memcpy(p, s, n);
    return p;
}

static char *vformat_string(const char *fmt, va_list args)
{
    va_list copy;
    int len;
    char *buffer;

    va_copy(copy, args);
    len = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);

    if (len < 0)
    {
        fprintf(stderr, "Formatting error in TAC generator\n");
        exit(1);
    }

    buffer = (char *)malloc((size_t)len + 1);
    if (!buffer)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }

    vsnprintf(buffer, (size_t)len + 1, fmt, args);
    return buffer;
}

// i dont understand
static char *format_string(const char *fmt, ...)
{
    va_list args;
    char *buffer;

    va_start(args, fmt);
    buffer = vformat_string(fmt, args);
    va_end(args);
    return buffer;
}

static void append_text(char **dest, const char *text)
{
    size_t old_len;
    size_t add_len;
    char *buffer;

    if (!text || !*text)
        return;

    if (!*dest)
    {
        *dest = xstrdup(text);
        return;
    }

    old_len = strlen(*dest);
    add_len = strlen(text);
    buffer = (char *)realloc(*dest, old_len + add_len + 1);
    if (!buffer)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }

    memcpy(buffer + old_len, text, add_len + 1);
    *dest = buffer;
}

// i dont understand
static void append_linef(char **dest, const char *fmt, ...)
{
    va_list args;
    char *line;

    va_start(args, fmt);
    line = vformat_string(fmt, args);
    va_end(args);

    append_text(dest, line);
    append_text(dest, "\n");
    free(line);
}

static void emit_code(const char *code)
{
    if (code && *code)
        fputs(code, tac_out);
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

static void set_node_code(Ast *node, char *code)
{
    if (!code)
        code = xstrdup("");

    if (node->tac_code)
        free(node->tac_code);
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
    char *code = NULL;

    if (node->tac_place)
        return xstrdup(node->tac_place);

    switch (node->kind)
    {
    case AST_NOT:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)node;
        char *child_place = gen_expr(u->child);
        char *result = gen_temp();

        append_text(&code, u->child->tac_code);
        append_linef(&code, "%s = !%s", result, child_place);

        set_node_code(node, code);
        set_node_place(node, result);

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

        append_text(&code, l->lhs->tac_code);
        append_text(&code, l->rhs->tac_code);
        append_linef(&code, "%s = %s %s %s", result, lhs_place, op, rhs_place);

        set_node_code(node, code);
        set_node_place(node, result);

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
        set_node_code(node, xstrdup(""));
        if (!n->entry || !n->entry->name)
            set_node_place(node, xstrdup("<undef>"));
        else
            set_node_place(node, xstrdup(n->entry->name));
        return xstrdup(node->tac_place);
    }

    case AST_NUMBER:
    {
        Number_Ast *n = (Number_Ast *)node;
        set_node_code(node, xstrdup(""));
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
        char *code = NULL;
        const char *op = "+";

        if (node->kind == AST_MINUS)
            op = "-";
        else if (node->kind == AST_MULT)
            op = "*";
        else if (node->kind == AST_DIV)
            op = "/";

        append_text(&code, b->lhs->tac_code);
        append_text(&code, b->rhs->tac_code);
        append_linef(&code, "%s = %s %s %s", tmp, lhs, op, rhs);

        set_node_code(node, code);
        set_node_place(node, tmp);

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    // have to check
    case AST_UMINUS:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)node;
        char *tmp = gen_temp();
        char *child = gen_expr(u->child);
        char *code = NULL;

        append_text(&code, u->child->tac_code);
        append_linef(&code, "%s = -%s", tmp, child);

        set_node_code(node, code);
        set_node_place(node, tmp);

        free(child);
        return xstrdup(node->tac_place);
    }

    case AST_RELATIONAL:
    {
        Relational_Ast *r = (Relational_Ast *)node;
        char *lhs = gen_expr(r->lhs);
        char *rhs = gen_expr(r->rhs);
        char *tmp = gen_temp();
        char *code = NULL;

        append_text(&code, r->lhs->tac_code);
        append_text(&code, r->rhs->tac_code);
        append_linef(&code, "%s = %s %s %s", tmp, lhs, relop_to_str(r->op), rhs);

        set_node_code(node, code);
        set_node_place(node, tmp);

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    // gotta check
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
        char *code = NULL;

        append_text(&code, i->cond->tac_code);
        append_linef(&code, "%s = !%s", t1, cond_place);
        append_linef(&code, "if %s goto %s", t1, l1);
        append_text(&code, i->then_part->tac_code);
        append_linef(&code, "%s = %s", t2, then_place);
        append_linef(&code, "goto %s", l2);
        append_linef(&code, "%s:", l1);
        append_text(&code, i->else_part->tac_code);
        append_linef(&code, "%s = %s", t2, else_place);
        append_linef(&code, "%s:", l2);

        set_node_code(node, code);
        set_node_place(node, t2);

        free(cond_place);
        free(then_place);
        free(else_place);
        free(t1);
        free(l1);
        free(l2);
        return xstrdup(node->tac_place);
    }

    case AST_ASSIGN:
    {
        Assignment_Ast *a = (Assignment_Ast *)node;
        char *lhs = gen_expr(a->lhs);
        char *rhs = gen_expr(a->rhs);
        char *code = NULL;

        append_text(&code, a->lhs->tac_code);
        append_text(&code, a->rhs->tac_code);
        append_linef(&code, "%s = %s", lhs, rhs);

        set_node_code(node, code);
        set_node_place(node, xstrdup(lhs));

        free(lhs);
        free(rhs);
        return xstrdup(node->tac_place);
    }

    default:
        set_node_code(node, xstrdup(""));
        set_node_place(node, xstrdup("0"));
        return xstrdup(node->tac_place);
    }
}

static void gen_if_else_stmt(If_Else_Stmt_Ast *node, Ast *base)
{
    char *cond_place = gen_bool_expr(node->cond);
    char *neg_cond = gen_temp();
    char *else_label = gen_label();
    char *end_label = gen_label();
    char *code = NULL;

    gen_stmt(node->then_part);
    if (node->else_part)
        gen_stmt(node->else_part);

    append_text(&code, node->cond->tac_code);
    append_linef(&code, "%s = !%s", neg_cond, cond_place);
    append_linef(&code, "if %s goto %s", neg_cond, else_label);
    append_text(&code, node->then_part ? node->then_part->tac_code : "");
    append_linef(&code, "goto %s", end_label);
    append_linef(&code, "%s:", else_label);
    append_text(&code, node->else_part ? node->else_part->tac_code : "");
    append_linef(&code, "%s:", end_label);

    set_node_code(base, code);

    free(cond_place);
    free(neg_cond);
    free(else_label);
    free(end_label);
}

static void gen_while_stmt(While_Ast *node, Ast *base)
{
    char *start_label = gen_label();
    char *end_label = gen_label();
    char *cond_place = NULL;
    char *neg_cond = NULL;
    char *code = NULL;

    cond_place = gen_bool_expr(node->cond);
    gen_stmt(node->body);

    if (node->is_do_form)
    {
        append_linef(&code, "%s:", start_label);
        append_text(&code, node->body ? node->body->tac_code : "");

        append_text(&code, node->cond->tac_code);
        append_linef(&code, "if %s goto %s", cond_place, start_label);
    }
    else
    {
        append_linef(&code, "%s:", start_label);

        neg_cond = gen_temp();
        append_text(&code, node->cond->tac_code);
        append_linef(&code, "%s = !%s", neg_cond, cond_place);
        append_linef(&code, "if %s goto %s", neg_cond, end_label);
        append_text(&code, node->body ? node->body->tac_code : "");
        append_linef(&code, "goto %s", start_label);
        append_linef(&code, "%s:", end_label);
    }

    set_node_code(base, code);

    if (cond_place)
        free(cond_place);
    if (neg_cond)
        free(neg_cond);
    free(start_label);
    free(end_label);
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
        char *code = NULL;

        while (cur)
        {
            gen_stmt(cur->stmt);
            append_text(&code, cur->stmt->tac_code);
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
        char *code = NULL;

        append_text(&code, a->lhs->tac_code);
        append_text(&code, a->rhs->tac_code);
        append_linef(&code, "%s = %s", lhs, rhs);

        set_node_code(node, code);
        free(lhs);
        free(rhs);
        return;
    }

    case AST_READ:
    {
        Read_Ast *r = (Read_Ast *)node;
        char *target = gen_expr(r->var);
        char *code = NULL;

        append_text(&code, r->var->tac_code);
        append_linef(&code, "read %s", target);

        set_node_code(node, code);
        free(target);
        return;
    }

    case AST_PRINT:
    {
        Print_Ast *p = (Print_Ast *)node;
        char *value = gen_expr(p->expr);
        char *code = NULL;

        append_text(&code, p->expr->tac_code);
        append_linef(&code, "print %s", value);

        set_node_code(node, code);
        free(value);
        return;
    }

    case AST_IF_ELSE_STMT:
        gen_if_else_stmt((If_Else_Stmt_Ast *)node, node);
        return;

    case AST_WHILE:
    case AST_DO_WHILE:
        gen_while_stmt((While_Ast *)node, node);
        return;

    case AST_RETURN:
        set_node_code(node, format_string("return\n"));
        return;

    default:
    {
        char *value = gen_expr(node);
        set_node_code(node, xstrdup(node->tac_code));
        free(value);
        return;
    }
    }
}

static void gen_program(Ast *root)
{
    if (!root)
        return;

    if (root->kind == AST_PROGRAM)
    {
        Program_Ast *p = (Program_Ast *)root;
        Ast_List *cur = p->procedures;
        char *code = NULL;

        while (cur)
        {
            Ast *proc = cur->stmt;

            if (proc && proc->kind == AST_PROCEDURE)
            {
                Procedure_Ast *pr = (Procedure_Ast *)proc;
                gen_stmt(pr->body);
                append_linef(&code, "proc %s begin", pr->name ? pr->name : "<anon>");
                append_text(&code, pr->body->tac_code);
                append_linef(&code, "proc %s end", pr->name ? pr->name : "<anon>");
                append_text(&code, "\n");
            }
            else
            {
                gen_stmt(proc);
                append_text(&code, proc->tac_code);
            }

            cur = cur->next;
        }

        set_node_code(root, code);
        return;
    }

    if (root->kind == AST_PROCEDURE)
    {
        Procedure_Ast *pr = (Procedure_Ast *)root;
        char *code = NULL;

        gen_stmt(pr->body);
        append_linef(&code, "proc %s begin", pr->name ? pr->name : "<anon>");
        append_text(&code, pr->body->tac_code);
        append_linef(&code, "proc %s end", pr->name ? pr->name : "<anon>");
        set_node_code(root, code);
        return;
    }

    gen_stmt(root);
}

void tac_reset_counters(void)
{
    nextTempNumber = 0;
    nextLabelNumber = 0;
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