#include "token.h"
#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

// Declarações externas do Flex
extern std::vector<Token> tokenList;
extern bool lexerHadError;
extern int lineNumber;
extern FILE* yyin;
int yylex();

void printUsage(const char* progName) {
    std::cerr << "Uso: " << progName << " [opções] <arquivo.ling>" << std::endl;
    std::cerr << "Opções:" << std::endl;
    std::cerr << "  --tokens    Exibe apenas a lista de tokens (sem parsing)" << std::endl;
    std::cerr << "  --help      Exibe esta mensagem" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    bool tokensOnly = false;
    const char* filename = nullptr;

    // Parse argumentos
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

    // Abre o arquivo fonte
    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Erro: não foi possível abrir o arquivo \"" << filename << "\"" << std::endl;
        return 1;
    }

    // Configura o Flex para ler do arquivo
    yyin = file;
    lineNumber = 1;

    // Executa a análise léxica
    yylex();
    fclose(file);

    // Adiciona token de fim de arquivo
    Token eofToken;
    eofToken.type = TokenType::END_OF_FILE;
    eofToken.lexeme = "$";
    eofToken.line = lineNumber;
    tokenList.push_back(eofToken);

    // Verifica erros léxicos
    if (lexerHadError) {
        std::cerr << "\n=== Análise léxica concluída com erros ===" << std::endl;
        return 1;
    }

    // Modo apenas tokens
    if (tokensOnly) {
        std::cout << "=== Análise léxica concluída com sucesso ===" << std::endl;
        std::cout << "\nLista de Tokens (" << tokenList.size() << " tokens):" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        for (const auto& tok : tokenList) {
            printToken(tok);
        }
        return 0;
    }

    // === Análise Sintática ===
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
