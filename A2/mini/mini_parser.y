%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *s);

/* Root of AST */
ProcedureAst* root = NULL;
%}


%code requires {
    #include "ast.h"
}
/* ---------- Union ---------- */
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

/* ---------- Tokens ---------- */
%token <token> INT FLOAT BOOL STRING VOID
%token <token> IDENTIFIER
%token <token> ASSIGNMENT
%token READ PRINT
%token FLOAT_NUMBER INTEGER_NUMBER STRING_CONSTANT

/* ---------- Types ---------- */
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
/* ---------- Grammar ---------- */

void_main_def
    : named_type IDENTIFIER '(' ')' '{' statement_list '}'
      {
          root = new_procedure_ast(
                     $1,
                     new_name_ast($2.lexeme),
                     $6
                 );
          $$ = (Ast*)root;
      }
    ;

named_type
    : INT    { $$ = (Ast*)new_name_ast("int"); }
    | FLOAT  { $$ = (Ast*)new_name_ast("float"); }
    | BOOL   { $$ = (Ast*)new_name_ast("bool"); }
    | STRING { $$ = (Ast*)new_name_ast("string"); }
    | VOID   { $$ = (Ast*)new_name_ast("void"); }
    ;

statement
    : assignment_statement
      {
          $$ = new_statement_ast($1);
      }
    ;

statement_list
    : statement_list statement
      {
          $$ = new_statement_list_ast($1, $2);
      }
    | /* empty */
      {
          $$ = NULL;
      }
    ;

assignment_statement
    : variable_as_operand ASSIGNMENT expression ';'
      {
          $$ = new_binary_op_ast($1, $3, '=');
      }
    ;

expression
    : expression '+' expression
      { $$ = (Ast*)new_binary_op_ast($1, $3, '+'); }
    | expression '-' expression
      { $$ = (Ast*)new_binary_op_ast($1, $3, '-'); }
    | expression '*' expression
      { $$ = (Ast*)new_binary_op_ast($1, $3, '*'); }
    | expression '/' expression
      { $$ = (Ast*)new_binary_op_ast($1, $3, '/'); }
    | variable_as_operand
      { $$ = $1; }
    ;

variable_as_operand
    : IDENTIFIER
      {
          $$ = (Ast*)new_name_ast($1.lexeme);
      }
    ;
%%
/* ---------- Error ---------- */

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}


