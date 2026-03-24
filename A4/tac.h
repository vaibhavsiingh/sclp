#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include "ast.h"

/* Generate TAC for a checked AST and write it to the provided stream. */
void tac_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes TAC to a file path. Returns 1 on success. */
int tac_generate_to_path(Ast *root, const char *path);

/* Reset temporary and label counters for deterministic output across runs. */
void tac_reset_counters(void);

#endif
