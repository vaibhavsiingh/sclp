#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

/* -------- Two Separate Lists -------- */

static Symbol_Table_Entry *global_table = NULL;
static Symbol_Table_Entry *local_table = NULL;

static Scope_Type current_scope = GLOBAL_SCOPE;

/* -------- Initialization -------- */

void init_symbol_table() {
    global_table = NULL;
    local_table = NULL;
    current_scope = GLOBAL_SCOPE;
}

void destroy_symbol_table() {
    clear_local_scope();

    Symbol_Table_Entry *entry = global_table;
    while (entry) {
        Symbol_Table_Entry *temp = entry;
        entry = entry->next;
        free(temp->name);
        free(temp);
    }

    global_table = NULL;
}

/* -------- Scope Handling -------- */

void set_scope(Scope_Type scope) {
    current_scope = scope;
}

/* -------- Local Scope Reset -------- */

void clear_local_scope() {
    Symbol_Table_Entry *entry = local_table;
    while (entry) {
        Symbol_Table_Entry *temp = entry;
        entry = entry->next;
        free(temp->name);
        free(temp);
    }
    local_table = NULL;
}

/* -------- Lookup -------- */

Symbol_Table_Entry *lookup_symbol(char *name) {
    /* Check local first */
    Symbol_Table_Entry *entry = local_table;
    while (entry) {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    /* Then global */
    entry = global_table;
    while (entry) {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }

    return NULL;
}

/* -------- Insert -------- */

Symbol_Table_Entry *insert_symbol(char *name, Data_Type type) {

    Symbol_Table_Entry **table =
        (current_scope == GLOBAL_SCOPE)
            ? &global_table
            : &local_table;

    /* Prevent redeclaration in same scope */
    Symbol_Table_Entry *temp = *table;
    while (temp) {
        if (strcmp(temp->name, name) == 0) {
            printf("Error: Redeclaration of %s\n", name);
            return NULL;
        }
        temp = temp->next;
    }

    Symbol_Table_Entry *entry = malloc(sizeof(Symbol_Table_Entry));
    entry->name = strdup(name);
    entry->type = type;
    entry->scope = current_scope;

    entry->next = *table;
    *table = entry;

    return entry;
}

/* -------- Debug Print -------- */

void print_symbol_table() {
    Symbol_Table_Entry *entry;

    printf("----- Global Scope -----\n");
    entry = global_table;
    while (entry) {
        printf("%s (type=%d)\n", entry->name, entry->type);
        entry = entry->next;
    }

    printf("----- Local Scope ------\n");
    entry = local_table;
    while (entry) {
        printf("%s (type=%d)\n", entry->name, entry->type);
        entry = entry->next;
    }

    printf("------------------------\n");
}
