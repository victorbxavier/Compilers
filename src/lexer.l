%{
/**
 * @file lexer.l
 * @brief Analisador Lexico (Lexer) do compilador MiniJava, implementado com Flex.
 *
 * O Flex gera um automato finito deterministico (AFD) a partir das regras
 * de expressoes regulares definidas abaixo. O AFD percorre o codigo fonte
 * caractere a caractere, identificando padroes e gerando tokens.
 *
 * Responsabilidades deste modulo:
 *   1. PRE-PROCESSAMENTO: remover comentarios e whitespace
 *   2. TOKENIZACAO: reconhecer palavras-chave, operadores, delimitadores,
 *      identificadores e numeros, gerando um Token para cada um
 *   3. TRATAMENTO DE ERROS: reportar caracteres invalidos com sugestoes
 *      de correcao baseadas em distancia de Levenshtein
 *
 * Saida: preenche o vetor global tokenList[] com todos os tokens encontrados.
 *
 * Estrutura do arquivo Flex (.l):
 *   Secao 1 (%{ ... %})  -> Codigo C++ copiado diretamente para o .c gerado
 *   Secao 2 (definicoes) -> Macros de expressoes regulares reutilizaveis
 *   Secao 3 (%% ... %%)  -> Regras padrao->acao (o coracao do lexer)
 */

#include "token.h"
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

// Lista de todos os tokens gerados durante a analise lexica.
// Preenchida sequencialmente conforme o Flex reconhece padroes.
std::vector<Token> tokenList;

// Flag que indica se algum erro lexico foi encontrado.
// Se true, o main.cpp nao prossegue para a analise sintatica.
bool lexerHadError = false;

// Contador manual de linhas. Incrementado a cada '\n' encontrado.
// Usado para reportar a localizacao de erros ao usuario.
int lineNumber = 1;

// Lista de palavras-chave da linguagem (usada para sugestoes de correcao).
// Quando o lexer encontra um token invalido, compara com estas palavras
// para sugerir a correcao mais provavel.
static const char* keywords[] = {
    "class", "public", "static", "void", "main", "String",
    "return", "int", "boolean", "if", "else", "while",
    "System", "out", "println", "true", "false", "this",
    "new", "extends", "length", nullptr
};

/**
 * @brief Cria um Token e o adiciona ao final de tokenList[].
 * @param type Tipo do token (ex: TokenType::CLASS, TokenType::ID)
 * @param text yytext do Flex -- variavel automatica que contem o trecho exato
 *             do codigo fonte que casou com a regra atual (ex: ";", "class", "myVar")
 * @param line Numero da linha atual
 */
void addToken(TokenType type, const char* text, int line) {
    Token tok;
    tok.type = type;
    tok.lexeme = std::string(text);
    tok.line = line;
    tokenList.push_back(tok);
}

/**
 * @brief Calcula a distancia de Levenshtein entre duas strings.
 *
 * A distancia de Levenshtein mede o numero minimo de operacoes
 * (insercao, remocao, substituicao) para transformar s1 em s2.
 * Usada para encontrar a palavra-chave mais proxima de um token invalido.
 *
 * Exemplo: levenshtein("clas", "class") = 1 (falta um 's')
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
 * @brief Reporta erro lexico e sugere correcao se possivel.
 *
 * Compara o token invalido com todas as palavras-chave usando
 * distancia de Levenshtein. Se a distancia for <= 2, sugere a correcao.
 *
 * Exemplo de saida:
 *   Erro lexico na linha 3: token invalido "clas"
 *     Sugestao: voce quis dizer "class"?
 */
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

/* === Opcoes do Flex === */
%option noyywrap
    /* noyywrap: nao chama yywrap() ao fim do arquivo (so processa um arquivo) */
%option yylineno
    /* yylineno: ativa a variavel automatica yylineno (contador de linhas do Flex) */
%option nounput
    /* nounput: nao gera a funcao yyunput (nao usada neste lexer) */

/* === Variaveis automaticas do Flex (referencia rapida) ===
 *   yytext  (char*) -- texto exato que casou com a regra atual
 *   yyleng  (int)   -- tamanho de yytext (strlen)
 *   yyin    (FILE*) -- arquivo de entrada sendo lido
 *   yylineno (int)  -- contador de linhas automatico
 */

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|"_")*
NUMBER      {DIGIT}+
WHITESPACE  [ \t\r]+
NEWLINE     \n

%%

    /* ================================================================ */
    /* REGRAS DO LEXER                                                  */
    /* Formato: padrao { acao }                                         */
    /* O Flex casa o padrao mais longo possivel (maximal munch).         */
    /* Em caso de empate no tamanho, a regra que aparece primeiro vence. */
    /* ================================================================ */

    /* --- Comentarios (pre-processador integrado) --- */
    /* Descartados sem gerar tokens. */
"//".*                          { /* Ignora comentario de linha unica */ }
"/*"([^*]|\*+[^*/])*\*+"/"      { 
    /* Comentario de multiplas linhas: conta \n internos */
    for (int i = 0; yytext[i]; i++) {
        if (yytext[i] == '\n') lineNumber++;
    }
}

    /* --- Espacos em branco (descartados) --- */
{WHITESPACE}    { /* Ignora espacos, tabs e \r */ }
{NEWLINE}       { lineNumber++; }

    /* --- Palavras-chave --- */
    /* Aparecem ANTES de {ID} para ter prioridade em caso de empate. */
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

    /* --- Operadores --- */
    /* "&&" vem antes para nao ser lido como dois tokens separados. */
"&&"            { addToken(TokenType::AND, yytext, lineNumber); }
"<"             { addToken(TokenType::LT, yytext, lineNumber); }
">"             { addToken(TokenType::GT, yytext, lineNumber); }
"+"             { addToken(TokenType::PLUS, yytext, lineNumber); }
"-"             { addToken(TokenType::MINUS, yytext, lineNumber); }
"*"             { addToken(TokenType::MULT, yytext, lineNumber); }
"="             { addToken(TokenType::ASSIGN, yytext, lineNumber); }
"!"             { addToken(TokenType::NOT, yytext, lineNumber); }

    /* --- Delimitadores --- */
"("             { addToken(TokenType::LPAREN, yytext, lineNumber); }
")"             { addToken(TokenType::RPAREN, yytext, lineNumber); }
"["             { addToken(TokenType::LBRACKET, yytext, lineNumber); }
"]"             { addToken(TokenType::RBRACKET, yytext, lineNumber); }
"{"             { addToken(TokenType::LBRACE, yytext, lineNumber); }
"}"             { addToken(TokenType::RBRACE, yytext, lineNumber); }
";"             { addToken(TokenType::SEMICOLON, yytext, lineNumber); }
","             { addToken(TokenType::COMMA, yytext, lineNumber); }
"."             { addToken(TokenType::DOT, yytext, lineNumber); }

    /* --- Identificadores --- */
    /* Comeca com letra, pode conter letras, digitos e _. */
    /* So casa aqui se NAO casou com nenhuma keyword acima. */
{ID}            { addToken(TokenType::ID, yytext, lineNumber); }

    /* --- Numeros inteiros --- */
{NUMBER}        { addToken(TokenType::NUMBER, yytext, lineNumber); }

    /* --- Identificadores invalidos (comecam com digito + letras) --- */
{DIGIT}+{LETTER}({LETTER}|{DIGIT}|"_")*  {
    suggestCorrection(yytext, lineNumber);
    addToken(TokenType::UNKNOWN, yytext, lineNumber);
}

    /* --- Caractere desconhecido (fallback) --- */
    /* O ponto (.) casa com qualquer caractere exceto \n. */
    /* DEVE ser a ultima regra. */
.               {
    std::cerr << "Erro léxico na linha " << lineNumber 
              << ": caractere inesperado '" << yytext << "'" << std::endl;
    lexerHadError = true;
    addToken(TokenType::UNKNOWN, yytext, lineNumber);
}

%%
