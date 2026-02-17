#ifndef AST_H
#define AST_H

#include <string>

/* ---------- Base ---------- */
class Ast {
public:
    virtual ~Ast() = default;
};

/* ---------- Name ---------- */
class NameAst : public Ast {
public:
    std::string name;
    explicit NameAst(const std::string& name);
};

/* ---------- Binary Operation ---------- */
class BinaryOpAst : public Ast {
public:
    Ast* left;
    Ast* right;
    char op;

    BinaryOpAst(Ast* left, Ast* right, char op);
};

/* ---------- Statement ---------- */
class StatementAst : public Ast {
public:
    BinaryOpAst* bopAst;
    explicit StatementAst(BinaryOpAst* bopAst);
};

/* ---------- Statement List ---------- */
class StatementListAst : public Ast {
public:
    StatementListAst* prev;
    StatementAst* stmt;

    StatementListAst(StatementListAst* prev, StatementAst* stmt);
};

/* ---------- Procedure ---------- */
class ProcedureAst : public Ast {
public:
    Ast* return_type;
    NameAst* name;
    StatementListAst* body;

    ProcedureAst(Ast* return_type, NameAst* name,
                 StatementListAst* body);
};

#endif
