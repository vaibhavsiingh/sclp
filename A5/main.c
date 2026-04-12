#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symbol_table.h"
#include "tac.h"
#include "rtl.h"
#include "spim.h"

int yyparse(void);
int yylex(void);
extern FILE *yyin;
extern Ast *root;
extern int yylineno;

int dump_tokens = 0;
FILE *tokfile = NULL;
FILE *astfile = NULL;
FILE *tacfile = NULL;
FILE *rtlfile = NULL;
FILE *spimfile = NULL;
int sa_scan = 0;
int sa_parse = 0;
int sa_ast = 0;
int sa_tac = 0;
int sa_rtl = 0;

typedef enum
{
    STAGE_SCAN,
    STAGE_PARSE,
    STAGE_AST,
    STAGE_TAC,
    STAGE_RTL,
    STAGE_SPIM
} Compile_Stage;

static void close_outputs(void)
{
    if (tokfile)
        fclose(tokfile);
    if (astfile)
        fclose(astfile);
    if (tacfile)
        fclose(tacfile);
    if (rtlfile)
        fclose(rtlfile);
    if (spimfile)
        fclose(spimfile);

    tokfile = NULL;
    astfile = NULL;
    tacfile = NULL;
    rtlfile = NULL;
    spimfile = NULL;
}

static FILE *open_output_with_suffix(const char *input_file, const char *suffix, const char *label)
{
    size_t path_len = strlen(input_file) + strlen(suffix) + 1;
    char *name = malloc(path_len);
    FILE *fp;

    if (!name)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    snprintf(name, path_len, "%s%s", input_file, suffix);
    fp = fopen(name, "w");
    free(name);

    if (!fp)
        perror(label);

    return fp;
}

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
    int show_rtl = 0;
    int show_spim = 0;
    Compile_Stage stop_stage = STAGE_SPIM;
    int parse_ok = 0;
    int semantic_ok = 1;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [--show-tokens] [--show-ast] [--show-tac] [--show-rtl] [--show-spim] [--show-asm] [--sa-scan] [--sa-parse] [--sa-ast] [--sa-tac] [--sa-rtl] <source-file>\n", argv[0]);
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
            else if (strcmp(argv[i], "--show-rtl") == 0)
            {
                show_rtl = 1;
            }
            else if (strcmp(argv[i], "--show-spim") == 0 || strcmp(argv[i], "--show-asm") == 0)
            {
                show_spim = 1;
            }
            else if (strcmp(argv[i], "--sa-parse") == 0)
            {
                sa_parse = 1;
            }
            else if (strcmp(argv[i], "--sa-ast") == 0)
            {
                sa_ast = 1;
            }
            else if (strcmp(argv[i], "--sa-tac") == 0)
            {
                sa_tac = 1;
            }
            else if (strcmp(argv[i], "--sa-rtl") == 0)
            {
                sa_rtl = 1;
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
        fprintf(stderr, "Usage: %s [--show-tokens] [--show-ast] [--show-tac] [--show-rtl] [--show-spim] [--show-asm] [--sa-scan] [--sa-parse] [--sa-ast] [--sa-tac] [--sa-rtl] <source-file>\n", argv[0]);
        return 1;
    }

    if (sa_scan)
        stop_stage = STAGE_SCAN;
    else if (sa_parse)
        stop_stage = STAGE_PARSE;
    else if (sa_ast)
        stop_stage = STAGE_AST;
    else if (sa_tac)
        stop_stage = STAGE_TAC;
    else if (sa_rtl)
        stop_stage = STAGE_RTL;

    yyin = fopen(input_file, "r");
    if (!yyin)
    {
        perror("fopen");
        return 1;
    }

    if (dump_tokens)
    {
        tokfile = open_output_with_suffix(input_file, ".toks", "tokfile");

        if (!tokfile)
        {
            fclose(yyin);
            return 1;
        }
    }

    init_symbol_table();

    /* Stage 1: scan */
    if (stop_stage == STAGE_SCAN)
    {
        int token;
        while ((token = yylex()) != 0)
        {
            /* yylex() handles token dumping via dump_token() */
        }
    }
    else
    {
        /* Stage 2: parse */
        parse_ok = (yyparse() == 0 && root);
        if (!parse_ok)
            semantic_ok = 0;

        if (semantic_ok && show_ast)
        {
            astfile = open_output_with_suffix(input_file, ".ast", "astfile");
            if (!astfile)
            {
                semantic_ok = 0;
            }
            else
            {
                root->print(root, astfile);
            }
        }

        if (semantic_ok && stop_stage != STAGE_PARSE)
        {
            /* Stage 3: AST semantic checks */
            semantic_ok = check_ast(root);

            /* Stage 4: TAC */
            if (semantic_ok && stop_stage != STAGE_AST && show_tac)
            {
                tacfile = open_output_with_suffix(input_file, ".tac", "tacfile");
                if (!tacfile)
                {
                    semantic_ok = 0;
                }
                else
                {
                    tac_generate(root, tacfile);
                }
            }

            /* Stage 5: RTL */
            if (semantic_ok && stop_stage != STAGE_AST && stop_stage != STAGE_TAC && show_rtl)
            {
                rtlfile = open_output_with_suffix(input_file, ".rtl", "rtlfile");
                if (!rtlfile)
                {
                    semantic_ok = 0;
                }
                else
                {
                    rtl_generate(root, rtlfile);
                }
            }

            /* Stage 6: SPIM/ASM */
            if (semantic_ok && stop_stage != STAGE_AST && stop_stage != STAGE_TAC && show_spim)
            {
                spimfile = open_output_with_suffix(input_file, ".spim", "spimfile");
                if (!spimfile)
                {
                    semantic_ok = 0;
                }
                else
                {
                    spim_generate(root, spimfile);
                }
            }
        }
    }

    destroy_symbol_table();
    close_outputs();
    fclose(yyin);

    return semantic_ok ? 0 : 1;
}
