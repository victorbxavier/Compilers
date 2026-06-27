/**
 * @file token.h
 * @brief Definição dos tipos de token da linguagem MiniJava.
 *
 * Este arquivo é o "contrato" central do compilador. Todos os módulos
 * (lexer, parser, main) dependem das definições aqui presentes.
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <iostream>

/**
 * @enum TokenType
 * @brief Enumeração de todos os tipos de token reconhecidos pelo compilador.
 */
enum class TokenType {
    // === Palavras-chave ===
    CLASS, PUBLIC, STATIC, VOID, MAIN, STRING, RETURN,
    INT, BOOLEAN, IF, ELSE, WHILE,
    SYSTEM, OUT, PRINTLN,
    TRUE, FALSE, THIS, NEW, EXTENDS, LENGTH,

    // === Operadores ===
    AND,        // &&
    LT,         // <
    GT,         // >
    PLUS,       // +
    MINUS,      // -
    MULT,       // *
    ASSIGN,     // =
    NOT,        // !

    // === Delimitadores ===
    LPAREN, RPAREN,       // ( )
    LBRACKET, RBRACKET,   // [ ]
    LBRACE, RBRACE,       // { }
    SEMICOLON,            // ;
    COMMA,                // ,
    DOT,                  // .

    // === Literais e Identificadores ===
    ID,         // Identificador
    NUMBER,     // Literal numérico inteiro

    // === Controle interno ===
    END_OF_FILE,
    UNKNOWN
};

/**
 * @struct Token
 * @brief Representa um token individual extraído do código fonte.
 *
 * Cada token carrega:
 *   - type: categoria do token
 *   - lexeme: texto original no código fonte
 *   - line: linha onde aparece
 *   - column: coluna onde começa
 */
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

/**
 * @brief Converte um TokenType para sua representação textual.
 */
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::CLASS:      return "CLASS";
        case TokenType::PUBLIC:     return "PUBLIC";
        case TokenType::STATIC:     return "STATIC";
        case TokenType::VOID:       return "VOID";
        case TokenType::MAIN:       return "MAIN";
        case TokenType::STRING:     return "STRING";
        case TokenType::RETURN:     return "RETURN";
        case TokenType::INT:        return "INT";
        case TokenType::BOOLEAN:    return "BOOLEAN";
        case TokenType::IF:         return "IF";
        case TokenType::ELSE:       return "ELSE";
        case TokenType::WHILE:      return "WHILE";
        case TokenType::SYSTEM:     return "SYSTEM";
        case TokenType::OUT:        return "OUT";
        case TokenType::PRINTLN:    return "PRINTLN";
        case TokenType::TRUE:       return "TRUE";
        case TokenType::FALSE:      return "FALSE";
        case TokenType::THIS:       return "THIS";
        case TokenType::NEW:        return "NEW";
        case TokenType::EXTENDS:    return "EXTENDS";
        case TokenType::LENGTH:     return "LENGTH";
        case TokenType::AND:        return "AND";
        case TokenType::LT:         return "LT";
        case TokenType::GT:         return "GT";
        case TokenType::PLUS:       return "PLUS";
        case TokenType::MINUS:      return "MINUS";
        case TokenType::MULT:       return "MULT";
        case TokenType::ASSIGN:     return "ASSIGN";
        case TokenType::NOT:        return "NOT";
        case TokenType::LPAREN:     return "LPAREN";
        case TokenType::RPAREN:     return "RPAREN";
        case TokenType::LBRACKET:   return "LBRACKET";
        case TokenType::RBRACKET:   return "RBRACKET";
        case TokenType::LBRACE:     return "LBRACE";
        case TokenType::RBRACE:     return "RBRACE";
        case TokenType::SEMICOLON:  return "SEMICOLON";
        case TokenType::COMMA:      return "COMMA";
        case TokenType::DOT:        return "DOT";
        case TokenType::ID:         return "ID";
        case TokenType::NUMBER:     return "NUMBER";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::UNKNOWN:    return "UNKNOWN";
    }
    return "UNKNOWN";
}

/**
 * @brief Imprime um token no formato <TIPO, "lexema", line:col>.
 */
inline void printToken(const Token& tok) {
    std::cout << "<" << tokenTypeToString(tok.type)
              << ", \"" << tok.lexeme << "\""
              << ", " << tok.line << ":" << tok.column
              << ">" << std::endl;
}

#endif // TOKEN_H
