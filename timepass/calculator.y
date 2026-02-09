%token INTEGER VARIABLE
%{
	#include<stdio.h>
	int yylex(void);
	int sym[26];
%}

%left '+' '-'
%left '*' '/'
%%
program: 	program statement '\n'	
         	|
			;
statement:	expr				{printf("Value of expr: %d\n", $1);}
			| VARIABLE '=' expr	{sym[$1] = $3;}
			;
expr:		INTEGER				{$$ = $1;}
			| VARIABLE			{$$ = sym[$1];}
			| expr '-' expr		{$$ = $1 - $3;}
			| expr '+' expr		{$$ = $1 + $3;}
			| expr '*' expr		{$$ = $1 * $3;}
			| expr '/' expr		{$$ = $1 / $3;}
			| '(' expr ')'		{$$ = $2;}
			;


%%



int main(){
	return yyparse();
}
