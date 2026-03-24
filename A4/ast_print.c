#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast_print.h"
#include "symbol_table.h"

/* ================= INDENTATION SUPPORT ================= */

static int indent_level = 0;

static void print_indent(FILE *file)
{
    for (int i = 0; i < indent_level; i++)
        fprintf(file, "  ");
}

/* ================= TYPE CONVERSION ================= */

static const char *data_type_to_string(Data_Type type)
{
    switch (type)
    {
    case INT_TYPE:
        return "int";
    case FLOAT_TYPE:
        return "float";
    case BOOL_TYPE:
        return "bool";
    case STRING_TYPE:
        return "string";
    case VOID_TYPE:
        return "void";
    default:
        return "unknown";
    }
}

/* Recursively determine the data type of an AST expression */
static const char *get_expr_type(Ast *ast)
{
    if (!ast)
        return "unknown";

    switch (ast->kind)
    {
    case AST_NAME:
    {
        Name_Ast *name = (Name_Ast *)ast;
        if (name->entry)
            return data_type_to_string(name->entry->type);
        return "unknown";
    }
    case AST_NUMBER:
    {
        Number_Ast *num = (Number_Ast *)ast;
        /* Check if number contains decimal point for float detection */
        if (strchr(num->value, '.'))
            return "float";
        else
            return "int";
    }
    case AST_PLUS:
    case AST_MINUS:
    case AST_MULT:
    case AST_DIV:
    {
        Binary_Expr_Ast *bin = (Binary_Expr_Ast *)ast;
        const char *lhs_type = get_expr_type(bin->lhs);
        const char *rhs_type = get_expr_type(bin->rhs);

        /* If either operand is float, result is float; otherwise int */
        if (strcmp(lhs_type, "float") == 0 || strcmp(rhs_type, "float") == 0)
            return "float";
        return "int";
    }
    case AST_UMINUS:
    {
        Unary_Expr_Ast *un = (Unary_Expr_Ast *)ast;
        return get_expr_type(un->child);
    }
    default:
        return "unknown";
    }
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
        fprintf(file, "Name %s : %s\n", name->entry->name, data_type_to_string(name->entry->type));
    else
        fprintf(file, "Name <undef>\n");
}

/* ================= BINARY ================= */

void print_binary_ast(Ast *ast, FILE *file)
{
    Binary_Expr_Ast *bin = (Binary_Expr_Ast *)ast;
    const char *op_name = NULL;
    const char *expr_type = NULL;

    print_indent(file);

    switch (ast->kind)
    {
    case AST_PLUS:
        op_name = "Plus";
        break;
    case AST_MINUS:
        op_name = "Minus";
        break;
    case AST_MULT:
        op_name = "Mult";
        break;
    case AST_DIV:
        op_name = "Div";
        break;
    default:
        op_name = "Binary";
    }

    /* Get the type of the entire expression */
    expr_type = get_expr_type(ast);
    if (expr_type && strcmp(expr_type, "unknown") != 0)
        fprintf(file, "%s : %s\n", op_name, expr_type);
    else
        fprintf(file, "%s\n", op_name);

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


void print_if_else_stmt_ast(Ast *ast, FILE *file)
{
    If_Else_Stmt_Ast *if_ast = (If_Else_Stmt_Ast *)ast;

    print_indent(file);
    // CHANGE REQUIRED
    fprintf(file, "IfElseStmt\n");

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
