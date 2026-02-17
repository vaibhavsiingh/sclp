#include <stdio.h>
#include "ast.h"

extern int yyparse(void);
extern ProcedureAst* root;

int main(void) {
    if (yyparse() == 0) {
        printf("Parse successful!\n\n");
        print_ast((Ast*)root, 0);
    } else {
        printf("Parse failed.\n");
    }
    return 0;
}
