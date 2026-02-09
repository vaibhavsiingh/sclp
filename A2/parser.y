%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;

/* token dump control */
int dump_tokens = 0;
FILE *tokfile = NULL;
int main_seen = 0;
int sa_scan = 0;


void dump_token(const char *type, const char *lexeme, int line);
%}

%union {
    struct {
        char *lexeme;
        int line;
    } token;
}

%token <token> INT FLOAT BOOL STRING VOID
%token <token> IDENTIFIER
%token <token> INTEGER_NUMBER FLOAT_NUMBER STRING_CONSTANT
%token <token> ASSIGNMENT
%token <token> PRINT READ

%type <token> named_type
%type <token> variable_name
%type <token> variable_as_operand


%left '+' '-'
%left '*' '/'
%right UMINUS

%%

program
    : global_decl_statement_list void_main_def
    | void_main_def
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
	: func_header '(' formal_param_list ')' ';'    
	| func_header '('')' ';'                       
;   

func_header
	: named_type IDENTIFIER                              
;

formal_param_list
	: formal_param_list ',' formal_param           
	| formal_param                                 
;

formal_param
	: param_type IDENTIFIER                              
;


param_type
	: INT                                                                              
	| FLOAT                                                                                  
	| BOOL                                                                                   
	| STRING                                                                                 
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

named_type
    : INT
    | FLOAT
    | BOOL
    | STRING
    | VOID
    ;

void_main_def
    : func_header IDENTIFIER '(' ')' '{'        
      optional_local_var_decl_stmt_list
      statement_list
      '}'
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
    | /* empty */
    ;

statement
    : assignment_statement
    | print_statement
    | read_statement
    ;

assignment_statement
    : variable_as_operand ASSIGNMENT expression ';'
    ;

variable_as_operand
    : variable_name
    ;

variable_name
    : IDENTIFIER
    ;

expression
    : expression '+' expression
    | expression '-' expression
    | expression '*' expression
    | expression '/' expression
    | '-' expression    %prec UMINUS
    | '(' expression ')' 
    | variable_as_operand
    | constant_as_operand
    ;

constant_as_operand
    : INTEGER_NUMBER
    | FLOAT_NUMBER
    | STRING_CONSTANT
    ;

print_statement
    : PRINT expression ';'
    ;

read_statement
    : READ variable_name ';'
    ;

%%

void dump_token(const char *type, const char *lexeme, int line) {
    if (!dump_tokens) return;
    fprintf(tokfile, "%-15s %-20s line %d\n", type, lexeme, line);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s [--show-tokens | --sa-scan] <input_file>\n", argv[0]);
        return 1;
    }

    char *input_file = NULL;

    if (argc == 3) {
        if (strcmp(argv[1], "--show-tokens") == 0) {
            dump_tokens = 1;
        } else if (strcmp(argv[1], "--sa-scan") == 0) {
            sa_scan = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            return 1;
        }
        input_file = argv[2];
    } else {
        input_file = argv[1];
    }

    yyin = fopen(input_file, "r");
    if (!yyin) {
        perror(input_file);
        return 1;
    }

    if (dump_tokens) {
        char tokname[512];
        snprintf(tokname, sizeof(tokname), "%s.toks", input_file);
        tokfile = fopen(tokname, "w");
        if (!tokfile) {
            perror(tokname);
            return 1;
        }
    }

    int res = 0;

    if (sa_scan) {        
        while (yylex() != 0);
        res = 0;
    } else {        
        res = yyparse();
    }

    if (tokfile) fclose(tokfile);
    fclose(yyin);

    return res;
}

void yyerror(const char *s) {
    printf("Parse error at line %d: %s\n", yylineno, s);
}
