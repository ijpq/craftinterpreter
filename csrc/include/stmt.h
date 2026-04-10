#pragma once

#include "expr.h"
#include "token.h"
namespace SST {
using syntax::Expr;
// clang-format off
/*

program        → statement* EOF ;

declaration    → funDecl
               | varDecl
               | statement ;
funDecl        → "fun" function ;
function       → IDENTIFIER "(" parameters? ")" block ;
parameters     → IDENTIFIER ( "," IDENTIFIER )* ;
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
primary        → "true" | "false" | "nil"
               | NUMBER | STRING
               | "(" expression ")"
               | IDENTIFIER ;
statement      → exprStmt | forStmt | ifStmt| printStmt | whileStmt | block ;
forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
                 expression? ";"
                 expression? ")" statement ;

whileStmt      → "while" "(" expression ")" statement ;
ifStmt         → "if" "(" expression ")" statement
               ( "else" statement )? ;
block          → "{" declaration* "}" ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
*/
// clang-format on
class Expression;
class Print;
class Var;
using StmtVisitorType = void;
class Block;
class If;
class While;
class Function;

/*
hold AST as member var, provide accept() to interpreter
*/
struct Stmt {  // statement
  template <typename R>
  struct Visitor {
    virtual R visitBlockStmt(Block* stmt) = 0;
    // virtual R visitClassStmt(Class* stmt) = 0;
    virtual R visitExpressionStmt(Expression* stmt) = 0;
    virtual R visitFunctionStmt(Function* stmt) = 0;
    virtual R visitIfStmt(If* stmt) = 0;
    virtual R visitPrintStmt(Print* stmt) = 0;
    // virtual R visitReturnStmt(Return* stmt) = 0;
    virtual R visitVarStmt(Var* stmt) = 0;
    virtual R visitWhileStmt(While* stmt) = 0;
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

struct Block : Stmt {
  Block(std::vector<std::unique_ptr<Stmt>>&& statements)
      : statements(std::move(statements)) {}
  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitBlockStmt(this);
  }
  std::vector<std::unique_ptr<Stmt>> statements;
};

struct If : Stmt {
  If(std::unique_ptr<syntax::Expr> condition, std::unique_ptr<SST::Stmt> then,
     std::unique_ptr<SST::Stmt> elsebranch)
      : condition(std::move(condition)),
        then(std::move(then)),
        elsebranch(std::move(elsebranch)) {}
  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitIfStmt(this);
  }
  std::unique_ptr<syntax::Expr> condition;
  std::unique_ptr<SST::Stmt> then;
  std::unique_ptr<SST::Stmt> elsebranch;
};

struct While : Stmt {
  While(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
      : body(std::move(body)), condition(std::move(condition)) {}

  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitWhileStmt(this);
  }
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;
};

// created when function was declared.
struct Function : Stmt {
  Function(Lexeme::Token name, std::vector<Lexeme::Token> params,
           std::vector<std::unique_ptr<Stmt>> body)
      : name(name), params(params), body(std::move(body)) {}
  StmtVisitorType accept(Stmt::Visitor<StmtVisitorType>* visitor) override {
    return visitor->visitFunctionStmt(this);
  }
  Lexeme::Token name;
  std::vector<Lexeme::Token> params;
  std::vector<std::unique_ptr<Stmt>> body;
};
}  // namespace SST