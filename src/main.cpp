/**
 * @file main.cpp
 * @brief Programa principal do compilador MiniJava — orquestra o pipeline.
 *
 * Este arquivo coordena todas as etapas da compilação:
 *   1. Leitura dos argumentos de linha de comando
 *   2. Abertura do arquivo fonte (.ling)
 *   3. Execução da análise léxica (via Flex)
 *   4. Execução da análise sintática (via Parser recursive descent)
 *   5. Exibição dos resultados (tokens, erros ou tabela de símbolos)
 *
 * Modos de execução:
 *   ./compiler <arquivo.ling>          → análise completa (léxica + sintática)
 *   ./compiler --tokens <arquivo.ling> → exibe apenas a lista de tokens
 *   ./compiler --help                  → exibe ajuda
 */

#include "token.h"
#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

// === Declarações externas do Flex ===
// Estas variáveis e funções são definidas no código gerado pelo Flex (lex.yy.c)
// e precisam ser declaradas aqui para que o main.cpp possa usá-las.
extern std::vector<Token> tokenList;  // Lista de tokens preenchida pelo lexer
extern bool lexerHadError;            // Flag: true se houve erro léxico
extern int lineNumber;                // Contador de linhas do código fonte
extern FILE* yyin;                    // Ponteiro para o arquivo de entrada do Flex
int yylex();                          // Função principal do Flex (executa a tokenização)

/**
 * @brief Exibe a mensagem de uso do programa.
 * Chamada quando o usuário passa --help ou argumentos inválidos.
 */
void printUsage(const char* progName) {
    std::cerr << "Uso: " << progName << " [opções] <arquivo.ling>" << std::endl;
    std::cerr << "Opções:" << std::endl;
    std::cerr << "  --tokens    Exibe apenas a lista de tokens (sem parsing)" << std::endl;
    std::cerr << "  --help      Exibe esta mensagem" << std::endl;
}

/**
 * @brief Ponto de entrada do compilador.
 *
 * Fluxo detalhado:
 *   1. Parseia argumentos CLI (--tokens, --help, nome do arquivo)
 *   2. Abre o arquivo .ling e configura o Flex para lê-lo
 *   3. Chama yylex() — o Flex lê todo o arquivo e preenche tokenList[]
 *   4. Insere token END_OF_FILE no final (sentinela para o parser)
 *   5. Se houve erro léxico, encerra com código 1
 *   6. Se modo --tokens, imprime a lista e encerra
 *   7. Caso contrário, cria o Parser e executa análise sintática
 *   8. Se sucesso, imprime confirmação + tabela de símbolos
 *   9. Se falha, imprime erro sintático e encerra com código 1
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    bool tokensOnly = false;   // Flag: modo somente tokens (--tokens)
    const char* filename = nullptr;  // Caminho do arquivo de entrada

    // Parse de argumentos da linha de comando
    // Aceita --tokens (modo lista), --help e o nome do arquivo em qualquer ordem
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            tokensOnly = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        std::cerr << "Erro: nenhum arquivo de entrada especificado." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // === ETAPA 1: Abertura do arquivo fonte ===
    // O arquivo .ling contém o código MiniJava a ser compilado
    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Erro: não foi possível abrir o arquivo \"" << filename << "\"" << std::endl;
        return 1;
    }

    // === ETAPA 2: Configuração e execução do Flex (Análise Léxica) ===
    // yyin é o ponteiro de entrada do Flex. Ao atribuir o arquivo,
    // o Flex sabe de onde ler os caracteres.
    // yylex() executa o autômato finito do Flex, que percorre todo o arquivo
    // aplicando as regras de lexer.l e preenchendo tokenList[].
    yyin = file;
    lineNumber = 1;

    // Executa a análise léxica completa
    // Após esta chamada, tokenList[] contém todos os tokens do programa
    yylex();
    fclose(file);

    // Adiciona token sentinela de fim de arquivo.
    // O parser usa este token para saber que não há mais entrada.
    // Sem ele, o parser tentaria ler além do final da lista.
    Token eofToken;
    eofToken.type = TokenType::END_OF_FILE;
    eofToken.lexeme = "$";
    eofToken.line = lineNumber;
    tokenList.push_back(eofToken);

    // === Verificação de erros léxicos ===
    // Se o lexer encontrou caracteres inválidos, não faz sentido
    // prosseguir para a análise sintática (tokens corrompidos).
    if (lexerHadError) {
        std::cerr << "\n=== Análise léxica concluída com erros ===" << std::endl;
        return 1;
    }

    // === Modo somente tokens (--tokens) ===
    // Imprime a lista completa de tokens e encerra sem fazer parsing.
    // Útil para depuração da análise léxica.
    if (tokensOnly) {
        std::cout << "=== Análise léxica concluída com sucesso ===" << std::endl;
        std::cout << "\nLista de Tokens (" << tokenList.size() << " tokens):" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        for (const auto& tok : tokenList) {
            printToken(tok);
        }
        return 0;
    }

    // === ETAPA 3: Análise Sintática ===
    // O parser recebe a lista de tokens e verifica se a sequência
    // respeita a gramática LL(1) da linguagem MiniJava.
    // Durante o parsing, também constrói a tabela de símbolos.
    Parser parser(tokenList);
    bool parseSuccess = parser.parse();

    if (parseSuccess) {
        std::cout << "=== Código sintaticamente correto ===" << std::endl;
        std::cout << "\n=== Tabela de Símbolos ===" << std::endl;
        parser.getSymbolTable().print();
    } else {
        std::cerr << "=== Análise sintática falhou ===" << std::endl;
        return 1;
    }

    return 0;
}
