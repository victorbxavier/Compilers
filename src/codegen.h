/**
 * @file codegen.h
 * @brief Gerador de Código Intermediário (3AC) a partir da AST.
 *
 * Percorre a AST recursivamente (pós-ordem) e gera instruções
 * de código de três endereços para cada nó visitado.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "three_address_code.h"
#include "symbol_table.h"
#include <string>
#include <memory>

class CodeGenerator {
public:
    CodeGenerator(const SymbolTable& table) : table_(table) {}

    /**
     * @brief Gera código intermediário para o programa inteiro.
     * @return Objeto TAC com todas as instruções geradas.
     */
    TAC generate(const Program* program) {
        if (!program) return tac_;

        // Gera código para o main
        tac_.emit(OpCode::LABEL, "", "main");
        for (auto& stmt : program->mainClass->body) {
            generateStmt(stmt.get());
        }
        tac_.emit(OpCode::HALT);

        // Gera código para cada método de cada classe
        for (auto& cls : program->classes) {
            currentClass_ = cls->name;
            for (auto& method : cls->methods) {
                generateMethod(method.get(), cls->name);
            }
        }

        return std::move(tac_);
    }

private:
    TAC tac_;
    const SymbolTable& table_;
    std::string currentClass_;
    std::string currentMethod_;

    // ==================== Geração de métodos ====================

    /**
     * @brief Gera código para um método: emite label, processa corpo, emite return.
     * @param method    Nó da AST representando o método
     * @param className Nome da classe dona do método (usado no label: "Classe.Metodo")
     */
    void generateMethod(const MethodDecl* method, const std::string& className) {
        std::string label = className + "." + method->name;
        currentMethod_ = method->name;
        tac_.emit(OpCode::LABEL, "", label);

        // Gera código do corpo
        for (auto& stmt : method->body) {
            generateStmt(stmt.get());
        }

        // Gera código do return
        std::string retVal = generateExp(method->returnExp.get());
        tac_.emit(OpCode::RETURN, "", retVal);
    }

    // ==================== Geração de comandos ====================

    /**
     * @brief Gera código 3AC para um comando (statement).
     *
     * Recebe um ponteiro genérico Stmt* e usa dynamic_cast para descobrir
     * qual é o tipo concreto do comando (AssignStmt, IfStmt, WhileStmt, etc.).
     * dynamic_cast retorna nullptr se o tipo não bate, então funciona como
     * um "detector de tipo": se entrou no if, é porque o stmt ERA daquele tipo.
     */
    void generateStmt(const Stmt* stmt) {
        if (!stmt) return;

        if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
            std::string val = generateExp(assign->value.get());
            tac_.emit(OpCode::COPY, assign->variable, val);
        }
        else if (auto* arrAssign = dynamic_cast<const ArrayAssignStmt*>(stmt)) {
            std::string idx = generateExp(arrAssign->index.get());
            std::string val = generateExp(arrAssign->value.get());
            tac_.emit(OpCode::ARRAY_STORE, val, arrAssign->array, idx);
        }
        else if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
            generateIf(ifStmt);
        }
        else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
            generateWhile(whileStmt);
        }
        else if (auto* printStmt = dynamic_cast<const PrintStmt*>(stmt)) {
            std::string val = generateExp(printStmt->expression.get());
            tac_.emit(OpCode::PRINT, "", val);
        }
        else if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
            for (auto& s : block->stmts) generateStmt(s.get());
        }
    }

    // ==================== Geração de if ====================

    /**
     * @brief Gera código para um comando if/else.
     *
     * Template com else:          Template sem else:
     *   (código condição → t)       (código condição → t)
     *   ifFalse t goto Lelse        ifFalse t goto Lfim
     *   (código do then)            (código do then)
     *   goto Lfim                   Lfim:
     *   Lelse:
     *   (código do else)
     *   Lfim:
     */
    void generateIf(const IfStmt* ifStmt) {
        std::string cond = generateExp(ifStmt->condition.get());

        if (ifStmt->elseBody.empty()) {
            // if sem else
            std::string lEnd = tac_.newLabel();
            tac_.emit(OpCode::IF_FALSE, "", cond, lEnd);
            for (auto& s : ifStmt->thenBody) generateStmt(s.get());
            tac_.emit(OpCode::LABEL, "", lEnd);
        } else {
            // if com else
            std::string lElse = tac_.newLabel();
            std::string lEnd = tac_.newLabel();
            tac_.emit(OpCode::IF_FALSE, "", cond, lElse);
            for (auto& s : ifStmt->thenBody) generateStmt(s.get());
            tac_.emit(OpCode::GOTO, "", lEnd);
            tac_.emit(OpCode::LABEL, "", lElse);
            for (auto& s : ifStmt->elseBody) generateStmt(s.get());
            tac_.emit(OpCode::LABEL, "", lEnd);
        }
    }

    // ==================== Geração de while ====================

    /**
     * @brief Gera código para um comando while.
     *
     * Template:
     *   Linicio:
     *   (código condição → t)
     *   ifFalse t goto Lfim
     *   (código do corpo)
     *   goto Linicio
     *   Lfim:
     */
    void generateWhile(const WhileStmt* whileStmt) {
        std::string lStart = tac_.newLabel();
        std::string lEnd = tac_.newLabel();

        tac_.emit(OpCode::LABEL, "", lStart);
        std::string cond = generateExp(whileStmt->condition.get());
        tac_.emit(OpCode::IF_FALSE, "", cond, lEnd);
        for (auto& s : whileStmt->body) generateStmt(s.get());
        tac_.emit(OpCode::GOTO, "", lStart);
        tac_.emit(OpCode::LABEL, "", lEnd);
    }

    // ==================== Geração de expressões ====================

    /**
     * @brief Gera código 3AC para uma expressão e retorna onde o resultado ficou.
     *
     * Usa dynamic_cast para identificar o tipo concreto da expressão
     * (IntLiteral, Plus, MethodCall, etc.) e gera as instruções adequadas.
     *
     * @param exp Ponteiro para o nó de expressão na AST
     * @return Nome do temporário/variável que contém o resultado.
     *         Ex: "t3" para temporário, "num" para identificador, "this" para this.
     *
     * Percorrimento pós-ordem: primeiro gera código dos filhos (operandos),
     * depois emite a instrução do nó atual usando os resultados dos filhos.
     */
    std::string generateExp(const Exp* exp) {
        if (!exp) return "";

        // Literais
        if (auto* intLit = dynamic_cast<const IntLiteralExp*>(exp)) {
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::COPY, temp, std::to_string(intLit->value));
            return temp;
        }
        if (dynamic_cast<const TrueLiteralExp*>(exp)) {
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::COPY, temp, "1");
            return temp;
        }
        if (dynamic_cast<const FalseLiteralExp*>(exp)) {
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::COPY, temp, "0");
            return temp;
        }

        // Identificador — retorna o próprio nome, sem gerar instrução
        if (auto* id = dynamic_cast<const IdentifierExp*>(exp)) {
            return id->name;
        }

        // this — retorna "this" como referência
        if (dynamic_cast<const ThisExp*>(exp)) {
            return "this";
        }

        // Operações binárias
        if (auto* plus = dynamic_cast<const PlusExp*>(exp)) {
            return generateBinOp(OpCode::ADD, plus->left.get(), plus->right.get());
        }
        if (auto* minus = dynamic_cast<const MinusExp*>(exp)) {
            return generateBinOp(OpCode::SUB, minus->left.get(), minus->right.get());
        }
        if (auto* times = dynamic_cast<const TimesExp*>(exp)) {
            return generateBinOp(OpCode::MUL, times->left.get(), times->right.get());
        }
        if (auto* lt = dynamic_cast<const LessThanExp*>(exp)) {
            return generateBinOp(OpCode::LT, lt->left.get(), lt->right.get());
        }
        if (auto* andExp = dynamic_cast<const AndExp*>(exp)) {
            return generateBinOp(OpCode::AND, andExp->left.get(), andExp->right.get());
        }

        // Not
        if (auto* notExp = dynamic_cast<const NotExp*>(exp)) {
            std::string val = generateExp(notExp->expr.get());
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::NOT, temp, val);
            return temp;
        }

        // Array lookup: a[i]
        if (auto* arrLookup = dynamic_cast<const ArrayLookupExp*>(exp)) {
            std::string arr = generateExp(arrLookup->array.get());
            std::string idx = generateExp(arrLookup->index.get());
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::ARRAY_LOAD, temp, arr, idx);
            return temp;
        }

        // Array length
        if (auto* arrLen = dynamic_cast<const ArrayLengthExp*>(exp)) {
            std::string arr = generateExp(arrLen->array.get());
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::ARRAY_LENGTH, temp, arr);
            return temp;
        }

        // New object
        if (auto* newObj = dynamic_cast<const NewObjectExp*>(exp)) {
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::NEW_OBJ, temp, newObj->className);
            return temp;
        }

        // New array
        if (auto* newArr = dynamic_cast<const NewArrayExp*>(exp)) {
            std::string size = generateExp(newArr->size.get());
            std::string temp = tac_.newTemp();
            tac_.emit(OpCode::NEW_ARRAY, temp, size);
            return temp;
        }

        // Method call: obj.method(args)
        if (auto* call = dynamic_cast<const MethodCallExp*>(exp)) {
           // 1. Gera código para o objeto
           std::string obj = generateExp(call->object.get());
           tac_.emit(OpCode::PARAM, "", obj);
    
           // 2. Gera código para cada argumento
           for (auto& arg : call->args) {
               std::string a = generateExp(arg.get());
               tac_.emit(OpCode::PARAM, "", a);
           }
           
           // 3. Determina o tipo do objeto para resolver o método
           std::string objType = inferObjectType(call->object.get());
           if (objType.empty()) {
               // Não foi possível determinar o tipo do objeto
               // (já deve ter sido verificado pela análise semântica)
               return "";
           }
           
           // 4. Busca o método (com suporte a herança)
           const Symbol* method = table_.lookupMethod(objType, call->method);
           if (!method) {
               // Erro: método não encontrado (não deveria acontecer se a análise semântica passou)
               return "";
           }
           
           // 5. Cria o label completo e gera a instrução CALL
           std::string temp = tac_.newTemp();
           int numArgs = (int)call->args.size() + 1;  // +1 para o objeto (this)
           std::string methodLabel = objType + "." + call->method;
           
           tac_.emit(OpCode::CALL, temp, methodLabel, std::to_string(numArgs));
           return temp;
       }

        return "";
    }

    // ==================== Auxiliar para operações binárias ====================

    /**
     * @brief Gera código para uma operação binária genérica (add, sub, mul, lt, and).
     *
     * Padrão: gera código do lado esquerdo, gera código do lado direito,
     * depois emite uma instrução combinando ambos num temporário novo.
     *
     * @param op    OpCode da operação (ADD, SUB, MUL, LT, AND)
     * @param left  Nó da subexpressão esquerda
     * @param right Nó da subexpressão direita
     * @return Nome do temporário com o resultado (ex: "t5")
     */
    std::string generateBinOp(OpCode op, const Exp* left, const Exp* right) {
        std::string l = generateExp(left);
        std::string r = generateExp(right);
        std::string temp = tac_.newTemp();
        tac_.emit(op, temp, l, r);
        return temp;
    }

    std::string currentScope() const {
        if (currentMethod_.empty()) return currentClass_;
        return currentClass_ + "." + currentMethod_;
    }

    std::string inferObjectType(const Exp* exp) const {
        if (!exp) return "";

        // Caso 1: Identificador - buscar na tabela de símbolos
        if (auto* id = dynamic_cast<const IdentifierExp*>(exp)) {
            const Symbol* sym = table_.lookup(id->name, currentScope());
            if (sym) return sym->type;
            return "";
        }

        // Caso 2: new Classe()
        if (auto* newObj = dynamic_cast<const NewObjectExp*>(exp)) {
            return newObj->className;
        }

        // Caso 3: this
        if (dynamic_cast<const ThisExp*>(exp)) {
            return currentClass_;
        }

        // Caso 4: Chamada de método encadeada: obj.metodo1().metodo2()
        if (auto* call = dynamic_cast<const MethodCallExp*>(exp)) {
            std::string objType = inferObjectType(call->object.get());
            const Symbol* method = table_.lookupMethod(objType, call->method);
            if (method) return method->type;
            return "";
        }

        return "";
    }
};

#endif // CODEGEN_H
