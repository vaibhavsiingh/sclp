#ifndef AST_PRINT_H
#define AST_PRINT_H

#include <stdio.h>
#include "ast.h"

void print_number_ast(Ast *ast, FILE *file);
void print_name_ast(Ast *ast, FILE *file);
void print_binary_ast(Ast *ast, FILE *file);
void print_unary_ast(Ast *ast, FILE *file);
void print_assignment_ast(Ast *ast, FILE *file);
void print_relational_ast(Ast *ast, FILE *file);
void print_logical_ast(Ast *ast, FILE *file);
void print_if_ast(Ast *ast, FILE *file);
void print_if_else_stmt_ast(Ast *ast, FILE *file);
void print_while_ast(Ast *ast, FILE *file);
void print_return_ast(Ast *ast, FILE *file);
void print_read_ast(Ast *ast, FILE *file);
void print_print_ast(Ast *ast, FILE *file);
void print_sequence_ast(Ast *ast, FILE *file);
void print_procedure_ast(Ast *ast, FILE *file);
void print_program_ast(Ast *ast, FILE *file);

#endif
