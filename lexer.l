%option c++
%{
/* C code / includes / declarations */
#include <iostream>
int chars = 0;
int words = 0;
int lines = 0;
%}

/* Flex only supports comments in %% */
/* Rules: pattern   action */
%%
[0-9]+          { words++; chars += strlen(yytext); printf("Number: %s\n", yytext); }
"+"             { words++; chars += strlen(yytext); printf("PLUS\n");}
"-"             { words++; chars += strlen(yytext); printf("MINUS\n");}
"*"             { words++; chars += strlen(yytext); printf("MULTIPLY\n");}
"="             { words++; chars += strlen(yytext); printf("ASSIGN\n");}
">"             { words++; chars += strlen(yytext); printf("GT\n");}
"("             { words++; chars += strlen(yytext); printf("OPEN_PARENTHESIS\n");}
")"             { words++; chars += strlen(yytext); printf("CLOSE_PARENTHESIS\n");}
"["             { words++; chars += strlen(yytext); printf("OPEN_BRACKET\n");}
"]"             { words++; chars += strlen(yytext); printf("CLOSE_BRACKET\n");}
"{"             { words++; chars += strlen(yytext); printf("OPEN_BRACES\n");}
"}"             { words++; chars += strlen(yytext); printf("CLOSE_BRACES\n");}
";"             { words++; chars += strlen(yytext); printf("SEMICOLON\n");}
"&&"            { words++; chars += strlen(yytext); printf("AND\n");}
"."             { words++; chars += strlen(yytext); printf("DOT\n");}
"!"             { words++; chars += strlen(yytext); printf("NOT\n");}
"class"         { words++; chars += strlen(yytext); printf("CLASS\n");}
"public"        { words++; chars += strlen(yytext); printf("PUBLIC\n");}
"static"        { words++; chars += strlen(yytext); printf("STATIC\n");}
"void"          { words++; chars += strlen(yytext); printf("VOID\n");}
"main"          { words++; chars += strlen(yytext); printf("MAIN\n");}
"String"        { words++; chars += strlen(yytext); printf("STRING\n");}
"return"        { words++; chars += strlen(yytext); printf("RETURN\n");}
"int"           { words++; chars += strlen(yytext); printf("INT\n");}
"boolean"       { words++; chars += strlen(yytext); printf("BOOLEAN\n");}
"if"            { words++; chars += strlen(yytext); printf("IF\n");}
"else"          { words++; chars += strlen(yytext); printf("ELSE\n");}
"while"         { words++; chars += strlen(yytext); printf("WHILE\n");}
"System"        { words++; chars += strlen(yytext); printf("SYSTEM\n");}
"out"           { words++; chars += strlen(yytext); printf("OUT\n");}
"println"       { words++; chars += strlen(yytext); printf("PRINTLN\n");}
"true"          { words++; chars += strlen(yytext); printf("TRUE\n");}
"false"         { words++; chars += strlen(yytext); printf("FALSE\n");}
"this"          { words++; chars += strlen(yytext); printf("THIS\n");}
"new"           { words++; chars += strlen(yytext); printf("NEW\n");}
[a-zA-Z][a-zA-Z0-9]*  { words++; chars += strlen(yytext); printf("Id: %s\n", yytext); }
[ \t\n]         { /* ignore whitespace */ }
"//".*          { /* Ignore single line comments */ }
"/*"[^*]*\*+([^/][^*]*\*+)*"/"  { /* Ignore multiple line comments */}
.               { printf("UNKNOWN: %s\n", yytext); }
%%

/* C functions */

int yyFlexLexer::yywrap() {return 1;}

int main(){
    yyFlexLexer lexer;
    lexer.yylex();
    return 0;
}
