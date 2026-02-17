#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symbol_table.h"

static int check_assignment_ast(Ast *ast);
static int check_name_ast(Ast *ast);
static int check_number_ast(Ast *ast);
static int check_binary_ast(Ast *ast);
static int check_unary_ast(Ast *ast);
static int check_relational_ast(Ast *ast);
static int check_logical_ast(Ast *ast);
static int check_if_ast(Ast *ast);
static int check_while_ast(Ast *ast);
static int check_return_ast(Ast *ast);
static int check_read_ast(Ast *ast);
static int check_print_ast(Ast *ast);
static int check_sequence_ast(Ast *ast);
static int check_procedure_ast(Ast *ast);
static int check_program_ast(Ast *ast);

static int type_error(int line, const char *msg)
{
    fprintf(stderr, "Type error at line %d: %s\n", line, msg);
    return 0;
}

static int is_numeric(Data_Type dt)
{
    return dt == INT_TYPE || dt == FLOAT_TYPE;
}

/* ------------------------------------------------------------ */
/* Utility allocation                                           */
/* ------------------------------------------------------------ */

static void *checked_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return ptr;
}

/* ------------------------------------------------------------ */
/* Assignment                                                   */
/* ------------------------------------------------------------ */

Ast *make_assignment_ast(Ast *lhs, Ast *rhs, int line)
{
    Assignment_Ast *node = checked_malloc(sizeof(Assignment_Ast));

    node->base.kind = AST_ASSIGN;
    node->base.lineno = line;
    node->base.check_ast = check_assignment_ast;
    node->base.print = print_assignment_ast;

    node->lhs = lhs;
    node->rhs = rhs;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Name                                                         */
/* ------------------------------------------------------------ */

Ast *make_name_ast(Symbol_Table_Entry *entry, int line)
{
    Name_Ast *node = checked_malloc(sizeof(Name_Ast));

    node->base.kind = AST_NAME;
    node->base.lineno = line;
    node->base.check_ast = check_name_ast;
    node->base.print = print_name_ast;

    node->entry = entry;
    node->base.data_type = entry ? entry->type : VOID_TYPE;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Number                                                       */
/* ------------------------------------------------------------ */

Ast *make_number_ast(char *value, Data_Type dt, int line)
{
    Number_Ast *node = checked_malloc(sizeof(Number_Ast));

    node->base.kind = AST_NUMBER;
    node->base.data_type = dt;
    node->base.lineno = line;
    node->base.check_ast = check_number_ast;
    node->base.print = print_number_ast;

    node->value = strdup(value);

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Binary Expression                                            */
/* ------------------------------------------------------------ */

Ast *make_binary_ast(Ast_Kind kind, Ast *l, Ast *r, int line)
{
    Binary_Expr_Ast *node = checked_malloc(sizeof(Binary_Expr_Ast));

    node->base.kind = kind;
    node->base.lineno = line;
    node->base.check_ast = check_binary_ast;
    node->base.print = print_binary_ast;

    node->lhs = l;
    node->rhs = r;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Unary Expression                                             */
/* ------------------------------------------------------------ */

Ast *make_unary_ast(Ast_Kind kind, Ast *child, int line)
{
    Unary_Expr_Ast *node = checked_malloc(sizeof(Unary_Expr_Ast));

    node->base.kind = kind;
    node->base.lineno = line;
    node->base.check_ast = check_unary_ast;
    node->base.print = print_unary_ast;

    node->child = child;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Relational                                                   */
/* ------------------------------------------------------------ */

Ast *make_relational_ast(Ast *l, Relational_Op op, Ast *r, int line)
{
    Relational_Ast *node = checked_malloc(sizeof(Relational_Ast));

    node->base.kind = AST_RELATIONAL;
    node->base.lineno = line;
    node->base.check_ast = check_relational_ast;
    node->base.print = print_relational_ast;

    node->lhs = l;
    node->rhs = r;
    node->op = op;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Logical                                                      */
/* ------------------------------------------------------------ */

Ast *make_logical_ast(Ast *l, Logical_Op op, Ast *r, int line)
{
    Logical_Ast *node = checked_malloc(sizeof(Logical_Ast));

    node->base.kind = AST_LOGICAL;
    node->base.lineno = line;
    node->base.check_ast = check_logical_ast;
    node->base.print = print_logical_ast;

    node->lhs = l;
    node->rhs = r;
    node->op = op;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* If                                                           */
/* ------------------------------------------------------------ */

Ast *make_if_ast(Ast *cond, Ast *then_p, Ast *else_p, int line)
{
    If_Ast *node = checked_malloc(sizeof(If_Ast));

    node->base.kind = AST_IF;
    node->base.lineno = line;
    node->base.check_ast = check_if_ast;
    node->base.print = print_if_ast;

    node->cond = cond;
    node->then_part = then_p;
    node->else_part = else_p;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* While / Do While                                             */
/* ------------------------------------------------------------ */

Ast *make_while_ast(Ast *cond, Ast *body, int is_do, int line)
{
    While_Ast *node = checked_malloc(sizeof(While_Ast));

    node->base.kind = is_do ? AST_DO_WHILE : AST_WHILE;
    node->base.lineno = line;
    node->base.check_ast = check_while_ast;
    node->base.print = print_while_ast;

    node->cond = cond;
    node->body = body;
    node->is_do_form = is_do;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Return                                                       */
/* ------------------------------------------------------------ */

Ast *make_return_ast(int line)
{
    Return_Ast *node = checked_malloc(sizeof(Return_Ast));

    node->base.kind = AST_RETURN;
    node->base.lineno = line;
    node->base.check_ast = check_return_ast;
    node->base.print = print_return_ast;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Read                                                        */
/* ------------------------------------------------------------ */

Ast *make_read_ast(Ast *var, int line)
{
    Read_Ast *node = checked_malloc(sizeof(Read_Ast));

    node->base.kind = AST_READ;
    node->base.lineno = line;
    node->base.check_ast = check_read_ast;
    node->base.print = print_read_ast;

    node->var = var;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Print                                                       */
/* ------------------------------------------------------------ */

Ast *make_print_ast(Ast *expr, int line)
{
    Print_Ast *node = checked_malloc(sizeof(Print_Ast));

    node->base.kind = AST_PRINT;
    node->base.lineno = line;
    node->base.check_ast = check_print_ast;
    node->base.print = print_print_ast;

    node->expr = expr;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Sequence                                                     */
/* ------------------------------------------------------------ */

Ast *make_sequence_ast(int line)
{
    Sequence_Ast *node = checked_malloc(sizeof(Sequence_Ast));

    node->base.kind = AST_SEQUENCE;
    node->base.lineno = line;
    node->base.check_ast = check_sequence_ast;
    node->base.print = print_sequence_ast;

    node->statements = NULL;

    return (Ast *)node;
}

void sequence_append(Sequence_Ast *seq, Ast *stmt)
{
    if (!stmt)
        return;

    Ast_List *new_node = checked_malloc(sizeof(Ast_List));
    new_node->stmt = stmt;
    new_node->next = NULL;

    if (!seq->statements)
    {
        seq->statements = new_node;
    }
    else
    {
        Ast_List *temp = seq->statements;
        while (temp->next)
            temp = temp->next;
        temp->next = new_node;
    }
}

/* ------------------------------------------------------------ */
/* Procedure                                                    */
/* ------------------------------------------------------------ */

Ast *make_procedure_ast(char *name, Ast_List *params, Ast *body, int line)
{
    Procedure_Ast *node = checked_malloc(sizeof(Procedure_Ast));

    node->base.kind = AST_PROCEDURE;
    node->base.lineno = line;
    node->base.check_ast = check_procedure_ast;
    node->base.print = print_procedure_ast;

    node->name = strdup(name);
    node->params = params;
    node->body = body;

    return (Ast *)node;
}

/* ------------------------------------------------------------ */
/* Program                                                      */
/* ------------------------------------------------------------ */

Ast *make_program_ast(int line)
{
    Program_Ast *node = checked_malloc(sizeof(Program_Ast));

    node->base.kind = AST_PROGRAM;
    node->base.lineno = line;
    node->base.check_ast = check_program_ast;
    node->base.print = print_program_ast;

    node->procedures = NULL;

    return (Ast *)node;
}

void program_append(Program_Ast *prog, Ast *proc)
{
    Ast_List *new_node = checked_malloc(sizeof(Ast_List));
    new_node->stmt = proc;
    new_node->next = NULL;

    if (!prog->procedures)
    {
        prog->procedures = new_node;
    }
    else
    {
        Ast_List *temp = prog->procedures;
        while (temp->next)
            temp = temp->next;
        temp->next = new_node;
    }
}

/* ------------------------------------------------------------ */
/* Type Checking                                                */
/* ------------------------------------------------------------ */

int check_ast(Ast *ast)
{
    if (!ast)
        return 1;
    if (!ast->check_ast)
        return 1;
    return ast->check_ast(ast);
}

static int check_assignment_ast(Ast *ast)
{
    Assignment_Ast *node = (Assignment_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->lhs))
        ok = 0;
    if (!check_ast(node->rhs))
        ok = 0;

    if (ok)
    {
        Data_Type lhs = node->lhs->data_type;
        Data_Type rhs = node->rhs->data_type;

        if (lhs != rhs)
        {
            return type_error(ast->lineno, "assignment type mismatch");
        }
        ast->data_type = lhs;
    }

    return ok;
}

static int check_name_ast(Ast *ast)
{
    Name_Ast *node = (Name_Ast *)ast;
    if (!node->entry)
        return type_error(ast->lineno, "use of undeclared variable");

    ast->data_type = node->entry->type;
    return 1;
}

static int check_number_ast(Ast *ast)
{
    return 1;
}

static int check_binary_ast(Ast *ast)
{
    Binary_Expr_Ast *node = (Binary_Expr_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->lhs))
        ok = 0;
    if (!check_ast(node->rhs))
        ok = 0;

    if (!ok)
        return 0;

    if (!is_numeric(node->lhs->data_type) ||
        !is_numeric(node->rhs->data_type))
    {
        return type_error(ast->lineno, "arithmetic operands must be numeric");
    }

    ast->data_type = (node->lhs->data_type == FLOAT_TYPE ||
                      node->rhs->data_type == FLOAT_TYPE)
                         ? FLOAT_TYPE
                         : INT_TYPE;

    return 1;
}

static int check_unary_ast(Ast *ast)
{
    Unary_Expr_Ast *node = (Unary_Expr_Ast *)ast;
    if (!check_ast(node->child))
        return 0;

    if (ast->kind == AST_UMINUS)
    {
        if (!is_numeric(node->child->data_type))
            return type_error(ast->lineno, "unary minus expects numeric operand");
        ast->data_type = node->child->data_type;
        return 1;
    }

    if (ast->kind == AST_NOT)
    {
        if (node->child->data_type != BOOL_TYPE)
            return type_error(ast->lineno, "logical not expects boolean operand");
        ast->data_type = BOOL_TYPE;
        return 1;
    }

    return 1;
}

static int check_relational_ast(Ast *ast)
{
    Relational_Ast *node = (Relational_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->lhs))
        ok = 0;
    if (!check_ast(node->rhs))
        ok = 0;
    if (!ok)
        return 0;

    if (!is_numeric(node->lhs->data_type) ||
        !is_numeric(node->rhs->data_type))
    {
        return type_error(ast->lineno, "relational operands must be numeric");
    }

    ast->data_type = BOOL_TYPE;
    return 1;
}

static int check_logical_ast(Ast *ast)
{
    Logical_Ast *node = (Logical_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->lhs))
        ok = 0;
    if (!check_ast(node->rhs))
        ok = 0;
    if (!ok)
        return 0;

    if (node->lhs->data_type != BOOL_TYPE || node->rhs->data_type != BOOL_TYPE)
        return type_error(ast->lineno, "logical operands must be boolean");

    ast->data_type = BOOL_TYPE;
    return 1;
}

static int check_if_ast(Ast *ast)
{
    If_Ast *node = (If_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->cond))
        ok = 0;

    if (node->cond && node->cond->data_type != BOOL_TYPE)
        ok = type_error(ast->lineno, "condition must be boolean");

    if (!check_ast(node->then_part))
        ok = 0;
    if (node->else_part && !check_ast(node->else_part))
        ok = 0;

    if (node->then_part && node->else_part)
    {
        if (node->then_part->data_type != node->else_part->data_type)
            ok = type_error(ast->lineno, "then/else type mismatch");
        else
            ast->data_type = node->then_part->data_type;
    }

    return ok;
}

static int check_while_ast(Ast *ast)
{
    While_Ast *node = (While_Ast *)ast;
    int ok = 1;

    if (!check_ast(node->cond))
        ok = 0;
    if (node->cond && node->cond->data_type != BOOL_TYPE)
        ok = type_error(ast->lineno, "loop condition must be boolean");
    if (!check_ast(node->body))
        ok = 0;

    return ok;
}

static int check_return_ast(Ast *ast)
{
    ast->data_type = VOID_TYPE;
    return 1;
}

static int check_read_ast(Ast *ast)
{
    Read_Ast *node = (Read_Ast *)ast;
    if (!check_ast(node->var))
        return 0;
    if (node->var->data_type == VOID_TYPE)
        return type_error(ast->lineno, "cannot read into void type");
    return 1;
}

static int check_print_ast(Ast *ast)
{
    Print_Ast *node = (Print_Ast *)ast;
    return check_ast(node->expr);
}

static int check_sequence_ast(Ast *ast)
{
    Sequence_Ast *node = (Sequence_Ast *)ast;
    Ast_List *cur = node->statements;
    int ok = 1;

    while (cur)
    {
        if (!check_ast(cur->stmt))
            ok = 0;
        cur = cur->next;
    }

    ast->data_type = VOID_TYPE;
    return ok;
}

static int check_procedure_ast(Ast *ast)
{
    Procedure_Ast *node = (Procedure_Ast *)ast;
    int ok = 1;

    Ast_List *param = node->params;
    while (param)
    {
        if (!check_ast(param->stmt))
            ok = 0;
        param = param->next;
    }

    if (!check_ast(node->body))
        ok = 0;
    ast->data_type = VOID_TYPE;
    return ok;
}

static int check_program_ast(Ast *ast)
{
    Program_Ast *node = (Program_Ast *)ast;
    Ast_List *cur = node->procedures;
    int ok = 1;

    while (cur)
    {
        if (!check_ast(cur->stmt))
            ok = 0;
        cur = cur->next;
    }

    ast->data_type = VOID_TYPE;
    return ok;
}
