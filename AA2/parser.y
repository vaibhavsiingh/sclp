%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;

Ast *root;          /* Final AST root */
int main_seen = 0;
static Data_Type current_decl_type = INT_TYPE;

static Ast_List *append_ast_list(Ast_List *list, Ast *node)
{
  if (!node)
    return list;

  Ast_List *new_node = malloc(sizeof(Ast_List));
  if (!new_node)
  {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  new_node->stmt = node;
  new_node->next = NULL;

  if (!list)
    return new_node;

  Ast_List *temp = list;
  while (temp->next)
    temp = temp->next;
  temp->next = new_node;

  return list;
}
%}

%union {
    struct {
        char *lexeme;
        int line;
    } token;

    Ast *ast;
    Ast_List *list;
  Data_Type dtype;
}

%token <token> INT FLOAT BOOL STRING VOID MAIN
%token <token> IDENTIFIER
%token <token> INTEGER_NUMBER FLOAT_NUMBER STRING_CONSTANT
%token <token> ASSIGNMENT
%token <token> PRINT READ
%token <token> LT LE GT GE EQ NE
%token <token> AND OR NOT

%type <ast> program
%type <ast> global_decls
%type <ast> global_declaration
%type <ast> void_main_def
%type <ast> statement_list
%type <ast> statement
%type <ast> assignment_statement
%type <ast> declaration_statement
%type <ast> read_statement
%type <ast> print_statement
%type <ast> expression
%type <ast> rel_expression
%type <ast> variable_name
%type <ast> variable_as_operand
%type <ast> constant_as_operand
%type <dtype> type_specifier
%type <list> identifier_list
%type <list> param_list
%type <list> param_list_opt

%right '?' ':'
%left OR
%left AND
%right NOT
%nonassoc LT LE GT GE EQ NE
%left '+' '-'
%left '*' '/'
%right UMINUS 

%%

program
  : global_decls void_main_def
      {
        Program_Ast *prog =
        (Program_Ast*) make_program_ast($2->lineno);

      program_append(prog, $2);
          root = (Ast*)prog;
          $$ = root;
      }
;

global_decls
  : /* empty */
    {
      set_scope(GLOBAL_SCOPE);
      $$ = NULL;
    }
  | global_decls global_declaration
    {
      $$ = NULL;
    }
;

global_declaration
  : type_specifier
    {
      set_scope(GLOBAL_SCOPE);
      current_decl_type = $1;
    }
    identifier_list ';'
    {
      $$ = NULL;
    }
  | VOID IDENTIFIER '(' ')' ';'
    {
      $$ = NULL;
    }
;

/* ================= MAIN PROCEDURE ================= */

void_main_def
    : VOID MAIN '('
    {
      set_scope(LOCAL_SCOPE);
    }
      param_list_opt ')'
      '{'
      statement_list
      '}'
      {
          $$ = make_procedure_ast(
                   $2.lexeme,
                   $5,
                   $8,
                   $2.line);
      }
;

/* ================= STATEMENTS ================= */

statement_list
    : statement_list statement
      {
          sequence_append((Sequence_Ast*)$1, $2);
          $$ = $1;
      }
    | /* empty */
      {
          $$ = make_sequence_ast(yylineno);
      }
;

statement
    : declaration_statement
      { $$ = $1; }
    | assignment_statement
      { $$ = $1; }
    | read_statement
      { $$ = $1; }
    | print_statement
      { $$ = $1; }
;

/* ================= DECLARATIONS ================= */

declaration_statement
  : type_specifier
    {
      current_decl_type = $1;
    }
    identifier_list ';'
      {
          $$ = NULL;
      }
;

identifier_list
    : IDENTIFIER
      {
      insert_symbol($1.lexeme, current_decl_type);
          $$ = NULL;
      }
    | identifier_list ',' IDENTIFIER
      {
      insert_symbol($3.lexeme, current_decl_type);
          $$ = NULL;
      }
;

/* ================= PARAMETERS ================= */

param_list_opt
    : /* empty */
      { $$ = NULL; }
    | param_list
      { $$ = $1; }
;

param_list
    : type_specifier IDENTIFIER
      {
          Symbol_Table_Entry *entry = insert_symbol($2.lexeme, $1);
          $$ = append_ast_list(NULL, make_name_ast(entry, $2.line));
      }
    | param_list ',' type_specifier IDENTIFIER
      {
          Symbol_Table_Entry *entry = insert_symbol($4.lexeme, $3);
          $$ = append_ast_list($1, make_name_ast(entry, $4.line));
      }
;

type_specifier
    : INT    { $$ = INT_TYPE; }
    | FLOAT  { $$ = FLOAT_TYPE; }
    | BOOL   { $$ = BOOL_TYPE; }
    | STRING { $$ = STRING_TYPE; }
;

/* ================= ASSIGNMENT ================= */

assignment_statement
    : variable_as_operand ASSIGNMENT expression ';'
      {
          $$ = make_assignment_ast($1, $3, $2.line);
      }
;

/* ================= READ / PRINT ================= */

read_statement
  : READ variable_as_operand ';'
    {
      $$ = make_read_ast($2, $1.line);
    }
;

print_statement
  : PRINT expression ';'
    {
      $$ = make_print_ast($2, $1.line);
    }
;

/* ================= EXPRESSIONS ================= */

expression
    : expression '+' expression
      {
          $$ = make_binary_ast(AST_PLUS, $1, $3, yylineno);
      }
    | expression '-' expression
      {
          $$ = make_binary_ast(AST_MINUS, $1, $3, yylineno);
      }
    | expression '*' expression
      {
          $$ = make_binary_ast(AST_MULT, $1, $3, yylineno);
      }
    | expression '/' expression
      {
          $$ = make_binary_ast(AST_DIV, $1, $3, yylineno);
      }
    | '-' expression %prec UMINUS
      {
          $$ = make_unary_ast(AST_UMINUS, $2, yylineno);
      }
    | '(' expression ')'
      {
          $$ = $2;
      }
    | variable_as_operand
      {
          $$ = $1;
      }
    | constant_as_operand
      {
          $$ = $1;
      }
    | rel_expression
      {
          $$ = $1;
      }
    | expression AND expression
      {
          $$ = make_logical_ast($1, LOGICAL_AND, $3, yylineno);
      }
    | expression OR expression
      {
          $$ = make_logical_ast($1, LOGICAL_OR, $3, yylineno);
      }
    | NOT expression
      {
          $$ = make_unary_ast(AST_NOT, $2, yylineno);
      }
    | expression '?' expression ':' expression
      {
          $$ = make_if_ast($1, $3, $5, yylineno);
      }
;

rel_expression
    : expression LT expression
      {
          $$ = make_relational_ast($1, REL_LT, $3, yylineno);
      }
    | expression LE expression
      {
          $$ = make_relational_ast($1, REL_LE, $3, yylineno);
      }
    | expression GT expression
      {
          $$ = make_relational_ast($1, REL_GT, $3, yylineno);
      }
    | expression GE expression
      {
          $$ = make_relational_ast($1, REL_GE, $3, yylineno);
      }
    | expression EQ expression
      {
          $$ = make_relational_ast($1, REL_EQ, $3, yylineno);
      }
    | expression NE expression
      {
          $$ = make_relational_ast($1, REL_NE, $3, yylineno);
      }
;

/* ================= VARIABLES ================= */

variable_as_operand
    : variable_name
      { $$ = $1; }
;

variable_name
    : IDENTIFIER
      {
          $$ = make_name_ast(
                   lookup_symbol($1.lexeme),
                   $1.line);
      }
;

/* ================= CONSTANTS ================= */

constant_as_operand
    : INTEGER_NUMBER
      {
          $$ = make_number_ast($1.lexeme, INT_TYPE, $1.line);
      }
    | FLOAT_NUMBER
      {
          $$ = make_number_ast($1.lexeme, FLOAT_TYPE, $1.line);
      }
    | STRING_CONSTANT
      {
          $$ = make_number_ast($1.lexeme, STRING_TYPE, $1.line);
      }
;

%%

void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
