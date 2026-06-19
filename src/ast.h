/**
 * @file ast.h
 * @brief Definição completa dos nós da Árvore Sintática Abstrata (AST).
 *
 * Hierarquia:
 *   ASTNode (base)
 *   ├── Program
 *   ├── MainClass
 *   ├── ClassDecl
 *   ├── MethodDecl
 *   ├── VarDecl
 *   ├── Type (IntType, BoolType, IntArrayType, IdentifierType)
 *   ├── Stmt (If, While, Print, Assign, ArrayAssign, Block)
 *   └── Exp  (And, LessThan, Plus, Minus, Times, Not, ArrayLookup,
 *             ArrayLength, MethodCall, IntLiteral, TrueLiteral,
 *             FalseLiteral, IdentifierExp, This, NewObject, NewArray)
 */

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// ============================================================
// Base
// ============================================================

struct ASTNode {
    int line = 0;  // Linha no código fonte (para mensagens de erro)
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
protected:
    void printIndent(int indent) const {
        for (int i = 0; i < indent; i++) std::cout << "  ";
    }
};

// ============================================================
// Tipos
// ============================================================

struct Type : ASTNode {};

struct IntType : Type {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "int" << std::endl;
    }
};

struct BoolType : Type {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "boolean" << std::endl;
    }
};

struct IntArrayType : Type {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "int[]" << std::endl;
    }
};

struct IdentifierType : Type {
    std::string name;
    explicit IdentifierType(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << name << std::endl;
    }
};

// Utilitário: retorna string representando o tipo
inline std::string typeToString(const Type* t) {
    if (dynamic_cast<const IntType*>(t)) return "int";
    if (dynamic_cast<const BoolType*>(t)) return "boolean";
    if (dynamic_cast<const IntArrayType*>(t)) return "int[]";
    if (auto* id = dynamic_cast<const IdentifierType*>(t)) return id->name;
    return "unknown";
}

// ============================================================
// Expressões
// ============================================================

struct Exp : ASTNode {};

struct AndExp : Exp {
    std::unique_ptr<Exp> left, right;
    AndExp(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "And" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
};

struct LessThanExp : Exp {
    std::unique_ptr<Exp> left, right;
    LessThanExp(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "LessThan" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
};

struct PlusExp : Exp {
    std::unique_ptr<Exp> left, right;
    PlusExp(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Plus" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
};

struct MinusExp : Exp {
    std::unique_ptr<Exp> left, right;
    MinusExp(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Minus" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
};

struct TimesExp : Exp {
    std::unique_ptr<Exp> left, right;
    TimesExp(std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : left(std::move(l)), right(std::move(r)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Times" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
};

struct NotExp : Exp {
    std::unique_ptr<Exp> expr;
    explicit NotExp(std::unique_ptr<Exp> e) : expr(std::move(e)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Not" << std::endl;
        expr->print(indent + 1);
    }
};

struct ArrayLookupExp : Exp {
    std::unique_ptr<Exp> array, index;
    ArrayLookupExp(std::unique_ptr<Exp> a, std::unique_ptr<Exp> i)
        : array(std::move(a)), index(std::move(i)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "ArrayLookup" << std::endl;
        array->print(indent + 1);
        index->print(indent + 1);
    }
};

struct ArrayLengthExp : Exp {
    std::unique_ptr<Exp> array;
    explicit ArrayLengthExp(std::unique_ptr<Exp> a) : array(std::move(a)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "ArrayLength" << std::endl;
        array->print(indent + 1);
    }
};

struct MethodCallExp : Exp {
    std::unique_ptr<Exp> object;
    std::string method;
    std::vector<std::unique_ptr<Exp>> args;
    MethodCallExp(std::unique_ptr<Exp> obj, const std::string& m,
                  std::vector<std::unique_ptr<Exp>> a)
        : object(std::move(obj)), method(m), args(std::move(a)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "MethodCall: " << method << std::endl;
        object->print(indent + 1);
        for (auto& arg : args) arg->print(indent + 1);
    }
};

struct IntLiteralExp : Exp {
    int value;
    explicit IntLiteralExp(int v) : value(v) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "IntLiteral: " << value << std::endl;
    }
};

struct TrueLiteralExp : Exp {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "true" << std::endl;
    }
};

struct FalseLiteralExp : Exp {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "false" << std::endl;
    }
};

struct IdentifierExp : Exp {
    std::string name;
    explicit IdentifierExp(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Id: " << name << std::endl;
    }
};

struct ThisExp : Exp {
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "this" << std::endl;
    }
};

struct NewObjectExp : Exp {
    std::string className;
    explicit NewObjectExp(const std::string& c) : className(c) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "NewObject: " << className << std::endl;
    }
};

struct NewArrayExp : Exp {
    std::unique_ptr<Exp> size;
    explicit NewArrayExp(std::unique_ptr<Exp> s) : size(std::move(s)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "NewArray" << std::endl;
        size->print(indent + 1);
    }
};

// ============================================================
// Comandos (Statements)
// ============================================================

struct Stmt : ASTNode {};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> s) : stmts(std::move(s)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Block" << std::endl;
        for (auto& s : stmts) s->print(indent + 1);
    }
};

struct IfStmt : Stmt {
    std::unique_ptr<Exp> condition;
    std::vector<std::unique_ptr<Stmt>> thenBody;
    std::vector<std::unique_ptr<Stmt>> elseBody;  // Pode ser vazio (else opcional)
    IfStmt(std::unique_ptr<Exp> c, std::vector<std::unique_ptr<Stmt>> t,
           std::vector<std::unique_ptr<Stmt>> e)
        : condition(std::move(c)), thenBody(std::move(t)), elseBody(std::move(e)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "If" << std::endl;
        printIndent(indent + 1); std::cout << "Condition:" << std::endl;
        condition->print(indent + 2);
        printIndent(indent + 1); std::cout << "Then:" << std::endl;
        for (auto& s : thenBody) s->print(indent + 2);
        if (!elseBody.empty()) {
            printIndent(indent + 1); std::cout << "Else:" << std::endl;
            for (auto& s : elseBody) s->print(indent + 2);
        }
    }
};

struct WhileStmt : Stmt {
    std::unique_ptr<Exp> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    WhileStmt(std::unique_ptr<Exp> c, std::vector<std::unique_ptr<Stmt>> b)
        : condition(std::move(c)), body(std::move(b)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "While" << std::endl;
        printIndent(indent + 1); std::cout << "Condition:" << std::endl;
        condition->print(indent + 2);
        printIndent(indent + 1); std::cout << "Body:" << std::endl;
        for (auto& s : body) s->print(indent + 2);
    }
};

struct PrintStmt : Stmt {
    std::unique_ptr<Exp> expression;
    explicit PrintStmt(std::unique_ptr<Exp> e) : expression(std::move(e)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Print" << std::endl;
        expression->print(indent + 1);
    }
};

struct AssignStmt : Stmt {
    std::string variable;
    std::unique_ptr<Exp> value;
    AssignStmt(const std::string& v, std::unique_ptr<Exp> val)
        : variable(v), value(std::move(val)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Assign: " << variable << std::endl;
        value->print(indent + 1);
    }
};

struct ArrayAssignStmt : Stmt {
    std::string array;
    std::unique_ptr<Exp> index;
    std::unique_ptr<Exp> value;
    ArrayAssignStmt(const std::string& a, std::unique_ptr<Exp> i, std::unique_ptr<Exp> v)
        : array(a), index(std::move(i)), value(std::move(v)) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "ArrayAssign: " << array << std::endl;
        printIndent(indent + 1); std::cout << "Index:" << std::endl;
        index->print(indent + 2);
        printIndent(indent + 1); std::cout << "Value:" << std::endl;
        value->print(indent + 2);
    }
};

// ============================================================
// Declarações de variáveis
// ============================================================

struct VarDecl : ASTNode {
    std::unique_ptr<Type> type;
    std::string name;
    VarDecl(std::unique_ptr<Type> t, const std::string& n)
        : type(std::move(t)), name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "VarDecl: " << typeToString(type.get())
                                       << " " << name << std::endl;
    }
};

// ============================================================
// Parâmetro formal (tipo + nome)
// ============================================================

struct Formal : ASTNode {
    std::unique_ptr<Type> type;
    std::string name;
    Formal(std::unique_ptr<Type> t, const std::string& n)
        : type(std::move(t)), name(n) {}
    void print(int indent = 0) const override {
        printIndent(indent); std::cout << typeToString(type.get())
                                       << " " << name << std::endl;
    }
};

// ============================================================
// Métodos
// ============================================================

struct MethodDecl : ASTNode {
    std::unique_ptr<Type> returnType;
    std::string name;
    std::vector<std::unique_ptr<Formal>> params;
    std::vector<std::unique_ptr<VarDecl>> locals;
    std::vector<std::unique_ptr<Stmt>> body;
    std::unique_ptr<Exp> returnExp;

    MethodDecl(std::unique_ptr<Type> rt, const std::string& n,
               std::vector<std::unique_ptr<Formal>> p,
               std::vector<std::unique_ptr<VarDecl>> l,
               std::vector<std::unique_ptr<Stmt>> b,
               std::unique_ptr<Exp> re)
        : returnType(std::move(rt)), name(n), params(std::move(p)),
          locals(std::move(l)), body(std::move(b)), returnExp(std::move(re)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "MethodDecl: " << typeToString(returnType.get())
                  << " " << name << "(";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << typeToString(params[i]->type.get()) << " " << params[i]->name;
        }
        std::cout << ")" << std::endl;
        for (auto& v : locals) v->print(indent + 1);
        for (auto& s : body) s->print(indent + 1);
        printIndent(indent + 1); std::cout << "Return:" << std::endl;
        returnExp->print(indent + 2);
    }
};

// ============================================================
// Classes
// ============================================================

struct ClassDecl : ASTNode {
    std::string name;
    std::string parent;  // Vazio se não usa extends
    std::vector<std::unique_ptr<VarDecl>> vars;
    std::vector<std::unique_ptr<MethodDecl>> methods;

    ClassDecl(const std::string& n, const std::string& p,
              std::vector<std::unique_ptr<VarDecl>> v,
              std::vector<std::unique_ptr<MethodDecl>> m)
        : name(n), parent(p), vars(std::move(v)), methods(std::move(m)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ClassDecl: " << name;
        if (!parent.empty()) std::cout << " extends " << parent;
        std::cout << std::endl;
        for (auto& v : vars) v->print(indent + 1);
        for (auto& m : methods) m->print(indent + 1);
    }
};

// ============================================================
// Classe Principal (main)
// ============================================================

struct MainClass : ASTNode {
    std::string name;
    std::string argsName;  // Nome do parâmetro String[] do main
    std::vector<std::unique_ptr<Stmt>> body;

    MainClass(const std::string& n, const std::string& a,
              std::vector<std::unique_ptr<Stmt>> b)
        : name(n), argsName(a), body(std::move(b)) {}

    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "MainClass: " << name << std::endl;
        for (auto& s : body) s->print(indent + 1);
    }
};

// ============================================================
// Programa (raiz da AST)
// ============================================================

struct Program : ASTNode {
    std::unique_ptr<MainClass> mainClass;
    std::vector<std::unique_ptr<ClassDecl>> classes;

    Program(std::unique_ptr<MainClass> mc, std::vector<std::unique_ptr<ClassDecl>> cls)
        : mainClass(std::move(mc)), classes(std::move(cls)) {}

    void print(int indent = 0) const override {
        printIndent(indent); std::cout << "Program" << std::endl;
        mainClass->print(indent + 1);
        for (auto& c : classes) c->print(indent + 1);
    }
};

#endif // AST_H
