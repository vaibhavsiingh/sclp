#include <stdio.h>
#include "ast.h"

/* helper */
static void indent(int n) {
    for (int i = 0; i < n; i++) printf("  ");
}

void print_ast(Ast* ast, int indent) {
    if (!ast) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (ast->kind) {

    case AST_NAME: {
        NameAst* n = (NameAst*)ast;
        printf("NameAst(%s)\n", n->name);
        break;
    }

    case AST_BINARY_OP: {
        BinaryOpAst* b = (BinaryOpAst*)ast;
        printf("BinaryOpAst('%c')\n", b->op);
        print_ast(b->left, indent + 1);
        print_ast(b->right, indent + 1);
        break;
    }

    case AST_STATEMENT: {
        StatementAst* s = (StatementAst*)ast;
        printf("StatementAst\n");
        print_ast((Ast*)s->bopAst, indent + 1);
        break;
    }

    case AST_STATEMENT_LIST: {
        StatementListAst* sl = (StatementListAst*)ast;
        printf("StatementListAst\n");
        if (sl->prev)
            print_ast((Ast*)sl->prev, indent + 1);
        print_ast((Ast*)sl->stmt, indent + 1);
        break;
    }

    case AST_PROCEDURE: {
        ProcedureAst* p = (ProcedureAst*)ast;
        printf("ProcedureAst\n");
        print_ast(p->return_type, indent + 1);
        print_ast((Ast*)p->name, indent + 1);
        print_ast((Ast*)p->body, indent + 1);
        break;
    }
    }
}


/* ---------- Name ---------- */
NameAst* new_name_ast(const char* name) {
    NameAst* node = malloc(sizeof(NameAst));
    node->base.kind = AST_NAME;
    node->name = strdup(name);
    return node;
}

BinaryOpAst* new_binary_op_ast(Ast* left, Ast* right, char op) {
    BinaryOpAst* node = malloc(sizeof(BinaryOpAst));
    node->base.kind = AST_BINARY_OP;
    node->left = left;
    node->right = right;
    node->op = op;
    return node;
}

StatementAst* new_statement_ast(BinaryOpAst* bopAst) {
    StatementAst* node = malloc(sizeof(StatementAst));
    node->base.kind = AST_STATEMENT;
    node->bopAst = bopAst;
    return node;
}

StatementListAst* new_statement_list_ast(StatementListAst* prev,
                                         StatementAst* stmt) {
    StatementListAst* node = malloc(sizeof(StatementListAst));
    node->base.kind = AST_STATEMENT_LIST;
    node->prev = prev;
    node->stmt = stmt;
    return node;
}

ProcedureAst* new_procedure_ast(Ast* return_type,
                                NameAst* name,
                                StatementListAst* body) {
    ProcedureAst* node = malloc(sizeof(ProcedureAst));
    node->base.kind = AST_PROCEDURE;
    node->return_type = return_type;
    node->name = name;
    node->body = body;
    return node;
}
