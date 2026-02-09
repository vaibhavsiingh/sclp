%{
#include "y.tab.h"
#include <string.h>
#include <stdlib.h>

extern int yylineno;
extern int dump_tokens;
extern FILE *tokfile;

void dump_token(const char *type, const char *lexeme, int line);
%}

digit      [0-9]
alphabet   [a-zA-Z_]
whitespace [ \t\r]

%%
\n                  { yylineno++; }
{whitespace}+       ;

"//".*              ;


"int" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("INT", yytext, yylineno);
    return INT;
}

"float" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("FLOAT", yytext, yylineno);
    return FLOAT;
}

"bool" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("BOOL", yytext, yylineno);
    return BOOL;
}

"string" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("STRING", yytext, yylineno);
    return STRING;
}

"void" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("VOID", yytext, yylineno);
    return VOID;
}

"=" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("ASSIGNMENT", yytext, yylineno);
    return ASSIGNMENT;
}

"read" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("READ", yytext, yylineno);
    return READ;
}

"print" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("PRINT", yytext, yylineno);
    return PRINT;
}

{alphabet}({alphabet}|{digit})* {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("IDENTIFIER", yytext, yylineno);
    return IDENTIFIER;
}

{digit}+\.{digit}* {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("FLOAT_NUMBER", yytext, yylineno);
    return FLOAT_NUMBER;
}

{digit}+ {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("INTEGER_NUMBER", yytext, yylineno);
    return INTEGER_NUMBER;
}

\"([^\\\"\n]|\\[nt\\])*\" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("STRING_CONST", yytext, yylineno);
    return STRING_CONSTANT;
}




[-+*/(),;{}?:] {
    char buf[2] = {yytext[0], '\0'};
    dump_token("SYMBOL", buf, yylineno);
    return yytext[0];
}



. {
    printf("Lexical error at line %d: %s\n", yylineno, yytext);
}
%%

int yywrap(void) {
    return 1;
}
