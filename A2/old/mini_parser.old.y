%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

 int yylex(void);
void yyerror(const char *s);

ProcedureAst* root = nullptr;
%}

/* Forward declarations for y.tab.h */

%union {
    struct {
        char *lexeme;
        int line;
    } token;

    Ast* ast;
    StatementAst* stmt;
    StatementListAst* stmt_list;
    BinaryOpAst* bop;
}

/* Tokens */
%token <token> INT FLOAT BOOL STRING VOID
%token <token> IDENTIFIER
%token <token> ASSIGNMENT
%token READ PRINT
%token FLOAT_NUMBER INTEGER_NUMBER STRING_CONSTANT

/* Types */
%type <ast> named_type
%type <ast> variable_as_operand
%type <ast> expression
%type <stmt> statement
%type <stmt_list> statement_list
%type <bop> assignment_statement
%type <ast> void_main_def

%left '+' '-'
%left '*' '/'

%%
void_main_def
    : named_type IDENTIFIER '(' ')' '{' statement_list '}'
      {
          root = new ProcedureAst($1, new NameAst($2.lexeme), $6);
          $$ = root;
      }
    ;

named_type
    : INT    { $$ = new NameAst("int"); }
    | FLOAT  { $$ = new NameAst("float"); }
    | BOOL   { $$ = new NameAst("bool"); }
    | STRING { $$ = new NameAst("string"); }
    | VOID   { $$ = new NameAst("void"); }
    ;

statement
    : assignment_statement { $$ = new StatementAst($1); }
    ;

statement_list
    : statement_list statement { $$ = new StatementListAst($1, $2); }
    | /* empty */             { $$ = nullptr; }
    ;

assignment_statement
    : variable_as_operand ASSIGNMENT expression ';'
      { $$ = new BinaryOpAst($1, $3, '='); }
    ;

expression
    : expression '+' expression { $$ = new BinaryOpAst($1, $3, '+'); }
    | expression '-' expression { $$ = new BinaryOpAst($1, $3, '-'); }
    | expression '*' expression { $$ = new BinaryOpAst($1, $3, '*'); }
    | expression '/' expression { $$ = new BinaryOpAst($1, $3, '/'); }
    | variable_as_operand       { $$ = $1; }
    ;

variable_as_operand
    : IDENTIFIER { $$ = new NameAst($1.lexeme); }
    ;
%%



void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}