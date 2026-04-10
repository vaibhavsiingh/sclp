#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

/* -------- Scope Type -------- */

typedef enum
{
    GLOBAL_SCOPE,
    LOCAL_SCOPE
} Scope_Type;

/* -------- Symbol Entry -------- */

typedef struct Symbol_Table_Entry
{
    char *name;
    Data_Type type;
    Scope_Type scope;

    struct Symbol_Table_Entry *next;
} Symbol_Table_Entry;

/* -------- Function Signatures -------- */

typedef struct Param_Type_List
{
    Data_Type type;
    struct Param_Type_List *next;
} Param_Type_List;

typedef struct Function_Entry
{
    char *name;
    Data_Type return_type;
    Param_Type_List *params;
    int defined;

    struct Function_Entry *next;
} Function_Entry;

/* -------- API -------- */

void init_symbol_table();
void destroy_symbol_table();

void set_scope(Scope_Type scope);

Symbol_Table_Entry *insert_symbol(char *name, Data_Type type);
Symbol_Table_Entry *lookup_symbol(char *name);
Symbol_Table_Entry *lookup_symbol_any_scope(const char *name);
void update_symbol_scope(Symbol_Table_Entry *entry, Scope_Type new_scope);

void clear_local_scope();

void print_symbol_table();

/* -------- Function Table API -------- */

Function_Entry *lookup_function(const char *name);
Function_Entry *add_function_entry(const char *name, Data_Type return_type,
                                   Param_Type_List *params, int defined);

Param_Type_List *param_types_from_ast_list(Ast_List *params);
int compare_param_type_lists(const Param_Type_List *a, const Param_Type_List *b);
void free_param_type_list(Param_Type_List *params);

#endif
