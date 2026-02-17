#ifndef AST_H
#define AST_H

#include <stdlib.h>

typedef enum {
    AST_NAME,
    AST_BINARY_OP,
    AST_STATEMENT,
    AST_STATEMENT_LIST,
    AST_PROCEDURE
} AstKind;

/* ---------- Base ---------- */
typedef struct Ast {
    AstKind kind;
} Ast;

/* ---------- Name ---------- */
typedef struct {
    Ast base;
    char* name;
} NameAst;

NameAst* new_name_ast(const char* name);

/* ---------- Binary Operation ---------- */
typedef struct {
    Ast base;
    Ast* left;
    Ast* right;
    char op;
} BinaryOpAst;

BinaryOpAst* new_binary_op_ast(Ast* left, Ast* right, char op);

/* ---------- Statement ---------- */
typedef struct {
    Ast base;
    BinaryOpAst* bopAst;
} StatementAst;

StatementAst* new_statement_ast(BinaryOpAst* bopAst);

/* ---------- Statement List ---------- */
typedef struct StatementListAst {
    Ast base;
    struct StatementListAst* prev;
    StatementAst* stmt;
} StatementListAst;

StatementListAst* new_statement_list_ast(StatementListAst* prev,
                                         StatementAst* stmt);

/* ---------- Procedure ---------- */
typedef struct {
    Ast base;
    Ast* return_type;
    NameAst* name;
    StatementListAst* body;
} ProcedureAst;

ProcedureAst* new_procedure_ast(Ast* return_type,
                                NameAst* name,
                                StatementListAst* body);


void print_ast(Ast* ast, int indent);
#endif
