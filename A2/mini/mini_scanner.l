%{
#include "mini_parser.tab.h"
#include <string.h>
#include <stdlib.h>


void dump_token(const char *type, const char *lexeme, int line) {}
%}



digit      [0-9]
alphabet   [a-zA-Z_]
whitespace [ \t\r]

%%
\n                  { yylineno++; }
{whitespace}+       ;
"//".*              ;

"int"     { yylval.token.lexeme=strdup(yytext); return INT; }
"float"   { yylval.token.lexeme=strdup(yytext); return FLOAT; }
"bool"    { yylval.token.lexeme=strdup(yytext); return BOOL; }
"string"  { yylval.token.lexeme=strdup(yytext); return STRING; }
"void"    { yylval.token.lexeme=strdup(yytext); return VOID; }

"="       { yylval.token.lexeme=strdup(yytext); return ASSIGNMENT; }

"read"    { return READ; }
"print"   { return PRINT; }

{alphabet}({alphabet}|{digit})* {
    yylval.token.lexeme = strdup(yytext);
    return IDENTIFIER;
}

{digit}+\.{digit}* { return FLOAT_NUMBER; }
{digit}+           { return INTEGER_NUMBER; }

\"([^\\\"\n]|\\[nt\\])*\" { return STRING_CONSTANT; }

[-+*/(),;{}] { return yytext[0]; }

. {
    printf("Lexical error at line %d: %s\n", yylineno, yytext);
}
%%

int yywrap(void) { return 1; }
