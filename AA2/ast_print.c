#include <stdio.h>
#include <stdlib.h>
#include "ast_print.h"
#include "symbol_table.h"

/* ================= INDENTATION SUPPORT ================= */

static int indent_level = 0;

static void print_indent(FILE *file)
{
    for (int i = 0; i < indent_level; i++)
        fprintf(file, "  ");
}

/* ================= NUMBER ================= */

void print_number_ast(Ast *ast, FILE *file)
{
    Number_Ast *num = (Number_Ast *)ast;

    print_indent(file);
    fprintf(file, "Number %s\n", num->value);
}

/* ================= NAME ================= */

void print_name_ast(Ast *ast, FILE *file)
{
    Name_Ast *name = (Name_Ast *)ast;

    print_indent(file);
    if (name->entry)
        fprintf(file, "Name %s\n", name->entry->name);
    else
        fprintf(file, "Name <undef>\n");
}

/* ================= BINARY ================= */

void print_binary_ast(Ast *ast, FILE *file)
{
    Binary_Expr_Ast *bin = (Binary_Expr_Ast *)ast;

    print_indent(file);

    switch (ast->kind)
    {
    case AST_PLUS:
        fprintf(file, "Plus\n");
        break;
    case AST_MINUS:
        fprintf(file, "Minus\n");
        break;
    case AST_MULT:
        fprintf(file, "Mult\n");
        break;
    case AST_DIV:
        fprintf(file, "Div\n");
        break;
    default:
        fprintf(file, "Binary\n");
    }

    indent_level++;
    bin->lhs->print(bin->lhs, file);
    bin->rhs->print(bin->rhs, file);
    indent_level--;
}

/* ================= UNARY ================= */

void print_unary_ast(Ast *ast, FILE *file)
{
    Unary_Expr_Ast *un = (Unary_Expr_Ast *)ast;

    print_indent(file);
    if (ast->kind == AST_NOT)
        fprintf(file, "Not\n");
    else
        fprintf(file, "UnaryMinus\n");

    indent_level++;
    un->child->print(un->child, file);
    indent_level--;
}

/* ================= ASSIGNMENT ================= */

void print_assignment_ast(Ast *ast, FILE *file)
{
    Assignment_Ast *as = (Assignment_Ast *)ast;

    print_indent(file);
    fprintf(file, "Assign\n");

    indent_level++;
    as->lhs->print(as->lhs, file);
    as->rhs->print(as->rhs, file);
    indent_level--;
}

/* ================= RELATIONAL ================= */

void print_relational_ast(Ast *ast, FILE *file)
{
    Relational_Ast *rel = (Relational_Ast *)ast;

    print_indent(file);

    switch (rel->op)
    {
    case REL_LT:
        fprintf(file, "Relational LT\n");
        break;
    case REL_LE:
        fprintf(file, "Relational LE\n");
        break;
    case REL_GT:
        fprintf(file, "Relational GT\n");
        break;
    case REL_GE:
        fprintf(file, "Relational GE\n");
        break;
    case REL_EQ:
        fprintf(file, "Relational EQ\n");
        break;
    case REL_NE:
        fprintf(file, "Relational NE\n");
        break;
    default:
        fprintf(file, "Relational\n");
    }

    indent_level++;
    rel->lhs->print(rel->lhs, file);
    rel->rhs->print(rel->rhs, file);
    indent_level--;
}

/* ================= LOGICAL ================= */

void print_logical_ast(Ast *ast, FILE *file)
{
    Logical_Ast *log = (Logical_Ast *)ast;

    print_indent(file);

    switch (log->op)
    {
    case LOGICAL_AND:
        fprintf(file, "Logical AND\n");
        break;
    case LOGICAL_OR:
        fprintf(file, "Logical OR\n");
        break;
    default:
        fprintf(file, "Logical\n");
    }

    indent_level++;
    log->lhs->print(log->lhs, file);
    log->rhs->print(log->rhs, file);
    indent_level--;
}

/* ================= IF ================= */

void print_if_ast(Ast *ast, FILE *file)
{
    If_Ast *if_ast = (If_Ast *)ast;

    print_indent(file);
    fprintf(file, "If\n");

    indent_level++;
    if_ast->cond->print(if_ast->cond, file);
    if_ast->then_part->print(if_ast->then_part, file);
    if (if_ast->else_part)
        if_ast->else_part->print(if_ast->else_part, file);
    indent_level--;
}

/* ================= WHILE / DO WHILE ================= */

void print_while_ast(Ast *ast, FILE *file)
{
    While_Ast *wh = (While_Ast *)ast;

    print_indent(file);
    fprintf(file, "%s\n", wh->is_do_form ? "DoWhile" : "While");

    indent_level++;
    wh->cond->print(wh->cond, file);
    wh->body->print(wh->body, file);
    indent_level--;
}

/* ================= RETURN ================= */

void print_return_ast(Ast *ast, FILE *file)
{
    (void)ast;

    print_indent(file);
    fprintf(file, "Return\n");
}

/* ================= READ ================= */

void print_read_ast(Ast *ast, FILE *file)
{
    Read_Ast *rd = (Read_Ast *)ast;

    print_indent(file);
    fprintf(file, "Read\n");

    indent_level++;
    rd->var->print(rd->var, file);
    indent_level--;
}

/* ================= PRINT ================= */

void print_print_ast(Ast *ast, FILE *file)
{
    Print_Ast *pr = (Print_Ast *)ast;

    print_indent(file);
    fprintf(file, "Print\n");

    indent_level++;
    pr->expr->print(pr->expr, file);
    indent_level--;
}

/* ================= SEQUENCE ================= */

void print_sequence_ast(Ast *ast, FILE *file)
{
    Sequence_Ast *seq = (Sequence_Ast *)ast;

    print_indent(file);
    fprintf(file, "Sequence\n");

    indent_level++;

    Ast_List *temp = seq->statements;
    while (temp)
    {
        temp->stmt->print(temp->stmt, file);
        temp = temp->next;
    }

    indent_level--;
}

/* ================= PROCEDURE ================= */

void print_procedure_ast(Ast *ast, FILE *file)
{
    Procedure_Ast *proc = (Procedure_Ast *)ast;

    print_indent(file);
    fprintf(file, "Procedure %s\n", proc->name);

    indent_level++;
    if (proc->params)
    {
        print_indent(file);
        fprintf(file, "Params\n");

        indent_level++;
        Ast_List *param = proc->params;
        while (param)
        {
            param->stmt->print(param->stmt, file);
            param = param->next;
        }
        indent_level--;
    }
    proc->body->print(proc->body, file);
    indent_level--;
}

/* ================= PROGRAM ================= */

void print_program_ast(Ast *ast, FILE *file)
{
    Program_Ast *prog = (Program_Ast *)ast;

    print_indent(file);
    fprintf(file, "Program\n");

    indent_level++;

    Ast_List *temp = prog->procedures;
    while (temp)
    {
        temp->stmt->print(temp->stmt, file);
        temp = temp->next;
    }

    indent_level--;
}
