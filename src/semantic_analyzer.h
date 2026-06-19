/**
 * @file semantic_analyzer.h
 * @brief Analisador Semântico do compilador MiniJava.
 *
 * Percorre a AST (após o parsing) e verifica:
 *   - Tipos de operadores e expressões
 *   - Existência de variáveis e classes
 *   - Compatibilidade de atribuições
 *   - Condições boolean em if/while
 *   - Chamadas de método (existência, argumentos)
 *   - Classes vazias
 */

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"
#include <string>
#include <vector>
#include <iostream>

class SemanticAnalyzer {
public:
    SemanticAnalyzer(const SymbolTable& table) : table_(table), errorCount_(0) {}

    bool analyze(const Program* program) {
        if (!program) return false;

        // Verifica classes vazias
        for (auto& cls : program->classes) {
            checkClassNotEmpty(cls.get());
        }

        // Verifica cada classe
        for (auto& cls : program->classes) {
            currentClass_ = cls->name;
            for (auto& method : cls->methods) {
                currentMethod_ = method->name;
                // Verifica corpo do método
                for (auto& stmt : method->body) {
                    checkStmt(stmt.get());
                }
                // Verifica tipo de retorno
                std::string retType = typeToString(method->returnType.get());
                std::string actualType = inferExpType(method->returnExp.get());
                if (!typesCompatible(retType, actualType)) {
                    reportError(0, "tipo de retorno incompatível no método '" + method->name +
                                "': esperado " + retType + ", obteve " + actualType);
                }
            }
            currentMethod_ = "";
        }

        // Verifica main class body
        currentClass_ = program->mainClass->name;
        currentMethod_ = "main";
        for (auto& stmt : program->mainClass->body) {
            checkStmt(stmt.get());
        }

        return errorCount_ == 0;
    }

    int getErrorCount() const { return errorCount_; }

private:
    const SymbolTable& table_;
    int errorCount_;
    std::string currentClass_;
    std::string currentMethod_;

    std::string currentScope() const {
        if (currentMethod_.empty()) return currentClass_;
        return currentClass_ + "." + currentMethod_;
    }

    void reportError(int line, const std::string& msg) {
        std::cerr << "Erro semântico";
        if (line > 0) std::cerr << " na linha " << line;
        std::cerr << ": " << msg << std::endl;
        errorCount_++;
    }

    bool typesCompatible(const std::string& expected, const std::string& actual) const {
        if (expected == actual) return true;
        if (expected == "unknown" || actual == "unknown") return true;  // Não propagar erros
        // Subtipo: classe filha é compatível com classe pai
        const Symbol* cls = table_.lookupClass(actual);
        if (cls && !cls->parent.empty() && cls->parent == expected) return true;
        return false;
    }

    void checkClassNotEmpty(const ClassDecl* cls) {
        if (cls->vars.empty() && cls->methods.empty()) {
            reportError(cls->line, "classe '" + cls->name + "' é vazia (sem atributos e sem métodos)");
        }
    }

    // ==================== Verificação de comandos ====================

    void checkStmt(const Stmt* stmt) {
        if (!stmt) return;

        if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
            std::string condType = inferExpType(ifStmt->condition.get());
            if (condType != "boolean" && condType != "unknown") {
                reportError(ifStmt->line, "condição do 'if' deve ser boolean, obteve " + condType);
            }
            for (auto& s : ifStmt->thenBody) checkStmt(s.get());
            for (auto& s : ifStmt->elseBody) checkStmt(s.get());
        }
        else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
            std::string condType = inferExpType(whileStmt->condition.get());
            if (condType != "boolean" && condType != "unknown") {
                reportError(whileStmt->line, "condição do 'while' deve ser boolean, obteve " + condType);
            }
            for (auto& s : whileStmt->body) checkStmt(s.get());
        }
        else if (auto* printStmt = dynamic_cast<const PrintStmt*>(stmt)) {
            std::string expType = inferExpType(printStmt->expression.get());
            if (expType != "int" && expType != "unknown") {
                reportError(printStmt->line, "System.out.println requer int, obteve " + expType);
            }
        }
        else if (auto* assignStmt = dynamic_cast<const AssignStmt*>(stmt)) {
            const Symbol* sym = table_.lookup(assignStmt->variable, currentScope());
            if (!sym) {
                reportError(assignStmt->line, "variável não declarada: '" + assignStmt->variable + "'");
            } else {
                std::string valType = inferExpType(assignStmt->value.get());
                if (!typesCompatible(sym->type, valType)) {
                    reportError(assignStmt->line, "atribuição incompatível: variável '" +
                                assignStmt->variable + "' é " + sym->type + ", valor é " + valType);
                }
            }
        }
        else if (auto* arrAssign = dynamic_cast<const ArrayAssignStmt*>(stmt)) {
            const Symbol* sym = table_.lookup(arrAssign->array, currentScope());
            if (!sym) {
                reportError(arrAssign->line, "variável não declarada: '" + arrAssign->array + "'");
            } else if (sym->type != "int[]") {
                reportError(arrAssign->line, "'" + arrAssign->array + "' não é um array (tipo: " + sym->type + ")");
            }
            std::string idxType = inferExpType(arrAssign->index.get());
            if (idxType != "int" && idxType != "unknown") {
                reportError(arrAssign->line, "índice de array deve ser int, obteve " + idxType);
            }
            std::string valType = inferExpType(arrAssign->value.get());
            if (valType != "int" && valType != "unknown") {
                reportError(arrAssign->line, "valor atribuído a array deve ser int, obteve " + valType);
            }
        }
        else if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
            for (auto& s : block->stmts) checkStmt(s.get());
        }
    }

    // ==================== Inferência de tipo de expressões ====================

    std::string inferExpType(const Exp* exp) const {
        if (!exp) return "unknown";

        if (dynamic_cast<const IntLiteralExp*>(exp)) return "int";
        if (dynamic_cast<const TrueLiteralExp*>(exp)) return "boolean";
        if (dynamic_cast<const FalseLiteralExp*>(exp)) return "boolean";
        if (dynamic_cast<const ThisExp*>(exp)) return currentClass_;

        if (auto* id = dynamic_cast<const IdentifierExp*>(exp)) {
            const Symbol* sym = table_.lookup(id->name, currentScope());
            if (!sym) {
                // Não reporta erro aqui pra não duplicar (reporta no checkStmt se for assignment)
                return "unknown";
            }
            return sym->type;
        }

        if (auto* plus = dynamic_cast<const PlusExp*>(exp)) {
            std::string lt = inferExpType(plus->left.get());
            std::string rt = inferExpType(plus->right.get());
            if (lt != "int" && lt != "unknown") reportErrorConst(exp->line, "operador '+' requer int à esquerda, obteve " + lt);
            if (rt != "int" && rt != "unknown") reportErrorConst(exp->line, "operador '+' requer int à direita, obteve " + rt);
            return "int";
        }
        if (auto* minus = dynamic_cast<const MinusExp*>(exp)) {
            std::string lt = inferExpType(minus->left.get());
            std::string rt = inferExpType(minus->right.get());
            if (lt != "int" && lt != "unknown") reportErrorConst(exp->line, "operador '-' requer int à esquerda, obteve " + lt);
            if (rt != "int" && rt != "unknown") reportErrorConst(exp->line, "operador '-' requer int à direita, obteve " + rt);
            return "int";
        }
        if (auto* times = dynamic_cast<const TimesExp*>(exp)) {
            std::string lt = inferExpType(times->left.get());
            std::string rt = inferExpType(times->right.get());
            if (lt != "int" && lt != "unknown") reportErrorConst(exp->line, "operador '*' requer int à esquerda, obteve " + lt);
            if (rt != "int" && rt != "unknown") reportErrorConst(exp->line, "operador '*' requer int à direita, obteve " + rt);
            return "int";
        }
        if (auto* lt = dynamic_cast<const LessThanExp*>(exp)) {
            std::string lType = inferExpType(lt->left.get());
            std::string rType = inferExpType(lt->right.get());
            if (lType != "int" && lType != "unknown") reportErrorConst(exp->line, "operador '<' requer int à esquerda, obteve " + lType);
            if (rType != "int" && rType != "unknown") reportErrorConst(exp->line, "operador '<' requer int à direita, obteve " + rType);
            return "boolean";
        }
        if (auto* andE = dynamic_cast<const AndExp*>(exp)) {
            std::string lType = inferExpType(andE->left.get());
            std::string rType = inferExpType(andE->right.get());
            if (lType != "boolean" && lType != "unknown") reportErrorConst(exp->line, "operador '&&' requer boolean à esquerda, obteve " + lType);
            if (rType != "boolean" && rType != "unknown") reportErrorConst(exp->line, "operador '&&' requer boolean à direita, obteve " + rType);
            return "boolean";
        }
        if (auto* notE = dynamic_cast<const NotExp*>(exp)) {
            std::string t = inferExpType(notE->expr.get());
            if (t != "boolean" && t != "unknown") reportErrorConst(exp->line, "operador '!' requer boolean, obteve " + t);
            return "boolean";
        }

        if (auto* arrLookup = dynamic_cast<const ArrayLookupExp*>(exp)) {
            std::string arrType = inferExpType(arrLookup->array.get());
            if (arrType != "int[]" && arrType != "unknown")
                reportErrorConst(exp->line, "operador '[]' requer int[], obteve " + arrType);
            std::string idxType = inferExpType(arrLookup->index.get());
            if (idxType != "int" && idxType != "unknown")
                reportErrorConst(exp->line, "índice de array deve ser int, obteve " + idxType);
            return "int";
        }
        if (auto* arrLen = dynamic_cast<const ArrayLengthExp*>(exp)) {
            std::string arrType = inferExpType(arrLen->array.get());
            if (arrType != "int[]" && arrType != "unknown")
                reportErrorConst(exp->line, ".length requer int[], obteve " + arrType);
            return "int";
        }
        if (auto* newArr = dynamic_cast<const NewArrayExp*>(exp)) {
            std::string sizeType = inferExpType(newArr->size.get());
            if (sizeType != "int" && sizeType != "unknown")
                reportErrorConst(exp->line, "tamanho de new int[] deve ser int, obteve " + sizeType);
            return "int[]";
        }
        if (auto* newObj = dynamic_cast<const NewObjectExp*>(exp)) {
            const Symbol* cls = table_.lookupClass(newObj->className);
            if (!cls) {
                reportErrorConst(exp->line, "classe não declarada: '" + newObj->className + "'");
                return "unknown";
            }
            return newObj->className;
        }
        if (auto* call = dynamic_cast<const MethodCallExp*>(exp)) {
            std::string objType = inferExpType(call->object.get());
            if (objType == "unknown") return "unknown";
            const Symbol* method = table_.lookupMethod(objType, call->method);
            if (!method) {
                reportErrorConst(exp->line, "método '" + call->method + "' não encontrado na classe '" + objType + "'");
                return "unknown";
            }
            // Verifica número de argumentos
            auto params = table_.getMethodParams(objType, call->method);
            if (params.size() != call->args.size()) {
                reportErrorConst(exp->line, "método '" + call->method + "' espera " +
                    std::to_string(params.size()) + " argumento(s), recebeu " +
                    std::to_string(call->args.size()));
            } else {
                for (size_t i = 0; i < params.size(); i++) {
                    std::string argType = inferExpType(call->args[i].get());
                    if (!typesCompatible(params[i]->type, argType)) {
                        reportErrorConst(exp->line, "argumento " + std::to_string(i+1) + " de '" +
                            call->method + "': esperado " + params[i]->type + ", obteve " + argType);
                    }
                }
            }
            return method->type;
        }

        return "unknown";
    }

    // Versão const de reportError para uso dentro de inferExpType
    void reportErrorConst(int line, const std::string& msg) const {
        std::cerr << "Erro semântico";
        if (line > 0) std::cerr << " na linha " << line;
        std::cerr << ": " << msg << std::endl;
        const_cast<SemanticAnalyzer*>(this)->errorCount_++;
    }
};

#endif // SEMANTIC_ANALYZER_H
