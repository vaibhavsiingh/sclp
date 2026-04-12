#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

/* -------- Two Separate Lists -------- */

static Symbol_Table_Entry *global_table = NULL;
static Symbol_Table_Entry *local_table = NULL;
static Symbol_Table_Entry *archived_locals = NULL;

static Function_Entry *function_table = NULL;

static Scope_Type current_scope = GLOBAL_SCOPE;

static void append_symbol_entry(Symbol_Table_Entry **table, Symbol_Table_Entry *entry)
{
    Symbol_Table_Entry *tail;

    if (!table || !entry)
        return;

    entry->next = NULL;
    if (!*table)
    {
        *table = entry;
        return;
    }

    tail = *table;
    while (tail->next)
        tail = tail->next;
    tail->next = entry;
}

/* -------- Initialization -------- */

void init_symbol_table()
{
    global_table = NULL;
    local_table = NULL;
    archived_locals = NULL;
    function_table = NULL;
    current_scope = GLOBAL_SCOPE;
}

static void free_symbol_list(Symbol_Table_Entry *entry)
{
    while (entry)
    {
        Symbol_Table_Entry *temp = entry;
        entry = entry->next;
        free(temp->name);
        free(temp);
    }
}

void destroy_symbol_table()
{
    clear_local_scope();

    free_symbol_list(local_table);
    free_symbol_list(archived_locals);
    free_symbol_list(global_table);

    local_table = NULL;
    archived_locals = NULL;
    global_table = NULL;

    Function_Entry *fn = function_table;
    while (fn)
    {
        Function_Entry *temp = fn;
        fn = fn->next;
        free(temp->name);
        free_param_type_list(temp->params);
        free(temp);
    }
    function_table = NULL;
}

/* -------- Scope Handling -------- */

void set_scope(Scope_Type scope)
{
    current_scope = scope;
}

/* -------- Local Scope Reset -------- */

void clear_local_scope()
{
    if (!local_table)
        return;

    Symbol_Table_Entry *tail = local_table;
    while (tail->next)
        tail = tail->next;

    tail->next = archived_locals;
    archived_locals = local_table;
    local_table = NULL;
}

/* -------- Lookup -------- */

Symbol_Table_Entry *lookup_symbol(char *name)
{
    /* Check local first */
    Symbol_Table_Entry *entry = local_table;
    while (entry)
    {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    /* Then global */
    entry = global_table;
    while (entry)
    {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    return NULL;
}

Symbol_Table_Entry *lookup_symbol_any_scope(const char *name)
{
    Symbol_Table_Entry *entry;

    if (!name)
        return NULL;

    entry = local_table;
    while (entry)
    {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    entry = global_table;
    while (entry)
    {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    entry = archived_locals;
    while (entry)
    {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    return NULL;
}

/* -------- Insert -------- */

Symbol_Table_Entry *insert_symbol(char *name, Data_Type type)
{
    Symbol_Table_Entry **table =
        (current_scope == GLOBAL_SCOPE)
            ? &global_table
            : &local_table;

    /* Prevent redeclaration in same scope */
    Symbol_Table_Entry *temp = *table;
    while (temp)
    {
        if (strcmp(temp->name, name) == 0)
        {
            printf("Error: Redeclaration of %s\n", name);
            return NULL;
        }
        temp = temp->next;
    }

    Symbol_Table_Entry *entry = malloc(sizeof(Symbol_Table_Entry));
    entry->name = strdup(name);
    entry->type = type;
    entry->scope = current_scope;

    if (current_scope == GLOBAL_SCOPE)
        append_symbol_entry(table, entry);
    else
    {
        entry->next = *table;
        *table = entry;
    }

    return entry;
}

/* -------- Update Scope -------- */

void update_symbol_scope(Symbol_Table_Entry *entry, Scope_Type new_scope)
{
    if (!entry)
        return;

    /* Remove from current table */
    Symbol_Table_Entry **old_table =
        (entry->scope == GLOBAL_SCOPE) ? &global_table : &local_table;

    Symbol_Table_Entry **ptr = old_table;
    while (*ptr && *ptr != entry)
    {
        ptr = &((*ptr)->next);
    }

    if (*ptr)
    {
        *ptr = entry->next;
    }

    /* Update scope */
    entry->scope = new_scope;

    /* Insert into new table */
    Symbol_Table_Entry **new_table =
        (new_scope == GLOBAL_SCOPE) ? &global_table : &local_table;

    if (new_scope == GLOBAL_SCOPE)
        append_symbol_entry(new_table, entry);
    else
    {
        entry->next = *new_table;
        *new_table = entry;
    }
}

Symbol_Table_Entry *get_global_symbol_table(void)
{
    return global_table;
}

/* -------- Debug Print -------- */

void print_symbol_table()
{
    Symbol_Table_Entry *entry;

    printf("----- Global Scope -----\n");
    entry = global_table;
    while (entry)
    {
        printf("%s (type=%d)\n", entry->name, entry->type);
        entry = entry->next;
    }

    printf("----- Local Scope ------\n");
    entry = local_table;
    while (entry)
    {
        printf("%s (type=%d)\n", entry->name, entry->type);
        entry = entry->next;
    }

    printf("------------------------\n");
}

/* -------- Function Table -------- */

Function_Entry *lookup_function(const char *name)
{
    Function_Entry *cur = function_table;
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

Function_Entry *add_function_entry(const char *name, Data_Type return_type,
                                   Param_Type_List *params, int defined)
{
    Function_Entry *entry = malloc(sizeof(Function_Entry));
    if (!entry)
        return NULL;

    entry->name = strdup(name);
    entry->return_type = return_type;
    entry->params = params;
    entry->defined = defined;

    entry->next = function_table;
    function_table = entry;
    return entry;
}

Param_Type_List *param_types_from_ast_list(Ast_List *params)
{
    Param_Type_List *head = NULL;
    Param_Type_List *tail = NULL;

    while (params)
    {
        Name_Ast *param = (Name_Ast *)params->stmt;
        Param_Type_List *node = malloc(sizeof(Param_Type_List));
        if (!node)
            break;
        node->type = param && param->entry ? param->entry->type : VOID_TYPE;
        node->next = NULL;

        if (!head)
        {
            head = node;
            tail = node;
        }
        else
        {
            tail->next = node;
            tail = node;
        }

        params = params->next;
    }

    return head;
}

int compare_param_type_lists(const Param_Type_List *a, const Param_Type_List *b)
{
    const Param_Type_List *pa = a;
    const Param_Type_List *pb = b;

    while (pa && pb)
    {
        if (pa->type != pb->type)
            return 0;
        pa = pa->next;
        pb = pb->next;
    }

    return pa == NULL && pb == NULL;
}

void free_param_type_list(Param_Type_List *params)
{
    while (params)
    {
        Param_Type_List *temp = params;
        params = params->next;
        free(temp);
    }
}
