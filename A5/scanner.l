%{
#include "ast.h"
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

"==" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("EQ", yytext, yylineno);
    return EQ;
}

"!=" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("NE", yytext, yylineno);
    return NE;
}

"!" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("NOT", yytext, yylineno);
    return NOT;
}

">=" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("GE", yytext, yylineno);
    return GE;
}

"<=" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("LE", yytext, yylineno);
    return LE;
}

">" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("GT", yytext, yylineno);
    return GT;
}

"<" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("LT", yytext, yylineno);
    return LT;
}

"&&" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("AND", yytext, yylineno);
    return AND;
}

"||" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("OR", yytext, yylineno);
    return OR;
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

"return" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("RETURN", yytext, yylineno);
    return RETURN;
}

"if" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("IF", yytext, yylineno);
    return IF;
}

"else" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("ELSE", yytext, yylineno);
    return ELSE;
}

"while" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("WHILE", yytext, yylineno);
    return WHILE;
}

"do" {
    yylval.token.lexeme = strdup(yytext);
    yylval.token.line = yylineno;
    dump_token("DO", yytext, yylineno);
    return DO;
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
