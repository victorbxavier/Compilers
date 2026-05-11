%{
#include "token.h"
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

// Lista global de tokens gerados
std::vector<Token> tokenList;

// Flag para indicar se houve erro léxico
bool lexerHadError = false;

// Contador de linha
int lineNumber = 1;

// Palavras-chave da linguagem (para sugestões de erro)
static const char* keywords[] = {
    "class", "public", "static", "void", "main", "String",
    "return", "int", "boolean", "if", "else", "while",
    "System", "out", "println", "true", "false", "this",
    "new", "extends", "length", nullptr
};

// Função auxiliar para adicionar token à lista
void addToken(TokenType type, const char* text, int line) {
    Token tok;
    tok.type = type;
    tok.lexeme = std::string(text);
    tok.line = line;
    tokenList.push_back(tok);
}

// Calcula distância de Levenshtein para sugestões
int levenshtein(const std::string& s1, const std::string& s2) {
    int len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));
    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost});
        }
    }
    return dp[len1][len2];
}

// Sugere correções para identificadores inválidos
void suggestCorrection(const char* text, int line) {
    std::string input(text);
    std::string bestMatch = "";
    int bestDist = 999;

    for (int i = 0; keywords[i] != nullptr; i++) {
        int dist = levenshtein(input, std::string(keywords[i]));
        if (dist < bestDist) {
            bestDist = dist;
            bestMatch = keywords[i];
        }
    }

    std::cerr << "Erro léxico na linha " << line << ": token inválido \"" << text << "\"" << std::endl;
    if (bestDist <= 2) {
        std::cerr << "  Sugestão: você quis dizer \"" << bestMatch << "\"?" << std::endl;
    }
    lexerHadError = true;
}

%}

%option noyywrap
%option yylineno

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|"_")*
NUMBER      {DIGIT}+
WHITESPACE  [ \t\r]+
NEWLINE     \n

%%

    /* Comentários (pré-processador integrado) */
"//".*                          { /* Ignora comentário de linha única */ }
"/*"([^*]|\*+[^*/])*\*+"/"     { 
    /* Ignora comentário de múltiplas linhas, conta newlines */
    for (int i = 0; yytext[i]; i++) {
        if (yytext[i] == '\n') lineNumber++;
    }
}

    /* Espaços em branco */
{WHITESPACE}    { /* Ignora espaços */ }
{NEWLINE}       { lineNumber++; }

    /* Palavras-chave */
"class"         { addToken(TokenType::CLASS, yytext, lineNumber); }
"public"        { addToken(TokenType::PUBLIC, yytext, lineNumber); }
"static"        { addToken(TokenType::STATIC, yytext, lineNumber); }
"void"          { addToken(TokenType::VOID, yytext, lineNumber); }
"main"          { addToken(TokenType::MAIN, yytext, lineNumber); }
"String"        { addToken(TokenType::STRING, yytext, lineNumber); }
"return"        { addToken(TokenType::RETURN, yytext, lineNumber); }
"int"           { addToken(TokenType::INT, yytext, lineNumber); }
"boolean"       { addToken(TokenType::BOOLEAN, yytext, lineNumber); }
"if"            { addToken(TokenType::IF, yytext, lineNumber); }
"else"          { addToken(TokenType::ELSE, yytext, lineNumber); }
"while"         { addToken(TokenType::WHILE, yytext, lineNumber); }
"System"        { addToken(TokenType::SYSTEM, yytext, lineNumber); }
"out"           { addToken(TokenType::OUT, yytext, lineNumber); }
"println"       { addToken(TokenType::PRINTLN, yytext, lineNumber); }
"true"          { addToken(TokenType::TRUE, yytext, lineNumber); }
"false"         { addToken(TokenType::FALSE, yytext, lineNumber); }
"this"          { addToken(TokenType::THIS, yytext, lineNumber); }
"new"           { addToken(TokenType::NEW, yytext, lineNumber); }
"extends"       { addToken(TokenType::EXTENDS, yytext, lineNumber); }
"length"        { addToken(TokenType::LENGTH, yytext, lineNumber); }

    /* Operadores */
"&&"            { addToken(TokenType::AND, yytext, lineNumber); }
"<"             { addToken(TokenType::LT, yytext, lineNumber); }
">"             { addToken(TokenType::GT, yytext, lineNumber); }
"+"             { addToken(TokenType::PLUS, yytext, lineNumber); }
"-"             { addToken(TokenType::MINUS, yytext, lineNumber); }
"*"             { addToken(TokenType::MULT, yytext, lineNumber); }
"="             { addToken(TokenType::ASSIGN, yytext, lineNumber); }
"!"             { addToken(TokenType::NOT, yytext, lineNumber); }

    /* Delimitadores */
"("             { addToken(TokenType::LPAREN, yytext, lineNumber); }
")"             { addToken(TokenType::RPAREN, yytext, lineNumber); }
"["             { addToken(TokenType::LBRACKET, yytext, lineNumber); }
"]"             { addToken(TokenType::RBRACKET, yytext, lineNumber); }
"{"             { addToken(TokenType::LBRACE, yytext, lineNumber); }
"}"             { addToken(TokenType::RBRACE, yytext, lineNumber); }
";"             { addToken(TokenType::SEMICOLON, yytext, lineNumber); }
","             { addToken(TokenType::COMMA, yytext, lineNumber); }
"."             { addToken(TokenType::DOT, yytext, lineNumber); }

    /* Identificadores */
{ID}            { addToken(TokenType::ID, yytext, lineNumber); }

    /* Números inteiros */
{NUMBER}        { addToken(TokenType::NUMBER, yytext, lineNumber); }

    /* Identificadores inválidos (começam com dígito seguido de letras) */
{DIGIT}+{LETTER}({LETTER}|{DIGIT}|"_")*  {
    suggestCorrection(yytext, lineNumber);
    addToken(TokenType::UNKNOWN, yytext, lineNumber);
}

    /* Caractere desconhecido */
.               {
    std::cerr << "Erro léxico na linha " << lineNumber 
              << ": caractere inesperado '" << yytext << "'" << std::endl;
    lexerHadError = true;
    addToken(TokenType::UNKNOWN, yytext, lineNumber);
}

%%
