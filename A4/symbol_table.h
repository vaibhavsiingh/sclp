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

/* -------- API -------- */

void init_symbol_table();
void destroy_symbol_table();

void set_scope(Scope_Type scope);

Symbol_Table_Entry *insert_symbol(char *name, Data_Type type);
Symbol_Table_Entry *lookup_symbol(char *name);
void update_symbol_scope(Symbol_Table_Entry *entry, Scope_Type new_scope);

void clear_local_scope();

void print_symbol_table();

#endif
