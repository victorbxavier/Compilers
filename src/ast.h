#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>


// Forward declarations
class Exp;
class ExpBinary;
class ExpNumber;
class ExpIdentifier;
class ExpTrue;
class ExpFalse;
class ExpThis;
class ExpNewObject;
class ExpNewArray;
class ExpNot;
class ExpParen;

// Base class for all expressions
// Every expression node will inherit from this
class Exp {
    public:
        virtual ~Exp() = default; 

};

// Binary operation: left op right
// Examples: 3 + 5, x < y, a && b
class ExpBinary : public Exp {
    public:
        TokenType op;
        std::unique_ptr<Exp> left;
        std::unique_ptr<Exp> right;

        ExpBinary(TokenType op, std::unique_ptr<Exp> left; std::unique_ptr<Exp> right;) 
            : op(op), left(std::move(left)), right(std::move(right)) {}
};

// Integer literal
class ExpNumber : public Exp {
    public:
        int value;
        explicit ExpNumber(int value) : value(value) {}
};

// Variable reference
class ExpIdentifier : public Exp{
    public:
        std::string name;
        explicit ExpIdentifier(const std::string& name) : name(name) {}

};

// Boolean literals
class ExpTrue : public Exp {};
class ExpFalse : public Exp {};

// This
class ExpThis : public Exp {};

// Object instantiation
class ExpNewObject : public Exp{
    public:
        std::string className;
        explicit ExpNewObject(const std::string& className) : className(className) {}
};

// Array instantiation
class ExpNewArray : public Exp{
    public:
        std::unique_ptr<Exp> size ;
        explicit ExpNewObject(std::unique_ptr<Exp>& size) : size(std::move(size)) {}
};

// Not
class ExpNot : public Exp{
    public:
        std::unique_ptr<Exp> expr ;
        explicit ExpNot(std::unique_ptr<Exp>& expr) : expr(std::move(expr)) {}
};

// Parenthesized expression
class ExpParen : public Exp {
public:
    std::unique_ptr<Exp> expr;
    explicit ExpParen(std::unique_ptr<Exp> expr) : expr(std::move(expr)) {}
};

#endif



