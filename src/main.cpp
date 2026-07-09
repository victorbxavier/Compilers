/**
 * @file main.cpp
 * @brief Programa principal do compilador MiniJava — Unidade 3.
 *
 * Pipeline: Lexer → Parser (AST + Tabela) → Análise Semântica → Geração de Código (3AC) → Saída
 *
 * Flags:
 *   --tokens           Exibe lista de tokens
 *   --ast              Exibe a árvore sintática abstrata
 *   --symbols          Exibe a tabela de símbolos
 *   --no-ir            Oculta o código intermediário (exibido por padrão)
 *   --suggest          Ativa sugestões de correção
 *   --stop-first-error Para no primeiro erro léxico
 *   --help             Exibe ajuda
 */

#include "token.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "codegen.h"
#include <iostream>
#include <cstring>
#include <memory>

// Declarações externas do Flex
extern std::vector<Token> tokenList;
extern bool lexerHadError;
extern int lineNumber;
extern int columnNumber;
extern bool stopOnFirstError;
extern bool showSuggestions;
extern FILE* yyin;
int yylex();

void printUsage(const char* progName) {
    std::cerr << "Uso: " << progName << " [opções] <arquivo.ling>\n"
              << "Opções:\n"
              << "  --tokens           Exibe a lista de tokens\n"
              << "  --ast              Exibe a árvore sintática abstrata\n"
              << "  --symbols          Exibe a tabela de símbolos\n"
              << "  --no-ir            Oculta o código intermediário (3AC)\n"
              << "  --suggest          Ativa sugestões de correção\n"
              << "  --stop-first-error Para no primeiro erro léxico\n"
              << "  --help             Exibe esta mensagem\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    bool flagTokens = false;
    bool flagAst = false;
    bool flagSymbols = false;
    bool flagNoIR = false;
    bool flagSuggest = false;
    bool flagStopFirst = false;
    const char* filename = nullptr;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            flagTokens = true;
        } else if (strcmp(argv[i], "--ast") == 0) {
            flagAst = true;
        } else if (strcmp(argv[i], "--symbols") == 0) {
            flagSymbols = true;
        } else if (strcmp(argv[i], "--no-ir") == 0) {
            flagNoIR = true;
        } else if (strcmp(argv[i], "--suggest") == 0) {
            flagSuggest = true;
        } else if (strcmp(argv[i], "--stop-first-error") == 0) {
            flagStopFirst = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        std::cerr << "Erro: nenhum arquivo de entrada especificado.\n";
        printUsage(argv[0]);
        return 1;
    }

    // === ETAPA 1: Análise Léxica ===
    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Erro: não foi possível abrir \"" << filename << "\"\n";
        return 1;
    }

    yyin = file;
    lineNumber = 1;
    columnNumber = 1;
    stopOnFirstError = flagStopFirst;
    showSuggestions = flagSuggest;

    yylex();
    fclose(file);

    Token eofToken;
    eofToken.type = TokenType::END_OF_FILE;
    eofToken.lexeme = "$";
    eofToken.line = lineNumber;
    eofToken.column = columnNumber;
    tokenList.push_back(eofToken);

    if (lexerHadError) {
        std::cerr << "\n=== Análise léxica concluída com erros ===" << std::endl;
        return 1;
    }

    if (flagTokens) {
        std::cout << "=== Lista de Tokens (" << tokenList.size() << " tokens) ===" << std::endl;
        for (const auto& tok : tokenList) {
            printToken(tok);
        }
        return 0;
    }

    // === ETAPA 2: Análise Sintática (constrói AST + preenche tabela) ===
    Parser parser(tokenList, flagStopFirst, flagSuggest);
    std::unique_ptr<Program> ast = parser.parse();

    if (!ast || parser.hadError()) {
        std::cerr << "\n=== Análise sintática falhou (" << parser.getErrorCount()
                  << " erro(s)) ===" << std::endl;
        return 1;
    }

    std::cout << "=== Análise sintática concluída com sucesso ===" << std::endl;

    if (flagAst) {
        std::cout << "\n=== Árvore Sintática Abstrata ===" << std::endl;
        ast->print();
    }

    if (flagSymbols) {
        std::cout << "\n=== Tabela de Símbolos ===" << std::endl;
        parser.getSymbolTable().print();
    }

    // === ETAPA 3: Análise Semântica (percorre a AST) ===
    SemanticAnalyzer analyzer(parser.getSymbolTable());
    bool semanticOk = analyzer.analyze(ast.get());

    if (semanticOk) {
        std::cout << "\n=== Análise semântica concluída sem erros ===" << std::endl;
    } else {
        std::cerr << "\n=== Análise semântica encontrou " << analyzer.getErrorCount()
                  << " erro(s) ===" << std::endl;
        return 1;
    }

    // === ETAPA 4: Geração de Código Intermediário (3AC) ===
    CodeGenerator codegen(parser.getSymbolTable(), ast.get()); // .get() extrai o ponteiro cru do unique_ptr
    TAC tac = codegen.generate();

    if (!flagNoIR) {
        std::cout << "\n=== Código Intermediário (3AC) — "
                  << tac.size() << " instruções ===" << std::endl;
        tac.print();
    }

    std::cout << "\n=== Compilação concluída com sucesso ===" << std::endl;
    return 0;
}
