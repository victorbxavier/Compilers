/**
 * @file parser.h
 * @brief Analisador Sintatico (Parser) do compilador MiniJava.
 *
 * Implementa um parser recursivo descendente (recursive descent) para
 * a gramatica LL(1) da linguagem MiniJava. Cada regra da gramatica
 * corresponde a uma funcao privada da classe Parser.
 *
 * O parser consome a lista de tokens produzida pelo lexer e:
 *   1. Valida se a sequencia respeita a gramatica
 *   2. Constroi a tabela de simbolos durante o parsing
 *   3. Reporta o primeiro erro sintatico encontrado (sem recuperacao)
 *
 * Tecnica: lookahead de 1 token (LL(1)) -- usa currentType() para
 * decidir qual producao seguir em cada ponto da gramatica.
 */

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

class Parser {
public:
    /**
     * @brief Construtor -- recebe a lista de tokens do lexer.
     * Inicializa o cursor (pos_) no inicio e sem erros.
     */
    Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0), hadError_(false) {}

    /**
     * @brief Ponto de entrada da analise sintatica.
     * Chama parseProg() (regra inicial da gramatica) e verifica
     * se todos os tokens foram consumidos (espera END_OF_FILE).
     * @return true se o programa e sintaticamente correto
     */
    bool parse() {
        parseProg();
        if (!hadError_) {
            match(TokenType::END_OF_FILE);
        }
        return !hadError_;
    }

    bool hadError() const { return hadError_; }

    const SymbolTable& getSymbolTable() const { return symbolTable_; }

private:
    std::vector<Token> tokens_;    // Lista de tokens recebida do lexer
    int pos_;                      // Cursor -- indice do token atual
    bool hadError_;                // Flag: true apos o primeiro erro (para a analise)
    SymbolTable symbolTable_;      // Tabela de simbolos preenchida durante o parsing
    std::string currentClass_;     // Nome da classe sendo parseada (escopo atual)
    std::string currentMethod_;    // Nome do metodo sendo parseado (escopo atual)

    // ==================== Funcoes auxiliares ====================

    // Retorna o token na posicao atual do cursor (lookahead)
    Token currentToken() const {
        if (pos_ < (int)tokens_.size()) {
            return tokens_[pos_];
        }
        Token eof;
        eof.type = TokenType::END_OF_FILE;
        eof.lexeme = "$";
        eof.line = tokens_.back().line;
        return eof;
    }

    // Alias para currentToken()
    Token peek() const {
        return currentToken();
    }

    // Retorna o tipo do token atual (usado para decisoes de lookahead)
    TokenType currentType() const {
        return currentToken().type;
    }

    // Avanca o cursor para o proximo token
    void advance() {
        if (pos_ < (int)tokens_.size()) {
            pos_++;
        }
    }

    // Consome o token atual se for do tipo esperado, senao reporta erro
    void match(TokenType expected) {
        if (currentType() == expected) {
            advance();
        } else {
            error(expected);
        }
    }

    // Consome o token esperado e retorna seu lexema (usado para capturar nomes)
    std::string matchAndGet(TokenType expected) {
        if (currentType() == expected) {
            std::string lex = currentToken().lexeme;
            advance();
            return lex;
        } else {
            error(expected);
            return "";
        }
    }

    // Retorna o escopo atual formatado (ex: "Factorial.compute")
    std::string currentScope() const {
        if (currentMethod_.empty()) {
            return currentClass_;
        }
        return currentClass_ + "." + currentMethod_;
    }

    // Reporta erro sintatico indicando token esperado vs encontrado
    void error(TokenType expected) {
        if (hadError_) return;
        hadError_ = true;
        Token tok = currentToken();
        std::cerr << "Erro sintático na linha " << tok.line << ": "
                  << "esperado " << tokenTypeToString(expected)
                  << " mas encontrou " << tokenTypeToString(tok.type);
        if (tok.type == TokenType::ID || tok.type == TokenType::NUMBER) {
            std::cerr << " (\"" << tok.lexeme << "\")";
        }
        std::cerr << std::endl;
    }

    void error(const std::string& msg) {
        if (hadError_) return;
        hadError_ = true;
        Token tok = currentToken();
        std::cerr << "Erro sintático na linha " << tok.line << ": " << msg;
        std::cerr << " (encontrou " << tokenTypeToString(tok.type);
        if (tok.type == TokenType::ID || tok.type == TokenType::NUMBER) {
            std::cerr << " \"" << tok.lexeme << "\"";
        }
        std::cerr << ")" << std::endl;
    }

    // Parseia um tipo (int, int[], boolean, ou Id) e retorna como string
    std::string parseTypeAndGet() {
        if (hadError_) return "";
        if (currentType() == TokenType::INT) {
            match(TokenType::INT);
            if (hadError_) return "";
            if (currentType() == TokenType::LBRACKET) {
                match(TokenType::LBRACKET);
                if (hadError_) return "";
                match(TokenType::RBRACKET);
                return "int[]";
            }
            return "int";
        } else if (currentType() == TokenType::BOOLEAN) {
            match(TokenType::BOOLEAN);
            return "boolean";
        } else if (currentType() == TokenType::ID) {
            std::string typeName = currentToken().lexeme;
            match(TokenType::ID);
            return typeName;
        } else {
            error("esperado um tipo (int, boolean, int[] ou identificador)");
            return "";
        }
    }

    // ==================== Regras da gramatica ====================
    // Cada funcao abaixo corresponde a uma producao da gramatica LL(1).
    // O comentario acima de cada funcao mostra a producao formal.
    // "lambda" indica producao vazia (a funcao simplesmente retorna).

    // Prog -> MainC DefCl
    void parseProg() {
        parseMainC();
        if (hadError_) return;
        parseDefCl();
    }

    // MainC -> 'class' Id '{' 'public' 'static' 'void' 'main' '(' 'String' '[' ']' Id ')' '{' CmdList '}' '}'
    // Parseia a classe principal (que contem o metodo main)
    void parseMainC() {
        match(TokenType::CLASS);
        if (hadError_) return;

        std::string className = matchAndGet(TokenType::ID);
        if (hadError_) return;
        int classLine = tokens_[pos_ - 1].line;
        currentClass_ = className;
        symbolTable_.addSymbol(className, "class", "global", classLine, SymbolCategory::CLASS);

        match(TokenType::LBRACE);
        if (hadError_) return;
        match(TokenType::PUBLIC);
        if (hadError_) return;
        match(TokenType::STATIC);
        if (hadError_) return;
        match(TokenType::VOID);
        if (hadError_) return;
        match(TokenType::MAIN);
        if (hadError_) return;

        currentMethod_ = "main";
        symbolTable_.addSymbol("main", "void", currentClass_, tokens_[pos_ - 1].line, SymbolCategory::METHOD);

        match(TokenType::LPAREN);
        if (hadError_) return;
        match(TokenType::STRING);
        if (hadError_) return;
        match(TokenType::LBRACKET);
        if (hadError_) return;
        match(TokenType::RBRACKET);
        if (hadError_) return;

        std::string paramName = matchAndGet(TokenType::ID);
        if (hadError_) return;
        symbolTable_.addSymbol(paramName, "String[]", currentScope(), tokens_[pos_ - 1].line, SymbolCategory::PARAMETER);

        match(TokenType::RPAREN);
        if (hadError_) return;
        match(TokenType::LBRACE);
        if (hadError_) return;
        parseCmdList();
        if (hadError_) return;
        match(TokenType::RBRACE);
        if (hadError_) return;
        match(TokenType::RBRACE);

        currentMethod_ = "";
        currentClass_ = "";
    }

    // DefCl -> 'class' Id DefCl' | lambda
    // Parseia zero ou mais definicoes de classe apos a classe principal
    void parseDefCl() {
        if (hadError_) return;
        if (currentType() == TokenType::CLASS) {
            match(TokenType::CLASS);
            if (hadError_) return;

            std::string className = matchAndGet(TokenType::ID);
            if (hadError_) return;
            int classLine = tokens_[pos_ - 1].line;
            currentClass_ = className;
            symbolTable_.addSymbol(className, "class", "global", classLine, SymbolCategory::CLASS);

            parseDefClTail();
            currentClass_ = "";
        }
        // λ
    }

    // DefCl' -> '{' DefVar DefMet '}' DefCl
    //          | 'extends' Id '{' DefVar DefMet '}' DefCl
    // Corpo da classe: pode ter heranca (extends) ou nao
    void parseDefClTail() {
        if (hadError_) return;
        if (currentType() == TokenType::LBRACE) {
            match(TokenType::LBRACE);
            if (hadError_) return;
            parseDefVar(SymbolCategory::INSTANCE_VAR);
            if (hadError_) return;
            parseDefMet();
            if (hadError_) return;
            match(TokenType::RBRACE);
            if (hadError_) return;
            parseDefCl();
        } else if (currentType() == TokenType::EXTENDS) {
            match(TokenType::EXTENDS);
            if (hadError_) return;
            match(TokenType::ID);
            if (hadError_) return;
            match(TokenType::LBRACE);
            if (hadError_) return;
            parseDefVar(SymbolCategory::INSTANCE_VAR);
            if (hadError_) return;
            parseDefMet();
            if (hadError_) return;
            match(TokenType::RBRACE);
            if (hadError_) return;
            parseDefCl();
        } else {
            error("esperado '{' ou 'extends' após nome da classe");
        }
    }

    // DefVar -> Type Id ';' DefVar | lambda
    // Parseia declaracoes de variaveis. Usa lookahead para distinguir
    // declaracao de variavel (Type Id ;) de inicio de metodo ou comando.
    // O parametro varCategory indica se e variavel de instancia ou local.
    void parseDefVar(SymbolCategory varCategory) {
        if (hadError_) return;
        if (isTypeStart()) {
            int savedPos = pos_;
            bool savedError = hadError_;

            if (currentType() == TokenType::INT) {
                advance();
                if (currentType() == TokenType::LBRACKET) {
                    advance();
                    if (currentType() == TokenType::RBRACKET) {
                        advance();
                        if (currentType() == TokenType::ID) {
                            advance();
                            if (currentType() == TokenType::SEMICOLON) {
                                pos_ = savedPos;
                                hadError_ = savedError;
                                std::string type = parseTypeAndGet();
                                if (hadError_) return;
                                std::string name = matchAndGet(TokenType::ID);
                                if (hadError_) return;
                                int line = tokens_[pos_ - 1].line;
                                symbolTable_.addSymbol(name, type, currentScope(), line, varCategory);
                                match(TokenType::SEMICOLON);
                                if (hadError_) return;
                                parseDefVar(varCategory);
                                return;
                            }
                        }
                    }
                } else if (currentType() == TokenType::ID) {
                    advance();
                    if (currentType() == TokenType::SEMICOLON) {
                        pos_ = savedPos;
                        hadError_ = savedError;
                        std::string type = parseTypeAndGet();
                        if (hadError_) return;
                        std::string name = matchAndGet(TokenType::ID);
                        if (hadError_) return;
                        int line = tokens_[pos_ - 1].line;
                        symbolTable_.addSymbol(name, type, currentScope(), line, varCategory);
                        match(TokenType::SEMICOLON);
                        if (hadError_) return;
                        parseDefVar(varCategory);
                        return;
                    }
                }
            } else if (currentType() == TokenType::BOOLEAN) {
                advance();
                if (currentType() == TokenType::ID) {
                    advance();
                    if (currentType() == TokenType::SEMICOLON) {
                        pos_ = savedPos;
                        hadError_ = savedError;
                        std::string type = parseTypeAndGet();
                        if (hadError_) return;
                        std::string name = matchAndGet(TokenType::ID);
                        if (hadError_) return;
                        int line = tokens_[pos_ - 1].line;
                        symbolTable_.addSymbol(name, type, currentScope(), line, varCategory);
                        match(TokenType::SEMICOLON);
                        if (hadError_) return;
                        parseDefVar(varCategory);
                        return;
                    }
                }
            } else if (currentType() == TokenType::ID) {
                advance();
                if (currentType() == TokenType::ID) {
                    advance();
                    if (currentType() == TokenType::SEMICOLON) {
                        pos_ = savedPos;
                        hadError_ = savedError;
                        std::string type = parseTypeAndGet();
                        if (hadError_) return;
                        std::string name = matchAndGet(TokenType::ID);
                        if (hadError_) return;
                        int line = tokens_[pos_ - 1].line;
                        symbolTable_.addSymbol(name, type, currentScope(), line, varCategory);
                        match(TokenType::SEMICOLON);
                        if (hadError_) return;
                        parseDefVar(varCategory);
                        return;
                    }
                }
            }

            pos_ = savedPos;
            hadError_ = savedError;
        }
        // λ
    }

    // DefMet -> 'public' Type Id '(' Args ')' '{' DefVar CmdList 'return' Exp ';' '}' DefMet | lambda
    // Parseia zero ou mais definicoes de metodo dentro de uma classe
    void parseDefMet() {
        if (hadError_) return;
        if (currentType() == TokenType::PUBLIC) {
            match(TokenType::PUBLIC);
            if (hadError_) return;

            std::string retType = parseTypeAndGet();
            if (hadError_) return;

            std::string methodName = matchAndGet(TokenType::ID);
            if (hadError_) return;
            int methodLine = tokens_[pos_ - 1].line;
            currentMethod_ = methodName;
            symbolTable_.addSymbol(methodName, retType, currentClass_, methodLine, SymbolCategory::METHOD);

            match(TokenType::LPAREN);
            if (hadError_) return;
            parseDefMetArgs();
            if (hadError_) return;
            match(TokenType::RPAREN);
            if (hadError_) return;
            match(TokenType::LBRACE);
            if (hadError_) return;
            parseDefVar(SymbolCategory::LOCAL_VAR);
            if (hadError_) return;
            parseCmdList();
            if (hadError_) return;
            match(TokenType::RETURN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::SEMICOLON);
            if (hadError_) return;
            match(TokenType::RBRACE);
            if (hadError_) return;

            currentMethod_ = "";
            parseDefMet();
        }
        // λ
    }

    // DefMetArgs -> Args | lambda
    // Parseia os parametros formais de um metodo (pode ser vazio)
    void parseDefMetArgs() {
        if (hadError_) return;
        if (isTypeStart()) {
            parseArgs();
        }
        // λ
    }

    // Type -> 'int' '[' ']' | 'boolean' | 'int' | Id
    void parseType() {
        if (hadError_) return;
        if (currentType() == TokenType::INT) {
            match(TokenType::INT);
            if (hadError_) return;
            if (currentType() == TokenType::LBRACKET) {
                match(TokenType::LBRACKET);
                if (hadError_) return;
                match(TokenType::RBRACKET);
            }
        } else if (currentType() == TokenType::BOOLEAN) {
            match(TokenType::BOOLEAN);
        } else if (currentType() == TokenType::ID) {
            match(TokenType::ID);
        } else {
            error("esperado um tipo (int, boolean, int[] ou identificador)");
        }
    }

    // Args -> Type Id Args'
    // Parseia lista de parametros e registra cada um na tabela de simbolos
    void parseArgs() {
        if (hadError_) return;
        std::string type = parseTypeAndGet();
        if (hadError_) return;
        std::string name = matchAndGet(TokenType::ID);
        if (hadError_) return;
        int line = tokens_[pos_ - 1].line;
        symbolTable_.addSymbol(name, type, currentScope(), line, SymbolCategory::PARAMETER);
        parseArgsTail();
    }

    // Args' -> ',' Args | lambda
    void parseArgsTail() {
        if (hadError_) return;
        if (currentType() == TokenType::COMMA) {
            match(TokenType::COMMA);
            if (hadError_) return;
            parseArgs();
        }
        // λ
    }

    // Cmd -> '{' CmdList '}'
    //       | 'if' '(' Exp ')' Cmd 'else' Cmd
    //       | 'while' '(' Exp ')' Cmd
    //       | 'System' '.' 'out' '.' 'println' '(' Exp ')' ';'
    //       | Id Cmd'
    // Usa o token atual (lookahead) para decidir qual alternativa seguir
    void parseCmd() {
        if (hadError_) return;
        if (currentType() == TokenType::LBRACE) {
            match(TokenType::LBRACE);
            if (hadError_) return;
            parseCmdList();
            if (hadError_) return;
            match(TokenType::RBRACE);
        } else if (currentType() == TokenType::IF) {
            match(TokenType::IF);
            if (hadError_) return;
            match(TokenType::LPAREN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RPAREN);
            if (hadError_) return;
            parseCmd();
            if (hadError_) return;
            match(TokenType::ELSE);
            if (hadError_) return;
            parseCmd();
        } else if (currentType() == TokenType::WHILE) {
            match(TokenType::WHILE);
            if (hadError_) return;
            match(TokenType::LPAREN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RPAREN);
            if (hadError_) return;
            parseCmd();
        } else if (currentType() == TokenType::SYSTEM) {
            match(TokenType::SYSTEM);
            if (hadError_) return;
            match(TokenType::DOT);
            if (hadError_) return;
            match(TokenType::OUT);
            if (hadError_) return;
            match(TokenType::DOT);
            if (hadError_) return;
            match(TokenType::PRINTLN);
            if (hadError_) return;
            match(TokenType::LPAREN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RPAREN);
            if (hadError_) return;
            match(TokenType::SEMICOLON);
        } else if (currentType() == TokenType::ID) {
            match(TokenType::ID);
            if (hadError_) return;
            parseCmdTail();
        } else {
            error("esperado um comando ('{', 'if', 'while', 'System' ou identificador)");
        }
    }

    // Cmd' -> '=' Exp ';'           (atribuicao simples)
    //        | '[' Exp ']' '=' Exp ';'  (atribuicao em array)
    void parseCmdTail() {
        if (hadError_) return;
        if (currentType() == TokenType::ASSIGN) {
            match(TokenType::ASSIGN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::SEMICOLON);
        } else if (currentType() == TokenType::LBRACKET) {
            match(TokenType::LBRACKET);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RBRACKET);
            if (hadError_) return;
            match(TokenType::ASSIGN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::SEMICOLON);
        } else {
            error("esperado '=' ou '[' após identificador em comando");
        }
    }

    // CmdList -> Cmd CmdList | lambda
    // Parseia zero ou mais comandos em sequencia
    void parseCmdList() {
        if (hadError_) return;
        if (isCmdStart()) {
            parseCmd();
            if (hadError_) return;
            parseCmdList();
        }
        // λ
    }

    // Exp -> PrimExp Exp'
    // Expressao: uma expressao primaria seguida de operacoes opcionais
    void parseExp() {
        if (hadError_) return;
        parsePrimExp();
        if (hadError_) return;
        parseExpTail();
    }

    // Exp' -> Op PrimExp Exp' | '[' Exp ']' Exp' | '.' PostExpDot | lambda
    // Cauda da expressao: operador binario, acesso a array, ou chamada de metodo
    void parseExpTail() {
        if (hadError_) return;
        if (isOp()) {
            parseOp();
            if (hadError_) return;
            parsePrimExp();
            if (hadError_) return;
            parseExpTail();
        } else if (currentType() == TokenType::LBRACKET) {
            match(TokenType::LBRACKET);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RBRACKET);
            if (hadError_) return;
            parseExpTail();
        } else if (currentType() == TokenType::DOT) {
            match(TokenType::DOT);
            if (hadError_) return;
            parsePostExpDot();
        }
        // λ
    }

    // Op -> '&&' | '<' | '>' | '+' | '-' | '*'
    void parseOp() {
        if (hadError_) return;
        if (currentType() == TokenType::AND || currentType() == TokenType::LT ||
            currentType() == TokenType::GT || currentType() == TokenType::PLUS ||
            currentType() == TokenType::MINUS || currentType() == TokenType::MULT) {
            advance();
        } else {
            error("esperado um operador (&&, <, >, +, -, *)");
        }
    }

    // PostExpDot -> 'length' Exp' | Id '(' ListExp ')' Exp'
    // Apos um '.': acesso a .length ou chamada de metodo
    void parsePostExpDot() {
        if (hadError_) return;
        if (currentType() == TokenType::LENGTH) {
            match(TokenType::LENGTH);
            if (hadError_) return;
            parseExpTail();
        } else if (currentType() == TokenType::ID) {
            match(TokenType::ID);
            if (hadError_) return;
            match(TokenType::LPAREN);
            if (hadError_) return;
            parseListExp();
            if (hadError_) return;
            match(TokenType::RPAREN);
            if (hadError_) return;
            parseExpTail();
        } else {
            error("esperado 'length' ou identificador de método após '.'");
        }
    }

    // ListExp -> Exp ListExp' | lambda
    // Lista de argumentos em chamada de metodo (pode ser vazia)
    void parseListExp() {
        if (hadError_) return;
        if (isPrimExpStart()) {
            parseExp();
            if (hadError_) return;
            parseListExpTail();
        }
        // λ
    }

    // ListExp' -> ',' Exp ListExp' | lambda
    void parseListExpTail() {
        if (hadError_) return;
        if (currentType() == TokenType::COMMA) {
            match(TokenType::COMMA);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            parseListExpTail();
        }
        // λ
    }

    // PrimExp -> 'new' PrimExp' | '!' Exp | '(' Exp ')'
    //           | 'true' | 'false' | Id | Number | 'this'
    // Expressao primaria: o "atomo" de uma expressao
    void parsePrimExp() {
        if (hadError_) return;
        if (currentType() == TokenType::NEW) {
            match(TokenType::NEW);
            if (hadError_) return;
            parsePrimExpTail();
        } else if (currentType() == TokenType::NOT) {
            match(TokenType::NOT);
            if (hadError_) return;
            parseExp();
        } else if (currentType() == TokenType::LPAREN) {
            match(TokenType::LPAREN);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RPAREN);
        } else if (currentType() == TokenType::TRUE) {
            match(TokenType::TRUE);
        } else if (currentType() == TokenType::FALSE) {
            match(TokenType::FALSE);
        } else if (currentType() == TokenType::ID) {
            match(TokenType::ID);
        } else if (currentType() == TokenType::NUMBER) {
            match(TokenType::NUMBER);
        } else if (currentType() == TokenType::THIS) {
            match(TokenType::THIS);
        } else {
            error("esperado uma expressão primária (identificador, número, 'true', 'false', 'this', 'new', '!' ou '(')");
        }
    }

    // PrimExp' -> Id '(' ')'        (instanciacao de objeto: new Classe())
    //            | 'int' '[' Exp ']'  (criacao de array: new int[size])
    void parsePrimExpTail() {
        if (hadError_) return;
        if (currentType() == TokenType::ID) {
            match(TokenType::ID);
            if (hadError_) return;
            match(TokenType::LPAREN);
            if (hadError_) return;
            match(TokenType::RPAREN);
        } else if (currentType() == TokenType::INT) {
            match(TokenType::INT);
            if (hadError_) return;
            match(TokenType::LBRACKET);
            if (hadError_) return;
            parseExp();
            if (hadError_) return;
            match(TokenType::RBRACKET);
        } else {
            error("esperado identificador ou 'int' após 'new'");
        }
    }

    // ==================== Conjuntos FIRST ====================
    // Funcoes que verificam se o token atual pertence ao conjunto FIRST
    // de uma producao. Usadas para decidir qual alternativa seguir.

    // FIRST(Type) = { int, boolean, ID }
    bool isTypeStart() const {
        return currentType() == TokenType::INT ||
               currentType() == TokenType::BOOLEAN ||
               currentType() == TokenType::ID;
    }

    // FIRST(Cmd) = { '{', if, while, System, ID }
    bool isCmdStart() const {
        return currentType() == TokenType::LBRACE ||
               currentType() == TokenType::IF ||
               currentType() == TokenType::WHILE ||
               currentType() == TokenType::SYSTEM ||
               currentType() == TokenType::ID;
    }

    // FIRST(Op) = { &&, <, >, +, -, * }
    bool isOp() const {
        return currentType() == TokenType::AND ||
               currentType() == TokenType::LT ||
               currentType() == TokenType::GT ||
               currentType() == TokenType::PLUS ||
               currentType() == TokenType::MINUS ||
               currentType() == TokenType::MULT;
    }

    // FIRST(PrimExp) = { new, !, (, true, false, ID, NUMBER, this }
    bool isPrimExpStart() const {
        return currentType() == TokenType::NEW ||
               currentType() == TokenType::NOT ||
               currentType() == TokenType::LPAREN ||
               currentType() == TokenType::TRUE ||
               currentType() == TokenType::FALSE ||
               currentType() == TokenType::ID ||
               currentType() == TokenType::NUMBER ||
               currentType() == TokenType::THIS;
    }
};

#endif // PARSER_H
