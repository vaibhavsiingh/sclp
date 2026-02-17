#include "ast.h"

/* ---------- Name ---------- */
NameAst::NameAst(const std::string& name) : name(name) {}

/* ---------- BinaryOp ---------- */
BinaryOpAst::BinaryOpAst(Ast* left, Ast* right, char op)
    : left(left), right(right), op(op) {}

/* ---------- Statement ---------- */
StatementAst::StatementAst(BinaryOpAst* bopAst)
    : bopAst(bopAst) {}

/* ---------- Statement List ---------- */
StatementListAst::StatementListAst(StatementListAst* prev, StatementAst* stmt)
    : prev(prev), stmt(stmt) {}

/* ---------- Procedure ---------- */
ProcedureAst::ProcedureAst(Ast* return_type, NameAst* name,
                           StatementListAst* body)
    : return_type(return_type), name(name), body(body) {}
