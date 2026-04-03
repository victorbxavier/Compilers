%{
/* C code / includes / declarations */
#include <stdio.h>
%}


/* Rules: pattern   action */
%%
[0-9]+          {printf("Number: %s\n", yytext); }
"+"             {printf("PLUS\n");}
"-"             {printf("MINUS\n");}
"*"             {printf("MULTIPLY\n");}
"/"             {printf("DIVIDE\n");}
"="             {printf("ASSIGN\n");}
[a-zA-Z][a-zA-Z0-9]*  { printf("Id: %s\n", yytext); }
[ \t\n]         { /* ignore whitespace */ }
.               { printf("UNKNOWN: %s\n", yytext); }
%%

/* C functions */

int yywrap() {return 1;}

int main(){
    yylex();
    return 0;
}
