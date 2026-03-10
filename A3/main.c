#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symbol_table.h"
#include "tac.h"

int yyparse(void);
int yylex(void);
extern FILE *yyin;
extern Ast *root;
extern int yylineno;

int dump_tokens = 0;
FILE *tokfile = NULL;
FILE *astfile = NULL;
FILE *tacfile = NULL;
int sa_scan = 0;
int sa_parse = 0;
int sa_ast = 0;

void dump_token(const char *type, const char *lexeme, int line)
{
    if (!dump_tokens || !tokfile)
        return;

    fprintf(tokfile, "%-15s %-20s line %d\n", type, lexeme, line);
}

int main(int argc, char **argv)
{
    const char *input_file = NULL;
    int show_ast = 0;
    int show_tac = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [--show-tokens] [--show-ast] [--show-tac] [--sa-scan] [--sa-parse] <source-file>\n", argv[0]);
        return 1;
    }

    /* Parse all arguments to extract flags and input file */
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            /* It's a flag */
            if (strcmp(argv[i], "--show-tokens") == 0)
            {
                dump_tokens = 1;
            }
            else if (strcmp(argv[i], "--show-ast") == 0)
            {
                show_ast = 1;
            }
            else if (strcmp(argv[i], "--sa-scan") == 0)
            {
                sa_scan = 1;
                // dump_tokens = 1; /* Scan mode also dumps tokens */
            }
            else if (strcmp(argv[i], "--show-tac") == 0)
            {
                show_tac = 1;
            }
            else if (strcmp(argv[i], "--sa-parse") == 0)
            {
                sa_parse = 1;
            }
            else
            {
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
                return 1;
            }
        }
        else
        {
            /* It's the input file (last non-flag argument) */
            input_file = argv[i];
        }
    }

    if (!input_file)
    {
        fprintf(stderr, "Error: No input file specified\n");
        fprintf(stderr, "Usage: %s [--show-tokens] [--show-ast] [--show-tac] [--sa-scan] [--sa-parse] <source-file>\n", argv[0]);
        return 1;
    }

    /* Priority: if sa-scan is set, show-ast/show-tac are irrelevant (we stop at scanning) */
    /* Priority: if sa-parse is set, show-tac is irrelevant (no semantic analysis) */
    if (sa_scan)
    {
        show_ast = 0; /* Ignore show-ast if we're only scanning */
        show_tac = 0;
    }
    else if (sa_parse)
    {
        show_tac = 0; /* TAC requires full semantic analysis */
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

    if (show_ast)
    {
        size_t path_len = strlen(input_file) + strlen(".ast") + 1;
        char *astname = malloc(path_len);
        if (!astname)
        {
            fprintf(stderr, "Out of memory\n");
            fclose(yyin);
            return 1;
        }

        snprintf(astname, path_len, "%s.ast", input_file);
        astfile = fopen(astname, "w");
        free(astname);

        if (!astfile)
        {
            perror("astfile");
            fclose(yyin);
            return 1;
        }
    }

    init_symbol_table();

    /* Scan-only mode: just tokenize and stop */
    if (sa_scan)
    {
        int token;
        while ((token = yylex()) != 0)
        {
            /* yylex() handles token dumping via dump_token() */
        }
    }
    /* Parse-only mode: tokenize and parse, but stop before semantic analysis */
    else if (sa_parse)
    {
        if (yyparse() == 0 && root)
        {
            /* Stop after parsing, optionally show AST if requested */
            if (show_ast)
                root->print(root, astfile);
        }
        else
        {
            destroy_symbol_table();
            if (tokfile)
                fclose(tokfile);
            if (astfile)
                fclose(astfile);
            fclose(yyin);
            return 1;
        }
    }
    /* Normal mode: full compilation with semantic checks */
    else
    {
        if (yyparse() == 0 && root)
        {
            /* Perform semantic checks */
            if (check_ast(root))
            {
                if (show_ast)
                    root->print(root, astfile);
                if (show_tac){
                    tac_generate(root, stdout);
                }
                    
            }
            else
            {
                destroy_symbol_table();
                if (tokfile)
                    fclose(tokfile);
                if (astfile)
                    fclose(astfile);
                fclose(yyin);
                return 1;
            }
        }
        else
        {
            destroy_symbol_table();
            if (tokfile)
                fclose(tokfile);
            if (astfile)
                fclose(astfile);
            fclose(yyin);
            return 1;
        }
    }

    if (tokfile)
        fclose(tokfile);
    if (astfile)
        fclose(astfile);

    destroy_symbol_table();
    fclose(yyin);

    return 0;
}
