#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symbol_table.h"

int yyparse(void);
extern FILE *yyin;
extern Ast *root;

int dump_tokens = 0;
FILE *tokfile = NULL;

void dump_token(const char *type, const char *lexeme, int line)
{
    if (!dump_tokens || !tokfile)
        return;

    fprintf(tokfile, "%-15s %-20s line %d\n", type, lexeme, line);
}

int main(int argc, char **argv)
{
    const char *input_file = NULL;

    if (argc == 2)
    {
        input_file = argv[1];
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], "--show-tokens") != 0)
        {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            return 1;
        }
        dump_tokens = 1;
        input_file = argv[2];
    }
    else
    {
        fprintf(stderr, "Usage: %s [--show-tokens] <source-file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(input_file, "r");
    if (!yyin)
    {
        perror("fopen");
        return 1;
    }

    if (dump_tokens)
    {
        size_t path_len = strlen(input_file) + strlen(".toks") + 1;
        char *tokname = malloc(path_len);
        if (!tokname)
        {
            fprintf(stderr, "Out of memory\n");
            fclose(yyin);
            return 1;
        }

        snprintf(tokname, path_len, "%s.toks", input_file);
        tokfile = fopen(tokname, "w");
        free(tokname);

        if (!tokfile)
        {
            perror("tokfile");
            fclose(yyin);
            return 1;
        }
    }

    init_symbol_table();

    if (yyparse() == 0 && root)
    {
        if (check_ast(root))
            root->print(root, stdout);
        else
            return 1;
    }

    if (tokfile)
        fclose(tokfile);

    destroy_symbol_table();
    fclose(yyin);

    return 0;
}
