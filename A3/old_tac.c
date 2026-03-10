#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"

#include <stdarg.h>

#include "symbol_table.h"

static int nextTempNumber = 0;
static int nextLabelNumber = 0;
static FILE *tac_out = NULL;

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p)
    {
        fprintf(stderr, "Out of memory in TAC generator\n");
        exit(1);
    }
    memcpy(p, s, n);
    return p;
}

static void emit(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(tac_out, fmt, args);
    va_end(args);
    fputc('\n', tac_out);
}

static char *gen_temp(void)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "temp%d", nextTempNumber++);
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

static void gen_stmt(Ast *node);
static char *gen_expr(Ast *node);
static void gen_cond_jump(Ast *cond, const char *true_label, const char *false_label);

static char *gen_bool_expr(Ast *node)
{
    char *result = gen_temp();
    char *l_true = gen_label();
    char *l_false = gen_label();
    char *l_end = gen_label();

    gen_cond_jump(node, l_true, l_false);
    emit("%s:", l_true);
    emit("%s = 1", result);
    emit("goto %s", l_end);
    emit("%s:", l_false);
    emit("%s = 0", result);
    emit("%s:", l_end);

    free(l_true);
    free(l_false);
    free(l_end);
    return result;
}

static char *gen_expr(Ast *node)
{
    if (!node)
        return xstrdup("0");

    switch (node->kind)
    {
    case AST_NAME:
    {
        Name_Ast *n = (Name_Ast *)node;
        if (!n->entry || !n->entry->name)
            return xstrdup("<undef>");
        return xstrdup(n->entry->name);
    }

    case AST_NUMBER:
    {
        Number_Ast *n = (Number_Ast *)node;
        return xstrdup(n->value ? n->value : "0");
    }

    case AST_PLUS:
    case AST_MINUS:
    case AST_MULT:
    case AST_DIV:
    {
        Binary_Expr_Ast *b = (Binary_Expr_Ast *)node;
        char *tmp = gen_temp();
        char *lhs = gen_expr(b->lhs);
        char *rhs = gen_expr(b->rhs);
        const char *op = "+";

        if (node->kind == AST_MINUS)
            op = "-";
        else if (node->kind == AST_MULT)
            op = "*";
        else if (node->kind == AST_DIV)
            op = "/";

        emit("%s = %s %s %s", tmp, lhs, op, rhs);

        free(lhs);
        free(rhs);
        return tmp;
    }

    case AST_UMINUS:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)node;
        char *tmp = gen_temp();
        char *child = gen_expr(u->child);
        emit("%s = -%s", tmp, child);
        free(child);
        return tmp;
    }

    case AST_RELATIONAL:
    {
        Relational_Ast *r = (Relational_Ast *)node;
        char *tmp = gen_temp();
        char *lhs = gen_expr(r->lhs);
        char *rhs = gen_expr(r->rhs);
        emit("%s = %s %s %s", tmp, lhs, relop_to_str(r->op), rhs);
        free(lhs);
        free(rhs);
        return tmp;
    }

    case AST_LOGICAL:
    case AST_NOT:
        return gen_bool_expr(node);

    case AST_IF:
    {
        If_Ast *i = (If_Ast *)node;

        char *cond_place = gen_expr(i->cond);
        char *then_place = gen_expr(i->then_part);
        char *t1 = gen_temp();
        char *t2 = gen_temp();
        char *l1 = gen_label();
        char *l2 = gen_label();

        emit("if %s goto %s", t1, l1);
        emit("%s = !%s", t1, cond_place);
        emit("%s = %s", t2, then_place);
        emit("goto %s", l2);
        char *else_place = gen_expr(i->else_part);
        emit("%s = %s", t2, else_place);
        free(else_place);

        free(cond_place);
        

        
        free(then_place);
        

        emit("%s:", l1);
        
            
        

        emit("%s:", l2);

        free(t1);
        free(l1);
        free(l2);
        return t2;
    }

    case AST_ASSIGN:
    {
        Assignment_Ast *a = (Assignment_Ast *)node;
        char *lhs = gen_expr(a->lhs);
        char *rhs = gen_expr(a->rhs);
        emit("%s = %s", lhs, rhs);
        free(rhs);
        return lhs;
    }

    default:
        return xstrdup("0");
    }
}

static void gen_cond_jump(Ast *cond, const char *true_label, const char *false_label)
{
    if (!cond)
    {
        emit("goto %s", false_label);
        return;
    }

    switch (cond->kind)
    {
    case AST_NOT:
    {
        Unary_Expr_Ast *u = (Unary_Expr_Ast *)cond;
        gen_cond_jump(u->child, false_label, true_label);
        return;
    }

    case AST_LOGICAL:
    {
        Logical_Ast *l = (Logical_Ast *)cond;
        char *mid = gen_label();

        if (l->op == LOGICAL_AND)
        {
            gen_cond_jump(l->lhs, mid, false_label);
            emit("%s:", mid);
            gen_cond_jump(l->rhs, true_label, false_label);
        }
        else
        {
            gen_cond_jump(l->lhs, true_label, mid);
            emit("%s:", mid);
            gen_cond_jump(l->rhs, true_label, false_label);
        }

        free(mid);
        return;
    }

    case AST_RELATIONAL:
    {
        Relational_Ast *r = (Relational_Ast *)cond;
        char *lhs = gen_expr(r->lhs);
        char *rhs = gen_expr(r->rhs);
        emit("if %s %s %s goto %s", lhs, relop_to_str(r->op), rhs, true_label);
        emit("goto %s", false_label);
        free(lhs);
        free(rhs);
        return;
    }

    default:
    {
        char *place = gen_expr(cond);
        emit("if %s != 0 goto %s", place, true_label);
        emit("goto %s", false_label);
        free(place);
        return;
    }
    }
}

static void gen_stmt(Ast *node)
{
    if (!node)
        return;

    switch (node->kind)
    {
    case AST_SEQUENCE:
    {
        Sequence_Ast *s = (Sequence_Ast *)node;
        Ast_List *cur = s->statements;
        while (cur)
        {
            gen_stmt(cur->stmt);
            cur = cur->next;
        }
        return;
    }

    case AST_ASSIGN:
    {        
        Assignment_Ast *a = (Assignment_Ast *)node;
        char *lhs = gen_expr(a->lhs);
        char *rhs = gen_expr(a->rhs);
        emit("%s = %s", lhs, rhs);
        free(lhs);
        free(rhs);
        return;
    }

    case AST_READ:
    {
        Read_Ast *r = (Read_Ast *)node;
        char *target = gen_expr(r->var);
        emit("read %s", target);
        free(target);
        return;
    }

    case AST_PRINT:
    {
        Print_Ast *p = (Print_Ast *)node;
        char *value = gen_expr(p->expr);
        emit("print %s", value);
        free(value);
        return;
    }

    
    case AST_WHILE:
    case AST_DO_WHILE:
    {
        While_Ast *w = (While_Ast *)node;
        char *l_head = gen_label();
        char *l_body = gen_label();
        char *l_exit = gen_label();

        if (node->kind == AST_DO_WHILE)
        {
            emit("%s:", l_body);
            gen_stmt(w->body);
            gen_cond_jump(w->cond, l_body, l_exit);
        }
        else
        {
            emit("%s:", l_head);
            gen_cond_jump(w->cond, l_body, l_exit);
            emit("%s:", l_body);
            gen_stmt(w->body);
            emit("goto %s", l_head);
        }

        emit("%s:", l_exit);

        free(l_head);
        free(l_body);
        free(l_exit);
        return;
    }

    case AST_RETURN:
        emit("return");
        return;

    default:
    {
        char *value = gen_expr(node);
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
        while (cur)
        {
            Ast *proc = cur->stmt;
            if (proc && proc->kind == AST_PROCEDURE)
            {
                Procedure_Ast *pr = (Procedure_Ast *)proc;
                emit("proc %s begin", pr->name ? pr->name : "<anon>");
                gen_stmt(pr->body);
                emit("proc %s end", pr->name ? pr->name : "<anon>");
                emit("");
            }
            else
            {
                gen_stmt(proc);
            }

            cur = cur->next;
        }
        return;
    }

    if (root->kind == AST_PROCEDURE)
    {
        Procedure_Ast *pr = (Procedure_Ast *)root;
        emit("proc %s begin", pr->name ? pr->name : "<anon>");
        gen_stmt(pr->body);
        emit("proc %s end", pr->name ? pr->name : "<anon>");
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