%option c++
%{
/* C code / includes / declarations */
#include <iostream>
int chars = 0;
int words = 0;
int lines = 0;
%}


/* Rules: pattern   action */
%%
// Symbols
[0-9]+          {printf("Number: %s\n", yytext); }
"+"             {printf("PLUS\n");}
"-"             {printf("MINUS\n");}
"*"             {printf("MULTIPLY\n");}
"="             {printf("ASSIGN\n");}
">"             {printf("GT\n");}
"<"             {printf("LT\n");}
"("             {printf("OPEN_PARENTHESIS\n");}
")"             {printf("CLOSE_PARENTHESIS\n");}
"["             {printf("OPEN_BRACKET\n");}
"]"             {printf("CLOSE_BRACKET\n");}
"{"             {printf("OPEN_BRACES\n");}
"}"             {printf("CLOSE_BRACES\n");}
";"             {printf("SEMICOLON\n");}
"&&"             {printf("AND\n");}
"."             {printf("DOT\n");}
"!"             {printf("LEQ\n");}
// Special Words
"class"         {printf("CLASS\n");}
"public"        {printf("PUBLIC\n");}
"static"        {printf("STATIC\n");}
"void"          {printf("VOID\n");}
"main"          {printf("MAIN\n");}
"String"        {printf("STRING\n");}
"return"        {printf("RETURN\n");}
"int"           {printf("INT\n");}
"boolean"       {printf("BOOLEAN\n");}
"if"            {printf("IF\n");}
"else"          {printf("ELSE\n");}
"while"         {printf("WHILE\n");}
"System"        {printf("SYSTEM\n");}
"out"           {printf("OUT\n");}
"println"       {printf("PRINTLN\n");}
"true"          {printf("TRUE\n");}
"false"         {printf("FALSE\n");}
"this"          {printf("THIS\n");}
"new"           {printf("NEW\n");}

// User created Words
[a-zA-Z][a-zA-Z0-9]*  { printf("Id: %s\n", yytext); }
[ \t\n]         { /* ignore whitespace */ }
[\/\/.*]         {// Ignore single line comments}
[\/\*][.][\*\/]     {}
.               { printf("UNKNOWN: %s\n", yytext); }
%%

/* C functions */

int yywrap() {return 1;}

int main(){
    yylex();
    return 0;
}
