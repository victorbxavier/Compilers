/**
 * @file symbol_table.h
 * @brief Tabela de Símbolos do compilador MiniJava.
 *
 * A tabela de símbolos é uma estrutura de dados que armazena informações
 * sobre todas as entidades declaradas no programa (classes, métodos, variáveis).
 *
 * É preenchida durante a análise sintática: sempre que o parser reconhece
 * uma declaração, ele insere um registro aqui. Isso permite:
 *   - Verificar se um identificador foi declarado antes de ser usado
 *   - Conhecer o tipo de cada variável/método
 *   - Saber em qual escopo cada símbolo foi definido
 *
 * Nesta implementação, a tabela é um vetor linear (sem hash),
 * suficiente para a fase atual do compilador.
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

/**
 * @enum SymbolCategory
 * @brief Classifica o tipo de declaração que um símbolo representa.
 *
 * Cada símbolo na tabela pertence a exatamente uma categoria:
 *   - CLASS: declaração de classe (escopo global)
 *   - METHOD: declaração de método (escopo da classe)
 *   - INSTANCE_VAR: variável de instância (atributo da classe)
 *   - LOCAL_VAR: variável local (dentro de um método)
 *   - PARAMETER: parâmetro formal de um método
 */
enum class SymbolCategory {
    CLASS,          // Declaração de classe
    METHOD,         // Declaração de método
    INSTANCE_VAR,   // Variável de instância (atributo)
    LOCAL_VAR,      // Variável local de método
    PARAMETER       // Parâmetro formal de método
};

/**
 * @struct Symbol
 * @brief Representa uma entrada individual na tabela de símbolos.
 *
 * Cada símbolo armazena:
 *   - name: nome do identificador (ex: "Factorial", "compute", "num")
 *   - type: tipo associado (ex: "int", "boolean", "int[]", "Factorial", "class", "void")
 *   - scope: escopo onde foi declarado (ex: "global", "Factorial", "Factorial.compute")
 *   - line: linha da declaração no código fonte
 *   - category: classificação do símbolo (classe, método, variável, etc.)
 *
 * Exemplo de entrada:
 *   { name="num", type="int", scope="Factorial.compute", line=5, category=LOCAL_VAR }
 */
struct Symbol {
    std::string name;           // Nome do identificador
    std::string type;           // Tipo (int, boolean, nome de classe, void, etc.)
    std::string scope;          // Escopo de declaração (global, Classe, Classe.metodo)
    int line;                   // Linha no código fonte onde foi declarado
    SymbolCategory category;    // Categoria do símbolo
};

/**
 * @brief Converte uma SymbolCategory para string legível em português.
 * Usado na impressão formatada da tabela de símbolos.
 */
inline std::string categoryToString(SymbolCategory cat) {
    switch (cat) {
        case SymbolCategory::CLASS:        return "classe";
        case SymbolCategory::METHOD:       return "método";
        case SymbolCategory::INSTANCE_VAR: return "var. instância";
        case SymbolCategory::LOCAL_VAR:    return "var. local";
        case SymbolCategory::PARAMETER:    return "parâmetro";
    }
    return "desconhecido";
}

/**
 * @class SymbolTable
 * @brief Armazena e gerencia todos os símbolos declarados no programa.
 *
 * Funciona como um registro sequencial: símbolos são adicionados na ordem
 * em que são encontrados durante o parsing. A impressão final mostra
 * uma tabela formatada com todas as declarações.
 *
 * Fluxo de uso:
 *   1. O parser encontra uma declaração (ex: "int x;")
 *   2. Chama addSymbol("x", "int", "Classe.metodo", linha, LOCAL_VAR)
 *   3. No final, main.cpp chama print() para exibir a tabela completa
 */
class SymbolTable {
public:
    /**
     * @brief Adiciona um novo símbolo à tabela.
     * @param name Nome do identificador
     * @param type Tipo associado ao símbolo
     * @param scope Escopo onde foi declarado
     * @param line Linha no código fonte
     * @param category Categoria do símbolo
     */
    void addSymbol(const std::string& name, const std::string& type,
                   const std::string& scope, int line, SymbolCategory category) {
        Symbol sym;
        sym.name = name;
        sym.type = type;
        sym.scope = scope;
        sym.line = line;
        sym.category = category;
        symbols_.push_back(sym);
    }

    /**
     * @brief Imprime a tabela de símbolos formatada no stdout.
     *
     * Formato:
     *   Nome                Tipo           Escopo                   Linha   Categoria
     *   --------------------------------------------------------------------------------------
     *   Factorial           class          global                   1       classe
     *   compute             int            Factorial                3       método
     *   num                 int            Factorial.compute        4       var. local
     */
    void print() const {
        if (symbols_.empty()) {
            std::cout << "(tabela vazia)" << std::endl;
            return;
        }

        // Cabeçalho
        std::cout << std::left
                  << std::setw(20) << "Nome"
                  << std::setw(15) << "Tipo"
                  << std::setw(25) << "Escopo"
                  << std::setw(8)  << "Linha"
                  << std::setw(18) << "Categoria"
                  << std::endl;
        std::cout << std::string(86, '-') << std::endl;

        // Linhas
        for (const auto& sym : symbols_) {
            std::cout << std::left
                      << std::setw(20) << sym.name
                      << std::setw(15) << sym.type
                      << std::setw(25) << sym.scope
                      << std::setw(8)  << sym.line
                      << std::setw(18) << categoryToString(sym.category)
                      << std::endl;
        }
    }

    /** @brief Retorna referência constante ao vetor interno de símbolos. */
    const std::vector<Symbol>& getSymbols() const { return symbols_; }

    /** @brief Retorna a quantidade de símbolos registrados. */
    size_t size() const { return symbols_.size(); }

private:
    std::vector<Symbol> symbols_;  // Armazenamento sequencial dos símbolos
};

#endif // SYMBOL_TABLE_H
