#ifndef RTL_H
#define RTL_H

#include <stdio.h>
#include "ast.h"

/* Generate RTL for a checked AST and write it to the provided stream. */
void rtl_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes RTL to a file path. Returns 1 on success. */
int rtl_generate_to_path(Ast *root, const char *path);

/* Reset virtual register and label counters for deterministic output. */
void rtl_reset_counters(void);

#endif
