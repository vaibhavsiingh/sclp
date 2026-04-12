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
extern int sa_scan;
extern int sa_parse;
extern FILE *tokfile;
extern void dump_token(const char *type, const char *lexeme, int line);

/* Parser globals */
int main_defined = 0;

Ast *root;
static Data_Type current_decl_type = INT_TYPE;
static Data_Type current_func_return_type = INT_TYPE;
static int collecting_function_locals = 0;
static int current_function_local_bytes = 0;
static Ast_List *current_function_locals = NULL;

static void append_ast_list_node(Ast_List **head, Ast *stmt)
{
    Ast_List *node;
    Ast_List *cur;

    if (!head || !stmt)
        return;

    node = (Ast_List *)malloc(sizeof(Ast_List));
    if (!node)
    {
        yyerror("out of memory while recording locals");
        return;
    }

    node->stmt = stmt;
    node->next = NULL;

    if (!*head)
    {
        *head = node;
        return;
    }

    cur = *head;
    while (cur->next)
        cur = cur->next;
    cur->next = node;
}

static int type_size_bytes(Data_Type type)
{
    switch (type)
    {
    case VOID_TYPE:
        return 0;
    default:
        return 4;
    }
}

static int register_function(const char *name, Data_Type ret_type,
                             Ast_List *params_ast, int is_definition, int line)
{
    (void)line;
    Param_Type_List *params = param_types_from_ast_list(params_ast);
    Function_Entry *entry = lookup_function(name);

    if (entry)
    {
        if (!is_definition)
        {
            yyerror("multiple declarations/definitions of functions");
            free_param_type_list(params);
            return 0;
        }

        if (entry->defined)
        {
            yyerror("multiple declarations/definitions of functions");
            free_param_type_list(params);
            return 0;
        }

        if (entry->return_type != ret_type ||
            !compare_param_type_lists(entry->params, params))
        {
            yyerror("function definition parameter list does not match declaration");
            free_param_type_list(params);
            return 0;
        }

        entry->defined = 1;
        free_param_type_list(params);
    }
    else
    {
        if (!add_function_entry(name, ret_type, params, is_definition))
        {
            yyerror("unable to record function declaration");
            free_param_type_list(params);
            return 0;
        }
    }

    if (is_definition && strcmp(name, "main") == 0)
        main_defined = 1;

    return 1;
}

static int same_procedure_signature(const Procedure_Ast *lhs, const Procedure_Ast *rhs)
{
    Param_Type_List *lhs_params;
    Param_Type_List *rhs_params;
    int same = 0;

    if (!lhs || !rhs)
        return 0;

    if (!lhs->name || !rhs->name)
        return 0;

    if (strcmp(lhs->name, rhs->name) != 0)
        return 0;

    if (lhs->return_type != rhs->return_type)
        return 0;

    lhs_params = param_types_from_ast_list(lhs->params);
    rhs_params = param_types_from_ast_list(rhs->params);

    same = compare_param_type_lists(lhs_params, rhs_params);

    free_param_type_list(lhs_params);
    free_param_type_list(rhs_params);

    return same;
}

static Procedure_Ast *find_matching_decl(Program_Ast *prog, const Procedure_Ast *def_proc)
{
    Ast_List *cur;

    if (!prog || !def_proc)
        return NULL;

    cur = prog->procedures;
    while (cur)
    {
        if (cur->stmt && cur->stmt->kind == AST_PROCEDURE)
        {
            Procedure_Ast *proc = (Procedure_Ast *)cur->stmt;
            if (!proc->has_body && same_procedure_signature(proc, def_proc))
                return proc;
        }
        cur = cur->next;
    }

    return NULL;
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

%token <token> INT FLOAT BOOL STRING VOID
%token <token> IDENTIFIER
%token <token> INTEGER_NUMBER FLOAT_NUMBER STRING_CONSTANT
%token <token> ASSIGNMENT
%token <token> PRINT READ RETURN
%token <token> IF ELSE WHILE DO
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
%type <ast> return_statement
%type <ast> if_statement
%type <ast> while_statement
%type <ast> block_statement
%type <ast> statement_list
%type <ast> function_def
%type <ast> function_decl
%type <ast> function_call
%type <ast> call_statement
%type <ast> program
%type <ast> external_decl_list
%type <ast> external_decl
%type <dtype> param_type
%type <ast> formal_param
%type <list> formal_param_list
%type <list> argument_list

%right '?' ':'
%left OR
%left AND
%right NOT
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%right UMINUS 
%nonassoc IFX
%nonassoc ELSE

%%

program
        : external_decl_list
            {
            root = $1;
                    if (!sa_parse && !main_defined) {
                            yyerror("main function must be defined");
                            YYERROR;
                    }
            }
        ;

external_decl_list
        : external_decl_list external_decl
            {
            $$ = $1;
            if ($2) {
                Ast *node = $2;
                if (node->kind == AST_PROCEDURE && ((Procedure_Ast *)node)->has_body) {
                    Procedure_Ast *def_proc = (Procedure_Ast *)node;
                    Procedure_Ast *decl_proc = find_matching_decl((Program_Ast *)$$, def_proc);

                    if (decl_proc) {
                        decl_proc->has_body = 1;
                        decl_proc->body = def_proc->body;
                        decl_proc->local_var_bytes = def_proc->local_var_bytes;
                        decl_proc->locals = def_proc->locals;
                    } else {
                        program_append((Program_Ast *)$$, node);
                    }
                } else {
                    program_append((Program_Ast *)$$, node);
                }
            }
            }
        | /* empty */
            {
            $$ = make_program_ast(yylineno);
            }
        ;

external_decl
        : var_decl_stmt
            {
                    $$ = NULL;
            }
        | function_decl
            {
            $$ = $1;
            }
        | function_def
            {
            $$ = $1;
            }
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

function_decl
    : named_type IDENTIFIER '(' formal_param_list ')' ';'
      {
          current_func_return_type = current_decl_type;
          if (!register_function($2.lexeme, current_func_return_type, $4, 0, $2.line)) {
              YYERROR;
          }
          $$ = make_procedure_ast($2.lexeme, current_func_return_type, $4, 0, 0, NULL, $2.line);
          clear_local_scope();
      }
    | named_type IDENTIFIER '(' ')' ';'
      {
          current_func_return_type = current_decl_type;
          if (!register_function($2.lexeme, current_func_return_type, NULL, 0, $2.line)) {
              YYERROR;
          }
          $$ = make_procedure_ast($2.lexeme, current_func_return_type, NULL, 0, 0, NULL, $2.line);
          clear_local_scope();
      }
    ;

function_def
    : named_type IDENTIFIER '(' ')' '{'
      {
          current_func_return_type = current_decl_type;
          if (!register_function($2.lexeme, current_func_return_type, NULL, 1, $2.line)) {
              YYERROR;
          }
          set_scope(LOCAL_SCOPE);
          collecting_function_locals = 1;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
      }
      optional_local_var_decl_stmt_list
      statement_list
      '}'
      {
          Ast *body = $8 ? $8 : make_sequence_ast($2.line);
          Ast *proc = make_procedure_ast($2.lexeme, current_func_return_type, NULL, current_function_local_bytes, 1, body, $2.line);
          ((Procedure_Ast *)proc)->locals = current_function_locals;
          $$ = proc;
          collecting_function_locals = 0;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
          set_scope(GLOBAL_SCOPE);
          clear_local_scope();
      }
    | named_type IDENTIFIER '(' formal_param_list ')' '{'
      {
          current_func_return_type = current_decl_type;
          if (!register_function($2.lexeme, current_func_return_type, $4, 1, $2.line)) {
              YYERROR;
          }
          set_scope(LOCAL_SCOPE);
          collecting_function_locals = 1;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
      }
      optional_local_var_decl_stmt_list
      statement_list
      '}'
      {
          Ast *body = $9 ? $9 : make_sequence_ast($2.line);
          Ast *proc = make_procedure_ast($2.lexeme, current_func_return_type, $4, current_function_local_bytes, 1, body, $2.line);
          ((Procedure_Ast *)proc)->locals = current_function_locals;
          $$ = proc;
          collecting_function_locals = 0;
          current_function_local_bytes = 0;
          current_function_locals = NULL;
          set_scope(GLOBAL_SCOPE);
          clear_local_scope();
      }
    ;

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
          Symbol_Table_Entry *entry = insert_symbol($1.lexeme, current_decl_type);
          if (collecting_function_locals)
          {
              current_function_local_bytes += type_size_bytes(current_decl_type);
              if (entry)
                  append_ast_list_node(&current_function_locals, make_name_ast(entry, $1.line));
          }
      }
    ;

named_type
    : INT    { current_decl_type = INT_TYPE; }
    | FLOAT  { current_decl_type = FLOAT_TYPE; }
    | BOOL   { current_decl_type = BOOL_TYPE; }
    | STRING { current_decl_type = STRING_TYPE; }
    | VOID   { current_decl_type = VOID_TYPE; }
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
    | return_statement      { $$ = $1; }
    | call_statement        { $$ = $1; }
    | if_statement          { $$ = $1; }
    | while_statement       { $$ = $1; }
    | block_statement       { $$ = $1; }
    ;

block_statement
    : '{' statement_list '}'
      {
          if ($2) {
              $$ = $2;
          } else {
              $$ = make_sequence_ast(yylineno);
          }
      }
    ;

if_statement
    : IF '(' expression ')' statement %prec IFX
      {
          $$ = make_if_else_stmt_ast($3, $5, NULL, $1.line);
      }
    | IF '(' expression ')' statement ELSE statement
      {
          $$ = make_if_else_stmt_ast($3, $5, $7, $1.line);
      }
    ;

while_statement
    : WHILE '(' expression ')' statement
      {
          $$ = make_while_ast($3, $5, 0, $1.line);
      }
        | DO statement WHILE '(' expression ')' ';'
            {
                    $$ = make_while_ast($5, $2, 1, $1.line);
            }
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
          if (!sa_parse && !entry) {
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

function_call
    : IDENTIFIER '(' ')'
      {
          if (!sa_parse && !lookup_function($1.lexeme)) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Function '%s' not declared", $1.lexeme);
              yyerror(buf);
              YYERROR;
          }
          $$ = make_call_ast($1.lexeme, NULL, $1.line);
      }
    | IDENTIFIER '(' argument_list ')'
      {
          if (!sa_parse && !lookup_function($1.lexeme)) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Function '%s' not declared", $1.lexeme);
              yyerror(buf);
              YYERROR;
          }
          $$ = make_call_ast($1.lexeme, $3, $1.line);
      }
    ;

argument_list
    : argument_list ',' expression
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
    | expression
      {
          Ast_List *new_node = malloc(sizeof(Ast_List));
          new_node->stmt = $1;
          new_node->next = NULL;
          $$ = new_node;
      }
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
        | function_call
            {
                    $$ = $1;
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
          if (!sa_parse && !entry) {
              char buf[256];
              snprintf(buf, sizeof(buf), "Variable '%s' not declared", $2.lexeme);
              yyerror(buf);
              YYERROR;
          }
          Ast *var = make_name_ast(entry, $2.line);
          $$ = make_read_ast(var, $1.line);
      }
    ;

return_statement
        : RETURN ';'
            {
                    $$ = make_return_ast(NULL, $1.line);
            }
        | RETURN expression ';'
            {
                    $$ = make_return_ast($2, $1.line);
            }
        ;

call_statement
        : function_call ';'
            {
            Call_Ast *call = (Call_Ast *)$1;
            if (call) {
                call->used_as_statement = 1;
            }
            $$ = $1;
            }
        ;

%%

void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
