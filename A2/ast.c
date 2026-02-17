#include <stdio.h>
#include "ast.h"

/* helper */
static void indent(int n) {
    for (int i = 0; i < n; i++) printf("  ");
}

void print_ast(Ast* ast, int indent_level) {
    if (!ast) {
        indent(indent_level);
        printf("(null)\n");
        return;
    }

    /* NameAst */
    if (((NameAst*)ast)->name) {
        NameAst* n = (NameAst*)ast;
        indent(indent_level);
        printf("NameAst(%s)\n", n->name);
        return;
    }

    /* BinaryOpAst */
    BinaryOpAst* b = (BinaryOpAst*)ast;
    if (b->left && b->right) {
        indent(indent_level);
        printf("BinaryOpAst('%c')\n", b->op);
        print_ast(b->left, indent_level + 1);
        print_ast(b->right, indent_level + 1);
        return;
    }

    /* StatementAst */
    StatementAst* s = (StatementAst*)ast;
    if (s->bopAst) {
        indent(indent_level);
        printf("StatementAst\n");
        print_ast((Ast*)s->bopAst, indent_level + 1);
        return;
    }

    /* StatementListAst */
    StatementListAst* sl = (StatementListAst*)ast;
    if (sl->stmt) {
        indent(indent_level);
        printf("StatementListAst\n");
        if (sl->prev)
            print_ast((Ast*)sl->prev, indent_level + 1);
        print_ast((Ast*)sl->stmt, indent_level + 1);
        return;
    }

    /* ProcedureAst */
    ProcedureAst* p = (ProcedureAst*)ast;
    if (p->name && p->body) {
        indent(indent_level);
        printf("ProcedureAst\n");
        indent(indent_level + 1);
        printf("ReturnType:\n");
        print_ast(p->return_type, indent_level + 2);
        indent(indent_level + 1);
        printf("Name:\n");
        print_ast((Ast*)p->name, indent_level + 2);
        indent(indent_level + 1);
        printf("Body:\n");
        print_ast((Ast*)p->body, indent_level + 2);
        return;
    }

    indent(indent_level);
    printf("UnknownAst\n");
}

/* ---------- Name ---------- */
NameAst* new_name_ast(const char* name) {
    NameAst* node = (NameAst*)malloc(sizeof(NameAst));
    node->name = strdup(name);
    return node;
}

/* ---------- Binary Operation ---------- */
BinaryOpAst* new_binary_op_ast(Ast* left, Ast* right, char op) {
    BinaryOpAst* node = (BinaryOpAst*)malloc(sizeof(BinaryOpAst));
    node->left = left;
    node->right = right;
    node->op = op;
    return node;
}

/* ---------- Statement ---------- */
StatementAst* new_statement_ast(BinaryOpAst* bopAst) {
    StatementAst* node = (StatementAst*)malloc(sizeof(StatementAst));
    node->bopAst = bopAst;
    return node;
}

/* ---------- Statement List ---------- */
StatementListAst* new_statement_list_ast(StatementListAst* prev,
                                         StatementAst* stmt) {
    StatementListAst* node =
        (StatementListAst*)malloc(sizeof(StatementListAst));
    node->prev = prev;
    node->stmt = stmt;
    return node;
}

/* ---------- Procedure ---------- */
ProcedureAst* new_procedure_ast(Ast* return_type,
                                NameAst* name,
                                StatementListAst* body) {
    ProcedureAst* node = (ProcedureAst*)malloc(sizeof(ProcedureAst));
    node->return_type = return_type;
    node->name = name;
    node->body = body;
    return node;
}
