/**
 * @file parser.h
 * @brief Analisador Sintático — Parser Recursivo Descendente (Unidade 2).
 *
 * Implementa a gramática LL(1) do professor (valdigleis.site) com:
 *   - Hierarquia de precedência de operadores (Andexp > Relexp > Addexp > Mulexp > Unexp > Psfexp > Priexp)
 *   - Construção da AST durante o parsing
 *   - Preenchimento da tabela de símbolos
 */

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens, bool stopOnFirstError = false)
        : tokens_(tokens), pos_(0), hadError_(false), errorCount_(0),
          stopOnFirstError_(stopOnFirstError) {}

    /**
     * @brief Executa o parsing e retorna a AST (ou nullptr se falhou).
     */
    std::unique_ptr<Program> parse() {
        auto prog = parseProg();
        if (!hadError_) match(TokenType::END_OF_FILE);
        if (hadError_) return nullptr;
        return prog;
    }

    bool hadError() const { return hadError_; }
    int getErrorCount() const { return errorCount_; }
    const SymbolTable& getSymbolTable() const { return symbolTable_; }

private:
    std::vector<Token> tokens_;
    int pos_;
    bool hadError_;
    int errorCount_;
    bool stopOnFirstError_;
    SymbolTable symbolTable_;
    std::string currentClass_;
    std::string currentMethod_;

    // ==================== Funções auxiliares ====================

    Token currentToken() const {
        if (pos_ < (int)tokens_.size()) return tokens_[pos_];
        Token eof; eof.type = TokenType::END_OF_FILE; eof.lexeme = "$";
        eof.line = tokens_.back().line; eof.column = 0;
        return eof;
    }

    Token peek() const { return currentToken(); }
    TokenType currentType() const { return currentToken().type; }

    void advance() {
        if (pos_ < (int)tokens_.size()) pos_++;
    }

    void match(TokenType expected) {
        if (hadError_) return;
        if (currentType() == expected) { advance(); }
        else { error(expected); }
    }

    std::string matchAndGet(TokenType expected) {
        if (hadError_) return "";
        if (currentType() == expected) {
            std::string lex = currentToken().lexeme;
            advance(); return lex;
        }
        error(expected); return "";
    }

    int currentLine() const { return currentToken().line; }

    std::string currentScope() const {
        if (currentMethod_.empty()) return currentClass_;
        return currentClass_ + "." + currentMethod_;
    }

    void error(TokenType expected) {
        if (hadError_ && stopOnFirstError_) return;
        hadError_ = true;
        errorCount_++;
        Token tok = currentToken();
        std::cerr << "Erro sintático na linha " << tok.line << ":" << tok.column
                  << ": esperado " << tokenTypeToString(expected)
                  << " mas encontrou " << tokenTypeToString(tok.type);
        if (tok.type == TokenType::ID || tok.type == TokenType::NUMBER)
            std::cerr << " (\"" << tok.lexeme << "\")";
        std::cerr << std::endl;
    }

    void error(const std::string& msg) {
        if (hadError_ && stopOnFirstError_) return;
        hadError_ = true;
        errorCount_++;
        Token tok = currentToken();
        std::cerr << "Erro sintático na linha " << tok.line << ":" << tok.column
                  << ": " << msg;
        std::cerr << " (encontrou " << tokenTypeToString(tok.type);
        if (tok.type == TokenType::ID || tok.type == TokenType::NUMBER)
            std::cerr << " \"" << tok.lexeme << "\"";
        std::cerr << ")" << std::endl;
    }

    // ==================== Parsing de tipos ====================

    std::unique_ptr<Type> parseType() {
        if (hadError_) return nullptr;
        if (currentType() == TokenType::INT) {
            match(TokenType::INT);
            if (hadError_) return nullptr;
            if (currentType() == TokenType::LBRACKET) {
                match(TokenType::LBRACKET);
                if (hadError_) return nullptr;
                match(TokenType::RBRACKET);
                return std::make_unique<IntArrayType>();
            }
            return std::make_unique<IntType>();
        } else if (currentType() == TokenType::BOOLEAN) {
            match(TokenType::BOOLEAN);
            return std::make_unique<BoolType>();
        } else if (currentType() == TokenType::ID) {
            std::string name = matchAndGet(TokenType::ID);
            return std::make_unique<IdentifierType>(name);
        }
        error("esperado um tipo (int, boolean, int[] ou identificador)");
        return nullptr;
    }

    std::string parseTypeString() {
        if (hadError_) return "";
        auto t = parseType();
        if (!t) return "";
        return typeToString(t.get());
    }

    // ==================== Prog → MainC DefCl ====================

    std::unique_ptr<Program> parseProg() {
        auto mc = parseMainC();
        if (hadError_) return nullptr;
        auto classes = parseDefCl();
        if (hadError_) return nullptr;
        return std::make_unique<Program>(std::move(mc), std::move(classes));
    }

    // MainC → 'class' Id '{' 'public' 'static' 'void' 'main' '(' 'String' '[' ']' Id ')' '{' Lcom '}' '}'
    std::unique_ptr<MainClass> parseMainC() {
        match(TokenType::CLASS); if (hadError_) return nullptr;
        std::string className = matchAndGet(TokenType::ID); if (hadError_) return nullptr;
        int classLine = tokens_[pos_ - 1].line;
        currentClass_ = className;
        symbolTable_.addSymbol(className, "class", "global", classLine, SymbolCategory::CLASS);

        match(TokenType::LBRACE); if (hadError_) return nullptr;
        match(TokenType::PUBLIC); if (hadError_) return nullptr;
        match(TokenType::STATIC); if (hadError_) return nullptr;
        match(TokenType::VOID); if (hadError_) return nullptr;
        match(TokenType::MAIN); if (hadError_) return nullptr;
        currentMethod_ = "main";
        symbolTable_.addSymbol("main", "void", currentClass_, tokens_[pos_-1].line, SymbolCategory::METHOD);

        match(TokenType::LPAREN); if (hadError_) return nullptr;
        match(TokenType::STRING); if (hadError_) return nullptr;
        match(TokenType::LBRACKET); if (hadError_) return nullptr;
        match(TokenType::RBRACKET); if (hadError_) return nullptr;
        std::string argsName = matchAndGet(TokenType::ID); if (hadError_) return nullptr;
        symbolTable_.addSymbol(argsName, "String[]", currentScope(), tokens_[pos_-1].line, SymbolCategory::PARAMETER);
        match(TokenType::RPAREN); if (hadError_) return nullptr;
        match(TokenType::LBRACE); if (hadError_) return nullptr;

        auto body = parseLcom();
        if (hadError_) return nullptr;
        match(TokenType::RBRACE); if (hadError_) return nullptr;
        match(TokenType::RBRACE); if (hadError_) return nullptr;
        currentMethod_ = "";
        currentClass_ = "";
        return std::make_unique<MainClass>(className, argsName, std::move(body));
    }

    // DefCl → 'class' Id DefCl' | λ
    std::vector<std::unique_ptr<ClassDecl>> parseDefCl() {
        std::vector<std::unique_ptr<ClassDecl>> classes;
        while (!hadError_ && currentType() == TokenType::CLASS) {
            match(TokenType::CLASS); if (hadError_) break;
            std::string className = matchAndGet(TokenType::ID); if (hadError_) break;
            int classLine = tokens_[pos_ - 1].line;
            currentClass_ = className;

            std::string parent = "";
            if (currentType() == TokenType::EXTENDS) {
                match(TokenType::EXTENDS); if (hadError_) break;
                parent = matchAndGet(TokenType::ID); if (hadError_) break;
            }

            symbolTable_.addSymbol(className, "class", "global", classLine, SymbolCategory::CLASS, parent);

            match(TokenType::LBRACE); if (hadError_) break;
            auto vars = parseDefV(SymbolCategory::INSTANCE_VAR);
            if (hadError_) break;
            auto methods = parseDefM();
            if (hadError_) break;
            match(TokenType::RBRACE); if (hadError_) break;

            classes.push_back(std::make_unique<ClassDecl>(className, parent, std::move(vars), std::move(methods)));
            currentClass_ = "";
        }
        return classes;
    }

    // ==================== DefV → Type Id ';' DefV | λ ====================

    std::vector<std::unique_ptr<VarDecl>> parseDefV(SymbolCategory varCat) {
        std::vector<std::unique_ptr<VarDecl>> vars;
        while (!hadError_ && isTypeStart()) {
            // Lookahead: se é Type Id ';' então é declaração de variável
            int savedPos = pos_;
            bool isVarDecl = false;

            if (currentType() == TokenType::INT) {
                advance();
                if (currentType() == TokenType::LBRACKET) {
                    advance();
                    if (currentType() == TokenType::RBRACKET) {
                        advance();
                        if (currentType() == TokenType::ID) { advance(); isVarDecl = (currentType() == TokenType::SEMICOLON); }
                    }
                } else if (currentType() == TokenType::ID) {
                    advance(); isVarDecl = (currentType() == TokenType::SEMICOLON);
                }
            } else if (currentType() == TokenType::BOOLEAN) {
                advance();
                if (currentType() == TokenType::ID) { advance(); isVarDecl = (currentType() == TokenType::SEMICOLON); }
            } else if (currentType() == TokenType::ID) {
                advance();
                if (currentType() == TokenType::ID) { advance(); isVarDecl = (currentType() == TokenType::SEMICOLON); }
            }

            pos_ = savedPos;
            if (!isVarDecl) break;

            auto type = parseType(); if (hadError_) break;
            std::string typeName = typeToString(type.get());
            std::string name = matchAndGet(TokenType::ID); if (hadError_) break;
            int ln = tokens_[pos_ - 1].line;
            symbolTable_.addSymbol(name, typeName, currentScope(), ln, varCat);
            match(TokenType::SEMICOLON); if (hadError_) break;
            vars.push_back(std::make_unique<VarDecl>(std::move(type), name));
        }
        return vars;
    }

    // ==================== DefM → 'public' Type Id '(' DefM' | λ ====================

    std::vector<std::unique_ptr<MethodDecl>> parseDefM() {
        std::vector<std::unique_ptr<MethodDecl>> methods;
        while (!hadError_ && currentType() == TokenType::PUBLIC) {
            match(TokenType::PUBLIC); if (hadError_) break;
            auto retType = parseType(); if (hadError_) break;
            std::string retTypeName = typeToString(retType.get());
            std::string methodName = matchAndGet(TokenType::ID); if (hadError_) break;
            int methodLine = tokens_[pos_ - 1].line;
            currentMethod_ = methodName;
            symbolTable_.addSymbol(methodName, retTypeName, currentClass_, methodLine, SymbolCategory::METHOD);
            match(TokenType::LPAREN); if (hadError_) break;

            // DefM' → Args ')' { DefV Lcom return Exp ; } DefM | ')' { ... }
            std::vector<std::unique_ptr<Formal>> params;
            if (currentType() != TokenType::RPAREN) {
                params = parseArgs();
                if (hadError_) break;
            }
            match(TokenType::RPAREN); if (hadError_) break;
            match(TokenType::LBRACE); if (hadError_) break;
            auto locals = parseDefV(SymbolCategory::LOCAL_VAR);
            if (hadError_) break;
            auto body = parseLcom();
            if (hadError_) break;
            match(TokenType::RETURN); if (hadError_) break;
            auto retExp = parseExp();
            if (hadError_) break;
            match(TokenType::SEMICOLON); if (hadError_) break;
            match(TokenType::RBRACE); if (hadError_) break;

            methods.push_back(std::make_unique<MethodDecl>(
                std::move(retType), methodName, std::move(params),
                std::move(locals), std::move(body), std::move(retExp)));
            currentMethod_ = "";
        }
        return methods;
    }

    // ==================== Args → Type Id Args' ====================

    std::vector<std::unique_ptr<Formal>> parseArgs() {
        std::vector<std::unique_ptr<Formal>> params;
        if (hadError_) return params;

        auto type = parseType(); if (hadError_) return params;
        std::string typeName = typeToString(type.get());
        std::string name = matchAndGet(TokenType::ID); if (hadError_) return params;
        int ln = tokens_[pos_ - 1].line;
        symbolTable_.addSymbol(name, typeName, currentScope(), ln, SymbolCategory::PARAMETER);
        params.push_back(std::make_unique<Formal>(std::move(type), name));

        // Args' → ',' Type Id Args' | λ
        while (!hadError_ && currentType() == TokenType::COMMA) {
            match(TokenType::COMMA); if (hadError_) break;
            auto t2 = parseType(); if (hadError_) break;
            std::string t2Name = typeToString(t2.get());
            std::string n2 = matchAndGet(TokenType::ID); if (hadError_) break;
            int ln2 = tokens_[pos_ - 1].line;
            symbolTable_.addSymbol(n2, t2Name, currentScope(), ln2, SymbolCategory::PARAMETER);
            params.push_back(std::make_unique<Formal>(std::move(t2), n2));
        }
        return params;
    }

    // ==================== Lcom → Com Lcom' ====================
    // Lcom' → Com Lcom' | λ

    std::vector<std::unique_ptr<Stmt>> parseLcom() {
        std::vector<std::unique_ptr<Stmt>> stmts;
        if (hadError_) return stmts;
        if (!isCmdStart()) return stmts;  // Lcom pode ser vazio se não há comandos

        auto first = parseCom();
        if (hadError_) return stmts;
        if (first) stmts.push_back(std::move(first));

        // Lcom'
        while (!hadError_ && isCmdStart()) {
            auto s = parseCom();
            if (hadError_) break;
            if (s) stmts.push_back(std::move(s));
        }
        return stmts;
    }

    // ==================== Com ====================
    // Com → Id ComAss | if ( Exp ) { Lcom } I | while ( Exp ) { Lcom } | System.out.println( Exp ) ;

    std::unique_ptr<Stmt> parseCom() {
        if (hadError_) return nullptr;

        if (currentType() == TokenType::ID) {
            std::string varName = matchAndGet(TokenType::ID); if (hadError_) return nullptr;
            int ln = tokens_[pos_ - 1].line;
            return parseComAss(varName, ln);
        }
        else if (currentType() == TokenType::IF) {
            match(TokenType::IF); if (hadError_) return nullptr;
            match(TokenType::LPAREN); if (hadError_) return nullptr;
            auto cond = parseExp(); if (hadError_) return nullptr;
            match(TokenType::RPAREN); if (hadError_) return nullptr;
            match(TokenType::LBRACE); if (hadError_) return nullptr;
            auto thenBody = parseLcom(); if (hadError_) return nullptr;
            match(TokenType::RBRACE); if (hadError_) return nullptr;

            // I → else { Lcom } | λ
            std::vector<std::unique_ptr<Stmt>> elseBody;
            if (currentType() == TokenType::ELSE) {
                match(TokenType::ELSE); if (hadError_) return nullptr;
                match(TokenType::LBRACE); if (hadError_) return nullptr;
                elseBody = parseLcom(); if (hadError_) return nullptr;
                match(TokenType::RBRACE); if (hadError_) return nullptr;
            }
            return std::make_unique<IfStmt>(std::move(cond), std::move(thenBody), std::move(elseBody));
        }
        else if (currentType() == TokenType::WHILE) {
            match(TokenType::WHILE); if (hadError_) return nullptr;
            match(TokenType::LPAREN); if (hadError_) return nullptr;
            auto cond = parseExp(); if (hadError_) return nullptr;
            match(TokenType::RPAREN); if (hadError_) return nullptr;
            match(TokenType::LBRACE); if (hadError_) return nullptr;
            auto body = parseLcom(); if (hadError_) return nullptr;
            match(TokenType::RBRACE); if (hadError_) return nullptr;
            return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
        }

        else if (currentType() == TokenType::SYSTEM) {
            match(TokenType::SYSTEM); if (hadError_) return nullptr;
            match(TokenType::DOT); if (hadError_) return nullptr;
            match(TokenType::OUT); if (hadError_) return nullptr;
            match(TokenType::DOT); if (hadError_) return nullptr;
            match(TokenType::PRINTLN); if (hadError_) return nullptr;
            match(TokenType::LPAREN); if (hadError_) return nullptr;
            auto exp = parseExp(); if (hadError_) return nullptr;
            match(TokenType::RPAREN); if (hadError_) return nullptr;
            match(TokenType::SEMICOLON); if (hadError_) return nullptr;
            return std::make_unique<PrintStmt>(std::move(exp));
        }
        else {
            error("esperado um comando (identificador, 'if', 'while' ou 'System')");
            return nullptr;
        }
    }

    // ComAss → '=' Exp ';' | '[' Exp ']' '=' Exp ';'
    std::unique_ptr<Stmt> parseComAss(const std::string& varName, int ln) {
        if (hadError_) return nullptr;
        if (currentType() == TokenType::ASSIGN) {
            match(TokenType::ASSIGN); if (hadError_) return nullptr;
            auto exp = parseExp(); if (hadError_) return nullptr;
            match(TokenType::SEMICOLON); if (hadError_) return nullptr;
            auto stmt = std::make_unique<AssignStmt>(varName, std::move(exp));
            stmt->line = ln;
            return stmt;
        } else if (currentType() == TokenType::LBRACKET) {
            match(TokenType::LBRACKET); if (hadError_) return nullptr;
            auto idx = parseExp(); if (hadError_) return nullptr;
            match(TokenType::RBRACKET); if (hadError_) return nullptr;
            match(TokenType::ASSIGN); if (hadError_) return nullptr;
            auto val = parseExp(); if (hadError_) return nullptr;
            match(TokenType::SEMICOLON); if (hadError_) return nullptr;
            auto stmt = std::make_unique<ArrayAssignStmt>(varName, std::move(idx), std::move(val));
            stmt->line = ln;
            return stmt;
        }
        error("esperado '=' ou '[' após identificador em comando");
        return nullptr;
    }

    // ==================== Expressões (hierarquia de precedência) ====================

    // Exp → Andexp
    std::unique_ptr<Exp> parseExp() {
        if (hadError_) return nullptr;
        return parseAndexp();
    }

    // Andexp → Relexp Andexp'
    // Andexp' → '&&' Relexp Andexp' | λ
    std::unique_ptr<Exp> parseAndexp() {
        if (hadError_) return nullptr;
        auto left = parseRelexp(); if (hadError_) return nullptr;
        while (currentType() == TokenType::AND) {
            match(TokenType::AND); if (hadError_) return nullptr;
            auto right = parseRelexp(); if (hadError_) return nullptr;
            left = std::make_unique<AndExp>(std::move(left), std::move(right));
        }
        return left;
    }

    // Relexp → Addexp Relexp'
    // Relexp' → '<' Addexp Relexp' | λ
    std::unique_ptr<Exp> parseRelexp() {
        if (hadError_) return nullptr;
        auto left = parseAddexp(); if (hadError_) return nullptr;
        while (currentType() == TokenType::LT) {
            match(TokenType::LT); if (hadError_) return nullptr;
            auto right = parseAddexp(); if (hadError_) return nullptr;
            left = std::make_unique<LessThanExp>(std::move(left), std::move(right));
        }
        return left;
    }

    // Addexp → Mulexp Addexp'
    // Addexp' → '+' Mulexp Addexp' | '-' Mulexp Addexp' | λ
    std::unique_ptr<Exp> parseAddexp() {
        if (hadError_) return nullptr;
        auto left = parseMulexp(); if (hadError_) return nullptr;
        while (currentType() == TokenType::PLUS || currentType() == TokenType::MINUS) {
            if (currentType() == TokenType::PLUS) {
                match(TokenType::PLUS); if (hadError_) return nullptr;
                auto right = parseMulexp(); if (hadError_) return nullptr;
                left = std::make_unique<PlusExp>(std::move(left), std::move(right));
            } else {
                match(TokenType::MINUS); if (hadError_) return nullptr;
                auto right = parseMulexp(); if (hadError_) return nullptr;
                left = std::make_unique<MinusExp>(std::move(left), std::move(right));
            }
        }
        return left;
    }

    // Mulexp → Unexp Mulexp'
    // Mulexp' → '*' Unexp Mulexp' | λ
    std::unique_ptr<Exp> parseMulexp() {
        if (hadError_) return nullptr;
        auto left = parseUnexp(); if (hadError_) return nullptr;
        while (currentType() == TokenType::MULT) {
            match(TokenType::MULT); if (hadError_) return nullptr;
            auto right = parseUnexp(); if (hadError_) return nullptr;
            left = std::make_unique<TimesExp>(std::move(left), std::move(right));
        }
        return left;
    }

    // Unexp → '!' Unexp | Psfexp
    std::unique_ptr<Exp> parseUnexp() {
        if (hadError_) return nullptr;
        if (currentType() == TokenType::NOT) {
            match(TokenType::NOT); if (hadError_) return nullptr;
            auto inner = parseUnexp(); if (hadError_) return nullptr;
            return std::make_unique<NotExp>(std::move(inner));
        }
        return parsePsfexp();
    }

    // Psfexp → Priexp Psfexp'
    // Psfexp' → '[' Exp ']' Psfexp' | '.' 'length' Psfexp' | '.' Id '(' Lexp ')' Psfexp' | λ
    std::unique_ptr<Exp> parsePsfexp() {
        if (hadError_) return nullptr;
        auto expr = parsePriexp(); if (hadError_) return nullptr;

        while (!hadError_) {
            if (currentType() == TokenType::LBRACKET) {
                match(TokenType::LBRACKET); if (hadError_) return nullptr;
                auto idx = parseExp(); if (hadError_) return nullptr;
                match(TokenType::RBRACKET); if (hadError_) return nullptr;
                expr = std::make_unique<ArrayLookupExp>(std::move(expr), std::move(idx));
            }
            else if (currentType() == TokenType::DOT) {
                match(TokenType::DOT); if (hadError_) return nullptr;
                if (currentType() == TokenType::LENGTH) {
                    match(TokenType::LENGTH); if (hadError_) return nullptr;
                    expr = std::make_unique<ArrayLengthExp>(std::move(expr));
                } else if (currentType() == TokenType::ID) {
                    std::string method = matchAndGet(TokenType::ID); if (hadError_) return nullptr;
                    match(TokenType::LPAREN); if (hadError_) return nullptr;
                    auto args = parseLexp(); if (hadError_) return nullptr;
                    match(TokenType::RPAREN); if (hadError_) return nullptr;
                    expr = std::make_unique<MethodCallExp>(std::move(expr), method, std::move(args));
                } else {
                    error("esperado 'length' ou identificador de método após '.'");
                    return nullptr;
                }
            }
            else { break; }
        }
        return expr;
    }

    // Priexp → '(' Exp ')' | true | false | Id | Number | this | new Id '(' ')' | new int '[' Exp ']'
    std::unique_ptr<Exp> parsePriexp() {
        if (hadError_) return nullptr;

        if (currentType() == TokenType::LPAREN) {
            match(TokenType::LPAREN); if (hadError_) return nullptr;
            auto e = parseExp(); if (hadError_) return nullptr;
            match(TokenType::RPAREN); if (hadError_) return nullptr;
            return e;
        }
        else if (currentType() == TokenType::TRUE) {
            match(TokenType::TRUE);
            return std::make_unique<TrueLiteralExp>();
        }
        else if (currentType() == TokenType::FALSE) {
            match(TokenType::FALSE);
            return std::make_unique<FalseLiteralExp>();
        }
        else if (currentType() == TokenType::ID) {
            std::string name = matchAndGet(TokenType::ID);
            auto exp = std::make_unique<IdentifierExp>(name);
            exp->line = tokens_[pos_ - 1].line;
            return exp;
        }
        else if (currentType() == TokenType::NUMBER) {
            int val = std::stoi(currentToken().lexeme);
            match(TokenType::NUMBER);
            return std::make_unique<IntLiteralExp>(val);
        }
        else if (currentType() == TokenType::THIS) {
            match(TokenType::THIS);
            return std::make_unique<ThisExp>();
        }
        else if (currentType() == TokenType::NEW) {
            match(TokenType::NEW); if (hadError_) return nullptr;
            if (currentType() == TokenType::INT) {
                match(TokenType::INT); if (hadError_) return nullptr;
                match(TokenType::LBRACKET); if (hadError_) return nullptr;
                auto size = parseExp(); if (hadError_) return nullptr;
                match(TokenType::RBRACKET); if (hadError_) return nullptr;
                return std::make_unique<NewArrayExp>(std::move(size));
            } else if (currentType() == TokenType::ID) {
                std::string className = matchAndGet(TokenType::ID); if (hadError_) return nullptr;
                match(TokenType::LPAREN); if (hadError_) return nullptr;
                match(TokenType::RPAREN); if (hadError_) return nullptr;
                return std::make_unique<NewObjectExp>(className);
            }
            error("esperado identificador ou 'int' após 'new'");
            return nullptr;
        }
        error("esperado expressão primária");
        return nullptr;
    }

    // Lexp → Exp Lexp' | λ
    // Lexp' → ',' Exp Lexp' | λ
    std::vector<std::unique_ptr<Exp>> parseLexp() {
        std::vector<std::unique_ptr<Exp>> args;
        if (hadError_) return args;
        if (!isPrimExpStart()) return args;  // λ

        auto first = parseExp(); if (hadError_) return args;
        args.push_back(std::move(first));

        while (!hadError_ && currentType() == TokenType::COMMA) {
            match(TokenType::COMMA); if (hadError_) break;
            auto e = parseExp(); if (hadError_) break;
            args.push_back(std::move(e));
        }
        return args;
    }

    // ==================== Conjuntos FIRST ====================

    bool isTypeStart() const {
        return currentType() == TokenType::INT ||
               currentType() == TokenType::BOOLEAN ||
               currentType() == TokenType::ID;
    }

    bool isCmdStart() const {
        return currentType() == TokenType::ID ||
               currentType() == TokenType::IF ||
               currentType() == TokenType::WHILE ||
               currentType() == TokenType::SYSTEM;
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
