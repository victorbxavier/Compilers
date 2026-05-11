#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

enum class SymbolCategory {
    CLASS,
    METHOD,
    INSTANCE_VAR,
    LOCAL_VAR,
    PARAMETER
};

struct Symbol {
    std::string name;
    std::string type;
    std::string scope;
    int line;
    SymbolCategory category;
};

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

class SymbolTable {
public:
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

    const std::vector<Symbol>& getSymbols() const { return symbols_; }
    size_t size() const { return symbols_.size(); }

private:
    std::vector<Symbol> symbols_;
};

#endif // SYMBOL_TABLE_H
