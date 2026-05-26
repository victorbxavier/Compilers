/**
 * @file token.h
 * @brief Definição dos tipos de token da linguagem MiniJava.
 *
 * Este arquivo é o "contrato" central do compilador. Todos os módulos
 * (lexer, parser, main) dependem das definições aqui presentes.
 * Ele define:
 *   - TokenType: enumeração com todas as categorias de token
 *   - Token: estrutura que representa um token individual
 *   - Funções utilitárias para conversão e impressão de tokens
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <iostream>

/**
 * @enum TokenType
 * @brief Enumeração de todos os tipos de token reconhecidos pelo compilador.
 *
 * Organizada em 5 categorias:
 *   1. Palavras-chave: tokens reservados da linguagem (class, if, while, etc.)
 *   2. Operadores: símbolos de operações aritméticas, lógicas e atribuição
 *   3. Delimitadores: símbolos de agrupamento e separação
 *   4. Literais/Identificadores: nomes definidos pelo programador e números
 *   5. Controle: tokens especiais usados internamente pelo compilador
 */
enum class TokenType {
    // === Palavras-chave ===
    // Cada palavra reservada da linguagem MiniJava tem seu próprio tipo.
    // Isso permite que o parser identifique rapidamente a construção sintática.
    CLASS,      // Declaração de classe
    PUBLIC,     // Modificador de acesso
    STATIC,     // Modificador estático (usado apenas no main)
    VOID,       // Tipo de retorno vazio (usado apenas no main)
    MAIN,       // Identificador especial do método principal
    STRING,     // Tipo String (usado apenas no parâmetro do main)
    RETURN,     // Instrução de retorno de método
    INT,        // Tipo inteiro
    BOOLEAN,    // Tipo booleano
    IF,         // Condicional
    ELSE,       // Ramo alternativo do condicional
    WHILE,      // Laço de repetição
    SYSTEM,     // Parte de System.out.println
    OUT,        // Parte de System.out.println
    PRINTLN,    // Parte de System.out.println
    TRUE,       // Literal booleano verdadeiro
    FALSE,      // Literal booleano falso
    THIS,       // Referência ao objeto atual
    NEW,        // Instanciação de objeto ou array
    EXTENDS,    // Herança de classe
    LENGTH,     // Propriedade de tamanho de array

    // === Operadores ===
    // Símbolos que representam operações sobre valores.
    AND,        // && (E lógico)
    LT,         // <  (menor que)
    GT,         // >  (maior que)
    PLUS,       // +  (adição)
    MINUS,      // -  (subtração)
    MULT,       // *  (multiplicação)
    ASSIGN,     // =  (atribuição)
    NOT,        // !  (negação lógica)

    // === Delimitadores ===
    // Símbolos que delimitam blocos, expressões e listas.
    LPAREN,     // (  abre parêntese
    RPAREN,     // )  fecha parêntese
    LBRACKET,   // [  abre colchete (acesso a array)
    RBRACKET,   // ]  fecha colchete
    LBRACE,     // {  abre chave (início de bloco)
    RBRACE,     // }  fecha chave (fim de bloco)
    SEMICOLON,  // ;  fim de instrução
    COMMA,      // ,  separador de argumentos
    DOT,        // .  acesso a membro (ex: obj.metodo())

    // === Literais e Identificadores ===
    ID,         // Nome definido pelo programador (variável, classe, método)
    NUMBER,     // Literal numérico inteiro (ex: 42, 0, 1000)

    // === Controle interno ===
    END_OF_FILE, // Marca o fim da entrada — usado pelo parser para saber quando parar
    UNKNOWN      // Token não reconhecido — indica erro léxico
};

/**
 * @struct Token
 * @brief Representa um token individual extraído do código fonte.
 *
 * Cada token carrega 3 informações:
 *   - type: a categoria do token (qual regra léxica o gerou)
 *   - lexeme: o texto original no código fonte (ex: "factorial", "42", "+")
 *   - line: número da linha onde o token aparece (para mensagens de erro)
 *
 * Exemplo: para o código "int x", o lexer gera:
 *   Token{INT, "int", 1} e Token{ID, "x", 1}
 */
struct Token {
    TokenType type;        // Categoria do token
    std::string lexeme;    // Texto original no código fonte
    int line;              // Linha onde o token foi encontrado
};

/**
 * @brief Converte um TokenType para sua representação textual.
 *
 * Usado em mensagens de erro do parser para informar ao usuário
 * qual token era esperado e qual foi encontrado.
 * Ex: "esperado SEMICOLON mas encontrou RBRACE"
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
 * @brief Imprime um token no formato <TIPO, "lexema", line N>.
 *
 * Sempre inclui o lexema na saída para facilitar a leitura.
 * O tipo é o nome interno (ex: SEMICOLON), enquanto o lexema é o texto
 * real no código fonte (ex: ";"). Mostrar ambos torna a saída mais clara.
 *
 * Exemplos de saída:
 *   <CLASS, "class", line 1>
 *   <SEMICOLON, ";", line 4>
 *   <ID, "factorial", line 3>
 *   <NUMBER, "10", line 5>
 */
inline void printToken(const Token& tok) {
    std::cout << "<" << tokenTypeToString(tok.type)
              << ", \"" << tok.lexeme << "\""
              << ", line " << tok.line << ">" << std::endl;
}

#endif // TOKEN_H
