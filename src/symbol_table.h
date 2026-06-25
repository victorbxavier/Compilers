/**
 * @file symbol_table.h
 * @brief Tabela de Símbolos do compilador MiniJava (Unidade 2).
 *
 * Melhorias em relação à Unidade 1:
 *   - Busca por nome e escopo (para resolução de nomes na análise semântica)
 *   - Suporte a herança (campo parent na classe)
 *   - Detecção de duplicatas
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

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
    std::string parent;  // Para classes: nome da classe pai (herança)
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
                   const std::string& scope, int line, SymbolCategory category,
                   const std::string& parent = "") {
        Symbol sym;
        sym.name = name;
        sym.type = type;
        sym.scope = scope;
        sym.line = line;
        sym.category = category;
        sym.parent = parent;
        symbols_.push_back(sym);
    }

    /**
     * @brief Busca um símbolo por nome e escopo.
     * Segue a resolução: escopo local → classe → classe pai.
     */
    const Symbol* lookup(const std::string& name, const std::string& scope) const {
        // 1. Busca no escopo exato (parâmetro/variável local)
        for (auto& sym : symbols_) {
            if (sym.name == name && sym.scope == scope &&
                (sym.category == SymbolCategory::LOCAL_VAR || sym.category == SymbolCategory::PARAMETER)) {
                return &sym;
            }
        }
        // 2. Busca como atributo da classe (escopo = nome da classe)
        std::string classScope = scope;
        auto dotPos = classScope.find('.');
        if (dotPos != std::string::npos) {
            classScope = classScope.substr(0, dotPos);
        }

        for (auto& sym : symbols_) {
            if (sym.name == name && sym.scope == classScope &&
                sym.category == SymbolCategory::INSTANCE_VAR) {
                return &sym;
            }
        }
        // 3. Busca em classe pai (herança)
        const Symbol* classSym = lookupClass(classScope);
        if (classSym && !classSym->parent.empty()) {
            return lookupInherited(name, classSym->parent);
        }
        return nullptr;
    }

    /** Busca uma classe por nome. */
    const Symbol* lookupClass(const std::string& name) const {
        for (auto& sym : symbols_) {
            if (sym.name == name && sym.category == SymbolCategory::CLASS) {
                return &sym;
            }
        }
        return nullptr;
    }

    /** Busca um método em uma classe (ou herdado). */
    const Symbol* lookupMethod(const std::string& className, const std::string& methodName) const {
        for (auto& sym : symbols_) {
            if (sym.name == methodName && sym.scope == className &&
                sym.category == SymbolCategory::METHOD) {
                return &sym;
            }
        }
        // Busca na classe pai
        const Symbol* cls = lookupClass(className);
        if (cls && !cls->parent.empty()) {
            return lookupMethod(cls->parent, methodName);
        }
        return nullptr;
    }

    /** Verifica se um símbolo já existe no mesmo escopo (duplicata). */
    bool isDuplicate(const std::string& name, const std::string& scope, SymbolCategory cat) const {
        for (auto& sym : symbols_) {
            if (sym.name == name && sym.scope == scope && sym.category == cat) {
                return true;
            }
        }
        return false;
    }

    /** Retorna todos os parâmetros de um método (com suporte a herança). */
    std::vector<const Symbol*> getMethodParams(const std::string& className, const std::string& methodName) const {
        std::vector<const Symbol*> params;
        std::string scope = className + "." + methodName;
        for (auto& sym : symbols_) {
            if (sym.scope == scope && sym.category == SymbolCategory::PARAMETER) {
                params.push_back(&sym);
            }
        }
        // Se não encontrou parâmetros, busca na classe pai
        if (params.empty()) {
            const Symbol* cls = lookupClass(className);
            if (cls && !cls->parent.empty()) {
                return getMethodParams(cls->parent, methodName);
            }
        }
        return params;
    }

    void print() const {
        if (symbols_.empty()) {
            std::cout << "(tabela vazia)" << std::endl;
            return;
        }
        std::cout << std::left
                  << std::setw(20) << "Nome"
                  << std::setw(15) << "Tipo"
                  << std::setw(25) << "Escopo"
                  << std::setw(8)  << "Linha"
                  << std::setw(18) << "Categoria"
                  << std::endl;
        std::cout << std::string(86, '-') << std::endl;
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

    const Symbol* lookupInherited(const std::string& name, const std::string& className) const {
        for (auto& sym : symbols_) {
            if (sym.name == name && sym.scope == className &&
                sym.category == SymbolCategory::INSTANCE_VAR) {
                return &sym;
            }
        }
        const Symbol* cls = lookupClass(className);
        if (cls && !cls->parent.empty()) {
            return lookupInherited(name, cls->parent);
        }
        return nullptr;
    }
};

#endif // SYMBOL_TABLE_H
