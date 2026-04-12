#ifndef SPIM_H
#define SPIM_H

#include <stdio.h>
#include "ast.h"

/* Generate SPIM-style assembly from in-memory RTL for a checked AST. */
void spim_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes SPIM-style assembly to a file path. */
int spim_generate_to_path(Ast *root, const char *path);

/* Reset counters for deterministic output. */
void spim_reset_counters(void);

#endif
