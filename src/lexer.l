%{
/**
 * @file lexer.l
 * @brief Analisador Léxico (Lexer) do compilador MiniJava, implementado com Flex.
 *
 * Responsabilidades:
 *   1. Remover comentários e whitespace
 *   2. Tokenizar: palavras-chave, operadores, delimitadores, identificadores, números
 *   3. Reportar erros léxicos com linha, coluna e sugestão de correção
 *
 * Saída: preenche o vetor global tokenList[].
 */

#include "token.h"
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

// Lista de todos os tokens gerados durante a análise léxica.
std::vector<Token> tokenList;

// Flag que indica se algum erro léxico foi encontrado.
bool lexerHadError = false;

// Contadores manuais de linha e coluna.
int lineNumber = 1;
int columnNumber = 1;

// Flag para controlar se o lexer deve parar no primeiro erro.
// Configurada pelo main.cpp antes de chamar yylex().
bool stopOnFirstError = false;

// Flag para controlar se sugestões de correção devem ser exibidas.
bool showSuggestions = true;

// Lista de palavras-chave (usada para sugestões de correção).
static const char* keywords[] = {
    "class", "public", "static", "void", "main", "String",
    "return", "int", "boolean", "if", "else", "while",
    "System", "out", "println", "true", "false", "this",
    "new", "extends", "length", nullptr
};

/**
 * @brief Cria um Token e o adiciona ao final de tokenList[].
 */
void addToken(TokenType type, const char* text, int line, int col) {
    Token tok;
    tok.type = type;
    tok.lexeme = std::string(text);
    tok.line = line;
    tok.column = col;
    tokenList.push_back(tok);
}

/**
 * @brief Atualiza o contador de coluna após consumir um token.
 */
void updateColumn() {
    columnNumber += yyleng;
}

/**
 * @brief Calcula a distância de Levenshtein entre duas strings.
 */
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

/**
 * @brief Reporta erro léxico com linha:coluna e sugere correção se possível.
 */
void reportLexError(const char* text, int line, int col) {
    std::string input(text);
    std::string bestMatch = "";
    int bestDist = 999;

    if (showSuggestions) {
        for (int i = 0; keywords[i] != nullptr; i++) {
            int dist = levenshtein(input, std::string(keywords[i]));
            if (dist < bestDist) {
                bestDist = dist;
                bestMatch = keywords[i];
            }
        }
    }

    std::cerr << "Erro léxico na linha " << line << ":" << col
              << ": token inválido \"" << text << "\"" << std::endl;
    if (showSuggestions && bestDist <= 2) {
        std::cerr << "  Sugestão: você quis dizer \"" << bestMatch << "\"?" << std::endl;
    }
    lexerHadError = true;
}

%}

/* === Opções do Flex === */
%option noyywrap
%option yylineno
%option nounput
%option noinput

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|"_")*
NUMBER      {DIGIT}+
WHITESPACE  [ \t\r]+
NEWLINE     \n

%%

    /* --- Comentários --- */
"//".*                          { updateColumn(); }
"/*"([^*]|\*+[^*/])*\*+"/"     {
    for (int i = 0; yytext[i]; i++) {
        if (yytext[i] == '\n') { lineNumber++; columnNumber = 1; }
        else { columnNumber++; }
    }
}

    /* --- Espaços em branco --- */
{WHITESPACE}    { updateColumn(); }
{NEWLINE}       { lineNumber++; columnNumber = 1; }

    /* --- Palavras-chave --- */
"class"         { addToken(TokenType::CLASS, yytext, lineNumber, columnNumber); updateColumn(); }
"public"        { addToken(TokenType::PUBLIC, yytext, lineNumber, columnNumber); updateColumn(); }
"static"        { addToken(TokenType::STATIC, yytext, lineNumber, columnNumber); updateColumn(); }
"void"          { addToken(TokenType::VOID, yytext, lineNumber, columnNumber); updateColumn(); }
"main"          { addToken(TokenType::MAIN, yytext, lineNumber, columnNumber); updateColumn(); }
"String"        { addToken(TokenType::STRING, yytext, lineNumber, columnNumber); updateColumn(); }
"return"        { addToken(TokenType::RETURN, yytext, lineNumber, columnNumber); updateColumn(); }
"int"           { addToken(TokenType::INT, yytext, lineNumber, columnNumber); updateColumn(); }
"boolean"       { addToken(TokenType::BOOLEAN, yytext, lineNumber, columnNumber); updateColumn(); }
"if"            { addToken(TokenType::IF, yytext, lineNumber, columnNumber); updateColumn(); }
"else"          { addToken(TokenType::ELSE, yytext, lineNumber, columnNumber); updateColumn(); }
"while"         { addToken(TokenType::WHILE, yytext, lineNumber, columnNumber); updateColumn(); }
"System"        { addToken(TokenType::SYSTEM, yytext, lineNumber, columnNumber); updateColumn(); }
"out"           { addToken(TokenType::OUT, yytext, lineNumber, columnNumber); updateColumn(); }
"println"       { addToken(TokenType::PRINTLN, yytext, lineNumber, columnNumber); updateColumn(); }
"true"          { addToken(TokenType::TRUE, yytext, lineNumber, columnNumber); updateColumn(); }
"false"         { addToken(TokenType::FALSE, yytext, lineNumber, columnNumber); updateColumn(); }
"this"          { addToken(TokenType::THIS, yytext, lineNumber, columnNumber); updateColumn(); }
"new"           { addToken(TokenType::NEW, yytext, lineNumber, columnNumber); updateColumn(); }
"extends"       { addToken(TokenType::EXTENDS, yytext, lineNumber, columnNumber); updateColumn(); }
"length"        { addToken(TokenType::LENGTH, yytext, lineNumber, columnNumber); updateColumn(); }

    /* --- Operadores --- */
"&&"            { addToken(TokenType::AND, yytext, lineNumber, columnNumber); updateColumn(); }
"<"             { addToken(TokenType::LT, yytext, lineNumber, columnNumber); updateColumn(); }
">"             { addToken(TokenType::GT, yytext, lineNumber, columnNumber); updateColumn(); }
"+"             { addToken(TokenType::PLUS, yytext, lineNumber, columnNumber); updateColumn(); }
"-"             { addToken(TokenType::MINUS, yytext, lineNumber, columnNumber); updateColumn(); }
"*"             { addToken(TokenType::MULT, yytext, lineNumber, columnNumber); updateColumn(); }
"="             { addToken(TokenType::ASSIGN, yytext, lineNumber, columnNumber); updateColumn(); }
"!"             { addToken(TokenType::NOT, yytext, lineNumber, columnNumber); updateColumn(); }

    /* --- Delimitadores --- */
"("             { addToken(TokenType::LPAREN, yytext, lineNumber, columnNumber); updateColumn(); }
")"             { addToken(TokenType::RPAREN, yytext, lineNumber, columnNumber); updateColumn(); }
"["             { addToken(TokenType::LBRACKET, yytext, lineNumber, columnNumber); updateColumn(); }
"]"             { addToken(TokenType::RBRACKET, yytext, lineNumber, columnNumber); updateColumn(); }
"{"             { addToken(TokenType::LBRACE, yytext, lineNumber, columnNumber); updateColumn(); }
"}"             { addToken(TokenType::RBRACE, yytext, lineNumber, columnNumber); updateColumn(); }
";"             { addToken(TokenType::SEMICOLON, yytext, lineNumber, columnNumber); updateColumn(); }
","             { addToken(TokenType::COMMA, yytext, lineNumber, columnNumber); updateColumn(); }
"."             { addToken(TokenType::DOT, yytext, lineNumber, columnNumber); updateColumn(); }

    /* --- Identificadores --- */
{ID}            { addToken(TokenType::ID, yytext, lineNumber, columnNumber); updateColumn(); }

    /* --- Números inteiros --- */
{NUMBER}        { addToken(TokenType::NUMBER, yytext, lineNumber, columnNumber); updateColumn(); }

    /* --- Identificadores inválidos (começam com dígito) --- */
{DIGIT}+{LETTER}({LETTER}|{DIGIT}|"_")*  {
    reportLexError(yytext, lineNumber, columnNumber);
    addToken(TokenType::UNKNOWN, yytext, lineNumber, columnNumber);
    updateColumn();
    if (stopOnFirstError) return 1;
}

    /* --- Caractere desconhecido (fallback) --- */
.               {
    std::cerr << "Erro léxico na linha " << lineNumber << ":" << columnNumber
              << ": caractere inesperado '" << yytext << "'" << std::endl;
    lexerHadError = true;
    addToken(TokenType::UNKNOWN, yytext, lineNumber, columnNumber);
    updateColumn();
    if (stopOnFirstError) return 1;
}

%%
