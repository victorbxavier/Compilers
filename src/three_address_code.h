/**
 * @file three_address_code.h
 * @brief Estrutura do Código de Três Endereços (3AC).
 *
 * Cada instrução 3AC tem a forma: result = op arg1 arg2
 * Onde result, arg1, arg2 são endereços (nomes de variáveis, temporários ou constantes)
 * e op é o código de operação.
 *
 * Este módulo também fornece funções para criar temporários (t0, t1, ...)
 * e labels (L0, L1, ...) usados no controle de fluxo.
 */

#ifndef THREE_ADDRESS_CODE_H
#define THREE_ADDRESS_CODE_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

/**
 * @enum OpCode
 * @brief Operações possíveis no código de três endereços.
 */
enum class OpCode {
    // Aritméticas
    ADD,            // result = arg1 + arg2
    SUB,            // result = arg1 - arg2
    MUL,            // result = arg1 * arg2

    // Relacionais e lógicas
    LT,             // result = arg1 < arg2
    AND,            // result = arg1 && arg2
    NOT,            // result = !arg1

    // Cópia e literais
    COPY,           // result = arg1

    // Controle de fluxo
    LABEL,          // arg1:  (define um rótulo)
    GOTO,           // goto arg1
    IF_TRUE,        // if arg1 goto arg2
    IF_FALSE,       // ifFalse arg1 goto arg2

    // Chamada de funções
    PARAM,          // param arg1
    CALL,           // result = call arg1, arg2 (arg2 = nº de args)
    RETURN,         // return arg1

    // I/O
    PRINT,          // print arg1

    // Arrays
    NEW_ARRAY,      // result = new int[arg1]
    ARRAY_LOAD,     // result = arg1[arg2]
    ARRAY_STORE,    // arg1[arg2] = result
    ARRAY_LENGTH,   // result = length arg1

    // Objetos
    NEW_OBJ,        // result = new arg1

    // Programa
    HALT,           // encerra execução
    NOP             // sem operação
};

/**
 * @brief Converte OpCode para string legível.
 */
inline std::string opcodeToString(OpCode op) {
    switch (op) {
        case OpCode::ADD:          return "add";
        case OpCode::SUB:          return "sub";
        case OpCode::MUL:          return "mul";
        case OpCode::LT:           return "lt";
        case OpCode::AND:          return "and";
        case OpCode::NOT:          return "not";
        case OpCode::COPY:         return "copy";
        case OpCode::LABEL:        return "label";
        case OpCode::GOTO:         return "goto";
        case OpCode::IF_TRUE:      return "ifTrue";
        case OpCode::IF_FALSE:     return "ifFalse";
        case OpCode::PARAM:        return "param";
        case OpCode::CALL:         return "call";
        case OpCode::RETURN:       return "return";
        case OpCode::PRINT:        return "print";
        case OpCode::NEW_ARRAY:    return "new_array";
        case OpCode::ARRAY_LOAD:   return "array_load";
        case OpCode::ARRAY_STORE:  return "array_store";
        case OpCode::ARRAY_LENGTH: return "array_length";
        case OpCode::NEW_OBJ:      return "new_obj";
        case OpCode::HALT:         return "halt";
        case OpCode::NOP:          return "nop";
    }
    return "unknown";
}

/**
 * @struct Instruction
 * @brief Uma instrução de código de três endereços.
 */
struct Instruction {
    OpCode op;
    std::string result;  // Destino (ou label para LABEL)
    std::string arg1;    // Primeiro operando
    std::string arg2;    // Segundo operando (pode ser vazio)
};

/**
 * @class TAC
 * @brief Lista de instruções 3AC com funções auxiliares.
 */
class TAC {
public:
    TAC() : tempCount_(0), labelCount_(0) {}

    /** Emite uma nova instrução e a adiciona ao final da lista.
     *  @param op     Código da operação (ADD, SUB, GOTO, etc.)
     *  @param result Endereço destino (variável ou temporário que recebe o resultado)
     *  @param arg1   Primeiro operando (ou label para GOTO/LABEL)
     *  @param arg2   Segundo operando (pode ser vazio para operações unárias)
     */
    void emit(OpCode op, const std::string& result = "",
              const std::string& arg1 = "", const std::string& arg2 = "") {
        instructions_.push_back({op, result, arg1, arg2});
    }

    /** Concatena todas as instruções de outra lista TAC no final desta.
     *  Usado quando se combina código gerado por subárvores diferentes.
     */
    void concat(const TAC& other) {
        for (auto& instr : other.instructions_) {
            instructions_.push_back(instr);
        }
    }

    /** Cria um novo temporário único (t0, t1, t2, ...).
     *  Temporários guardam resultados intermediários de expressões.
     *  Ex: em "a + b * c", o resultado de "b * c" fica em um temporário.
     */
    std::string newTemp() {
        return "t" + std::to_string(tempCount_++);
    }

    /** Cria um novo label único (L0, L1, L2, ...).
     *  Labels marcam pontos no código para onde desvios (goto/if) podem saltar.
     *  Ex: início de while, bloco else de um if, fim de um loop.
     */
    std::string newLabel() {
        return "L" + std::to_string(labelCount_++);
    }

    /** Imprime todas as instruções numeradas no formato legível.
     *  Cada linha mostra: "índice:  instrução formatada"
     */
    void print() const {
        for (size_t i = 0; i < instructions_.size(); i++) {
            const auto& instr = instructions_[i];
            printInstruction(instr, i);
        }
    }

    /** Retorna a lista de instruções (somente leitura). */
    const std::vector<Instruction>& getInstructions() const { return instructions_; }

    /** Retorna a quantidade total de instruções geradas. */
    size_t size() const { return instructions_.size(); }

private:
    std::vector<Instruction> instructions_;
    int tempCount_;
    int labelCount_;

    /** Imprime uma única instrução formatada.
     *  Cada OpCode tem um formato de impressão específico:
     *  - LABEL → "L0:"
     *  - GOTO → "goto L0"
     *  - IF_FALSE → "ifFalse t1 goto L0"
     *  - Binários → "t1 = add x y"
     *  - COPY → "x = y"
     *  etc.
     */
    void printInstruction(const Instruction& instr, size_t index) const {
        // Numeração opcional
        std::cout << std::setw(4) << index << ":  ";

        switch (instr.op) {
            case OpCode::LABEL:
                std::cout << instr.arg1 << ":" << std::endl;
                return;
            case OpCode::GOTO:
                std::cout << "goto " << instr.arg1 << std::endl;
                return;
            case OpCode::IF_TRUE:
                std::cout << "if " << instr.arg1 << " goto " << instr.arg2 << std::endl;
                return;
            case OpCode::IF_FALSE:
                std::cout << "ifFalse " << instr.arg1 << " goto " << instr.arg2 << std::endl;
                return;
            case OpCode::PARAM:
                std::cout << "param " << instr.arg1 << std::endl;
                return;
            case OpCode::CALL:
                if (!instr.result.empty())
                    std::cout << instr.result << " = call " << instr.arg1 << ", " << instr.arg2 << std::endl;
                else
                    std::cout << "call " << instr.arg1 << ", " << instr.arg2 << std::endl;
                return;
            case OpCode::RETURN:
                std::cout << "return " << instr.arg1 << std::endl;
                return;
            case OpCode::PRINT:
                std::cout << "print " << instr.arg1 << std::endl;
                return;
            case OpCode::HALT:
                std::cout << "halt" << std::endl;
                return;
            case OpCode::COPY:
                std::cout << instr.result << " = " << instr.arg1 << std::endl;
                return;
            case OpCode::NOT:
                std::cout << instr.result << " = not " << instr.arg1 << std::endl;
                return;
            case OpCode::NEW_OBJ:
                std::cout << instr.result << " = new " << instr.arg1 << std::endl;
                return;
            case OpCode::NEW_ARRAY:
                std::cout << instr.result << " = new int[" << instr.arg1 << "]" << std::endl;
                return;
            case OpCode::ARRAY_LOAD:
                std::cout << instr.result << " = " << instr.arg1 << "[" << instr.arg2 << "]" << std::endl;
                return;
            case OpCode::ARRAY_STORE:
                std::cout << instr.arg1 << "[" << instr.arg2 << "] = " << instr.result << std::endl;
                return;
            case OpCode::ARRAY_LENGTH:
                std::cout << instr.result << " = length " << instr.arg1 << std::endl;
                return;
            case OpCode::NOP:
                std::cout << "nop" << std::endl;
                return;
            default:
                // Operações binárias: result = op arg1 arg2
                std::cout << instr.result << " = " << opcodeToString(instr.op)
                          << " " << instr.arg1 << " " << instr.arg2 << std::endl;
                return;
        }
    }
};

#endif // THREE_ADDRESS_CODE_H
