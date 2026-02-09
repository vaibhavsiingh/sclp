%{
    #include "y.tab.h"
%}

digit [0-9]

%%
[a-z]       {yylval = *yytext-'a'; return VARIABLE;}
{digit}+    {yylval = atoi(yytext); return INTEGER;}
[-+/()=*\n]      {return *yytext;}
[ \t]       ;
.           {printf("Invalid");}

%%
int yyerror(char *s){
    printf("ERROR: %s", s);
}

int yywrap(){
    return 1;
}
