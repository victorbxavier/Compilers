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
    Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0), hadError_(false) {}

    // Ponto de entrada: parseia o programa inteiro
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
    std::vector<Token> tokens_;
    int pos_;
    bool hadError_;
    SymbolTable symbolTable_;
    std::string currentClass_;   // Classe sendo parseada (escopo)
    std::string currentMethod_;  // Método sendo parseado (escopo)

    // ==================== Funções auxiliares ====================

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

    Token peek() const {
        return currentToken();
    }

    TokenType currentType() const {
        return currentToken().type;
    }

    void advance() {
        if (pos_ < (int)tokens_.size()) {
            pos_++;
        }
    }

    // Consome o token esperado ou reporta erro
    void match(TokenType expected) {
        if (currentType() == expected) {
            advance();
        } else {
            error(expected);
        }
    }

    // Consome e retorna o lexema do token
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

    // Retorna o escopo atual formatado
    std::string currentScope() const {
        if (currentMethod_.empty()) {
            return currentClass_;
        }
        return currentClass_ + "." + currentMethod_;
    }

    // Reporta erro sintático
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

    // Parseia um tipo e retorna sua representação como string
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

    // ==================== Regras da gramática ====================

    // Prog → MainC DefCl
    void parseProg() {
        parseMainC();
        if (hadError_) return;
        parseDefCl();
    }

    // MainC → 'class' Id '{' 'public' 'static' 'void' 'main' '(' 'String' '[' ']' Id ')' '{' CmdList '}' '}'
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

    // DefCl → 'class' Id DefCl' | λ
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

    // DefCl' → '{' DefVar DefMet '}' DefCl
    //         | 'extends' Id '{' DefVar DefMet '}' DefCl
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

    // DefVar → Type Id ';' DefVar | λ
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

    // DefMet → 'public' Type Id '(' DefMetArgs ')' '{' DefVar CmdList 'return' Exp ';' '}' DefMet | λ
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

    // DefMetArgs → Args | λ
    void parseDefMetArgs() {
        if (hadError_) return;
        if (isTypeStart()) {
            parseArgs();
        }
        // λ
    }

    // Type → 'int' '[' ']' | 'boolean' | 'int' | Id
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

    // Args → Type Id Args'
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

    // Args' → ',' Args | λ
    void parseArgsTail() {
        if (hadError_) return;
        if (currentType() == TokenType::COMMA) {
            match(TokenType::COMMA);
            if (hadError_) return;
            parseArgs();
        }
        // λ
    }

    // Cmd → '{' CmdList '}'
    //      | 'if' '(' Exp ')' Cmd 'else' Cmd
    //      | 'while' '(' Exp ')' Cmd
    //      | 'System' '.' 'out' '.' 'println' '(' Exp ')' ';'
    //      | Id Cmd'
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

    // Cmd' → '=' Exp ';'
    //       | '[' Exp ']' '=' Exp ';'
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

    // CmdList → Cmd CmdList | λ
    void parseCmdList() {
        if (hadError_) return;
        if (isCmdStart()) {
            parseCmd();
            if (hadError_) return;
            parseCmdList();
        }
        // λ
    }

    // Exp → PrimExp Exp'
    void parseExp() {
        if (hadError_) return;
        parsePrimExp();
        if (hadError_) return;
        parseExpTail();
    }

    // Exp' → Op PrimExp Exp'
    //       | '[' Exp ']' Exp'
    //       | '.' PostExpDot
    //       | λ
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

    // Op → '&&' | '<' | '>' | '+' | '-' | '*'
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

    // PostExpDot → 'length' Exp'
    //            | Id '(' ListExp ')' Exp'
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

    // ListExp → Exp ListExp' | λ
    void parseListExp() {
        if (hadError_) return;
        if (isPrimExpStart()) {
            parseExp();
            if (hadError_) return;
            parseListExpTail();
        }
        // λ
    }

    // ListExp' → ',' Exp ListExp' | λ
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

    // PrimExp → 'new' PrimExp'
    //         | '!' Exp
    //         | '(' Exp ')'
    //         | 'true'
    //         | 'false'
    //         | Id
    //         | Number
    //         | 'this'
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

    // PrimExp' → Id '(' ')'
    //          | 'int' '[' Exp ']'
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

    // ==================== Funções de verificação (FIRST sets) ====================

    bool isTypeStart() const {
        return currentType() == TokenType::INT ||
               currentType() == TokenType::BOOLEAN ||
               currentType() == TokenType::ID;
    }

    bool isCmdStart() const {
        return currentType() == TokenType::LBRACE ||
               currentType() == TokenType::IF ||
               currentType() == TokenType::WHILE ||
               currentType() == TokenType::SYSTEM ||
               currentType() == TokenType::ID;
    }

    bool isOp() const {
        return currentType() == TokenType::AND ||
               currentType() == TokenType::LT ||
               currentType() == TokenType::GT ||
               currentType() == TokenType::PLUS ||
               currentType() == TokenType::MINUS ||
               currentType() == TokenType::MULT;
    }

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
