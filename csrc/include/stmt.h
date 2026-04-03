#pragma once
#include <variant>

#include "expr.h"
#include "tokentype.h"
namespace SST {

// clang-format off
/*

program        → statement* EOF ;

declaration    → varDecl
               | statement ;
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
primary        → "true" | "false" | "nil"
               | NUMBER | STRING
               | "(" expression ")"
               | IDENTIFIER ;
statement      → exprStmt
       | printStmt ;

exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
*/
// clang-format on
class Expression;
class Print;
class Var;
using StmtVisitorType = void;

/*
hold AST as member var, provide accept() to interpreter
*/
struct Stmt {  // statement
  template <typename R>
  struct Visitor {
    // virtual R visitBlockStmt(Block* stmt) = 0;
    // virtual R visitClassStmt(Class* stmt) = 0;
    virtual R visitExpressionStmt(Expression* stmt) = 0;
    // virtual R visitFunctionStmt(Function* stmt) = 0;
    // virtual R visitIfStmt(If* stmt) = 0;
    virtual R visitPrintStmt(Print* stmt) = 0;
    // virtual R visitReturnStmt(Return* stmt) = 0;
    virtual R visitVarStmt(Var* stmt) = 0;
    // virtual R visitWhileStmt(While* stmt) = 0;
    ~Visitor() = default;
  };

  virtual void accept(Visitor<void>* visitor) = 0;
  ~Stmt() = default;
};

struct Expression : Stmt {                   // exprStmt
  std::unique_ptr<syntax::Expr> expression;  // have ownership

  Expression(std::unique_ptr<syntax::Expr> expression) {
    this->expression = std::move(expression);
  }

  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitExpressionStmt(this);
  }
};

struct Print : Stmt {                        // printStmt
  std::unique_ptr<syntax::Expr> expression;  // having ownership
  Print(std::unique_ptr<syntax::Expr> expression) {
    this->expression = std::move(expression);
  }

  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitPrintStmt(this);
  }
};

struct Var : Stmt {
  Lexeme::Token identifier;
  std::unique_ptr<syntax::Expr> init;
  // LoxValueType value;
  Var(Lexeme::Token name, std::unique_ptr<syntax::Expr> init)
      : init(std::move(init)), identifier(name) {}

  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitVarStmt(this);
  }
};
}  // namespace SST