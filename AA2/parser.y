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

/* External declarations from main.c */
extern int dump_tokens;
extern FILE *tokfile;
extern void dump_token(const char *type, const char *lexeme, int line);

/* Parser globals */
int main_seen = 0;

Ast *root;
static Data_Type current_decl_type = INT_TYPE;
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

%token <token> INT FLOAT BOOL STRING VOID
%token <token> IDENTIFIER
%token <token> INTEGER_NUMBER FLOAT_NUMBER STRING_CONSTANT
%token <token> ASSIGNMENT
%token <token> PRINT READ
%token <token> EQ NE LT LE GT GE
%token <token> AND OR NOT

%type <token> named_type
%type <token> variable_name
%type <ast> variable_as_operand
%type <ast> expression
%type <ast> constant_as_operand
%type <ast> statement
%type <ast> assignment_statement
%type <ast> print_statement
%type <ast> read_statement
%type <ast> statement_list
%type <ast> void_main_def
%type <dtype> param_type
%type <ast> formal_param
%type <list> formal_param_list

%right '?' ':'
%left OR
%left AND
%right NOT
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%right UMINUS 

%%

program
    : global_decl_statement_list void_main_def
      {
          root = make_program_ast(yylineno);
          program_append((Program_Ast *)root, $2);
      }
    | void_main_def
      {
          root = make_program_ast(yylineno);
          program_append((Program_Ast *)root, $1);
      }
    ;

global_decl_statement_list
    : global_decl_statement_list var_decl_stmt
    | global_decl_statement_list main_decl
      {
          if (main_seen) {
              yyerror("multiple declarations/definitions of functions");
              YYERROR;
          }
          main_seen = 1;
      }
    | var_decl_stmt
    | main_decl
      {
          if (main_seen) {
              yyerror("multiple declarations/definitions of main");
              YYERROR;
          }
          main_seen = 1;
      }
    ;


main_decl
	: named_type IDENTIFIER '(' formal_param_list ')' ';'    
	| named_type IDENTIFIER '(' ')' ';'                       
    ;

formal_param_list
	: formal_param_list ',' formal_param
      {
          
          Ast_List *new_node = malloc(sizeof(Ast_List));
          new_node->stmt = $3;
          new_node->next = NULL;
          
          if ($1 == NULL) {
              $$ = new_node;
          } else {
              Ast_List *temp = $1;
              while (temp->next)
                  temp = temp->next;
              temp->next = new_node;
              $$ = $1;
          }
          
      }
	| formal_param
      {
          Ast_List *new_node = malloc(sizeof(Ast_List));
          new_node->stmt = $1;
          new_node->next = NULL;
          $$ = new_node;
      }
;

formal_param
    : param_type IDENTIFIER
      {   set_scope(LOCAL_SCOPE);
          Symbol_Table_Entry *entry = insert_symbol($2.lexeme, $1);
          $$ = make_name_ast(entry, $2.line);
          set_scope(GLOBAL_SCOPE);
      }
    ;


param_type
    : INT       { $$ = INT_TYPE; }
    | FLOAT     { $$ = FLOAT_TYPE; }
    | BOOL      { $$ = BOOL_TYPE; }
    | STRING    { $$ = STRING_TYPE; }

/* var_decl_stmt_list
    : var_decl_stmt
    | var_decl_stmt_list var_decl_stmt
; */

var_decl_stmt
    : named_type var_decl_item_list ';' 
    ;

var_decl_item_list
    : var_decl_item_list ',' var_decl_item
    | var_decl_item
    ;

var_decl_item
    : IDENTIFIER
      {     
          //printf("%s, %d\n", $1.lexeme, (int)current_decl_type);
          insert_symbol($1.lexeme, current_decl_type);
          //print_symbol_table();
      }
    ;

named_type
    : INT    { current_decl_type = INT_TYPE; }
    | FLOAT  { current_decl_type = FLOAT_TYPE; }
    | BOOL   { current_decl_type = BOOL_TYPE; }
    | STRING { current_decl_type = STRING_TYPE; }
    | VOID   { current_decl_type = VOID_TYPE; }
    ;

void_main_def
    : named_type IDENTIFIER '(' ')' '{'
      {
          set_scope(LOCAL_SCOPE);
      }
      optional_local_var_decl_stmt_list
      statement_list
      '}'
      {
          $$ = make_procedure_ast($2.lexeme, NULL, $8, $2.line);
          set_scope(GLOBAL_SCOPE);
      }
    | named_type IDENTIFIER '(' formal_param_list ')' '{'
      {
          set_scope(LOCAL_SCOPE);
      }
      optional_local_var_decl_stmt_list
      statement_list
      '}'
      {
          $$ = make_procedure_ast($2.lexeme, $4, $9, $2.line);
          set_scope(GLOBAL_SCOPE);
      }
    ;

optional_local_var_decl_stmt_list
    : var_decl_stmt_list
    | /* empty */
    ;

var_decl_stmt_list
    : var_decl_stmt_list var_decl_stmt
    | var_decl_stmt
    ;

statement_list
    : statement_list statement
      {
          if ($1 == NULL) {
              $$ = make_sequence_ast(yylineno);
          } else {
              $$ = $1;
          }
          sequence_append((Sequence_Ast *)$$, $2);
      }
    | /* empty */
      {
          $$ = NULL;
      }
    ;

statement
    : assignment_statement  { $$ = $1; }
    | print_statement       { $$ = $1; }
    | read_statement        { $$ = $1; }
    ;

assignment_statement
    : variable_as_operand ASSIGNMENT expression ';'
      {
          $$ = make_assignment_ast($1, $3, $2.line);
      }
    ;

variable_as_operand
    : variable_name
      {
          Symbol_Table_Entry *entry = lookup_symbol($1.lexeme);
          if (!entry) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Variable '%s' not declared", $1.lexeme);
              yyerror(buf);
              YYERROR;
          }
          $$ = make_name_ast(entry, $1.line);
      }
    ;

variable_name
    : IDENTIFIER
    ;

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
    | '-' expression    %prec UMINUS
      {
          $$ = make_unary_ast(AST_UMINUS, $2, yylineno);
      }
    | NOT expression
      {
          $$ = make_unary_ast(AST_NOT, $2, yylineno);
      }
    | expression EQ expression
      {
          $$ = make_relational_ast($1, REL_EQ, $3, yylineno);
      }
    | expression NE expression
      {
          $$ = make_relational_ast($1, REL_NE, $3, yylineno);
      }
    | expression LT expression
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
    | expression AND expression
      {
          $$ = make_logical_ast($1, LOGICAL_AND, $3, yylineno);
      }
    | expression OR expression
      {
          $$ = make_logical_ast($1, LOGICAL_OR, $3, yylineno);
      }
    | expression '?' expression ':' expression
      {
          $$ = make_if_ast($1, $3, $5, yylineno);
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
    ;

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

print_statement
    : PRINT expression ';'
      {
          $$ = make_print_ast($2, $1.line);
      }
    ;

read_statement
    : READ variable_name ';'
      {
          Symbol_Table_Entry *entry = lookup_symbol($2.lexeme);
          if (!entry) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Variable '%s' not declared", $2.lexeme);
              yyerror(buf);
              YYERROR;
          }
          Ast *var = make_name_ast(entry, $2.line);
          $$ = make_read_ast(var, $1.line);
      }
    ;

%%

void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
