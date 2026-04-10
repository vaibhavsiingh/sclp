#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef struct Tac_Seq Tac_Seq;

typedef enum
{
    AST_ASSIGN,
    AST_NAME,
    AST_NUMBER,

    AST_PLUS,
    AST_MINUS,
    AST_MULT,
    AST_DIV,
    AST_UMINUS,
    AST_NOT,

    AST_RELATIONAL,
    AST_LOGICAL,

    AST_IF,
    AST_IF_ELSE_STMT,
    AST_WHILE,
    AST_DO_WHILE,

    AST_CALL,
    AST_RETURN,
    AST_READ,
    AST_PRINT,
    AST_SEQUENCE,

    AST_PROCEDURE,
    AST_PROGRAM
} Ast_Kind;

typedef enum
{
    INT_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    STRING_TYPE,
    VOID_TYPE
} Data_Type;

typedef enum
{
    REL_LT,
    REL_LE,
    REL_GT,
    REL_GE,
    REL_EQ,
    REL_NE
} Relational_Op;

typedef enum
{
    LOGICAL_AND,
    LOGICAL_OR
} Logical_Op;

typedef struct Ast Ast;

typedef void (*Ast_Print_Fn)(Ast *ast, FILE *file);
typedef int (*Ast_Check_Fn)(Ast *ast);

/* Base AST node */
struct Ast
{
    Ast_Kind kind;
    int lineno;
    Data_Type data_type;
    Ast_Check_Fn check_ast;
    Ast_Print_Fn print;
    Tac_Seq *tac_code;
    char *tac_place;
};

typedef struct Symbol_Table_Entry Symbol_Table_Entry;

typedef struct Ast_List
{
    Ast *stmt;
    struct Ast_List *next;
} Ast_List;

typedef struct
{
    Ast base;
    Ast *lhs;
    Ast *rhs;
} Assignment_Ast;

typedef struct
{
    Ast base;
    Symbol_Table_Entry *entry;
} Name_Ast;

typedef struct
{
    Ast base;
    char *value;
} Number_Ast;

typedef struct
{
    Ast base;
    Ast *lhs;
    Ast *rhs;
} Binary_Expr_Ast;

typedef struct
{
    Ast base;
    Ast *child;
} Unary_Expr_Ast;

typedef struct
{
    Ast base;
    Ast *lhs;
    Ast *rhs;
    Relational_Op op;
} Relational_Ast;

typedef struct
{
    Ast base;
    Ast *lhs;
    Ast *rhs;
    Logical_Op op;
} Logical_Ast;

typedef struct
{
    Ast base;
    Ast *cond;
    Ast *then_part;
    Ast *else_part;
} If_Ast;

typedef struct
{
    Ast base;
    Ast *cond;
    Ast *then_part;
    Ast *else_part;
} If_Else_Stmt_Ast;

typedef struct
{
    Ast base;
    Ast *cond;
    Ast *body;
    int is_do_form;
} While_Ast;

typedef struct
{
    Ast base;
    Ast *expr;
} Return_Ast;

typedef struct
{
    Ast base;
    char *name;
    Ast_List *args;
    int used_as_statement;
} Call_Ast;

typedef struct
{
    Ast base;
    Ast *var;
} Read_Ast;

typedef struct
{
    Ast base;
    Ast *expr;
} Print_Ast;

typedef struct
{
    Ast base;
    Ast_List *statements;
} Sequence_Ast;

typedef struct
{
    Ast base;
    char *name;
    Data_Type return_type;
    Ast_List *params;
    int has_body;
    Ast *body;
} Procedure_Ast;

typedef struct
{
    Ast base;
    Ast_List *procedures;
} Program_Ast;

/* Constructors */
Ast *make_assignment_ast(Ast *lhs, Ast *rhs, int line);
Ast *make_name_ast(Symbol_Table_Entry *entry, int line);
Ast *make_number_ast(char *value, Data_Type dt, int line);
Ast *make_binary_ast(Ast_Kind kind, Ast *l, Ast *r, int line);
Ast *make_unary_ast(Ast_Kind kind, Ast *child, int line);
Ast *make_relational_ast(Ast *l, Relational_Op op, Ast *r, int line);
Ast *make_logical_ast(Ast *l, Logical_Op op, Ast *r, int line);
Ast *make_if_ast(Ast *cond, Ast *then_p, Ast *else_p, int line);
Ast *make_if_else_stmt_ast(Ast *cond, Ast *then_p, Ast *else_p, int line);
Ast *make_while_ast(Ast *cond, Ast *body, int is_do, int line);
Ast *make_call_ast(char *name, Ast_List *args, int line);
Ast *make_return_ast(Ast *expr, int line);
Ast *make_read_ast(Ast *var, int line);
Ast *make_print_ast(Ast *expr, int line);
Ast *make_sequence_ast(int line);
void sequence_append(Sequence_Ast *seq, Ast *stmt);
Ast *make_procedure_ast(char *name, Data_Type return_type, Ast_List *params, int has_body, Ast *body, int line);
Ast *make_program_ast(int line);
void program_append(Program_Ast *prog, Ast *proc);

int check_ast(Ast *ast);

#endif