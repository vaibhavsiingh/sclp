#ifndef SPIM_H
#define SPIM_H

#include <stdio.h>
#include "ast.h"

/* Generate SPIM assembly for a checked AST and write it to the provided stream. */
void spim_generate(Ast *root, FILE *out);

/* Convenience wrapper that writes SPIM assembly to a file path. Returns 1 on success. */
int spim_generate_to_path(Ast *root, const char *path);

#endif
