#pragma once
#include <initializer_list>
#include <memory>
#include <variant>

#include "helper/object.h"
#include "loxvalue.h"
#include "token.h"
namespace syntax {
class Binary;
class Grouping;
class Literal;
class Unary;
using Lexeme::Token;
class Expr;
class Variable;
class Assign;

struct Visitor {
  using ReturnType = LoxValueType;

 public:
  virtual ReturnType visitBinaryExpr(Binary* expr) = 0;
  virtual ReturnType visitGroupingExpr(Grouping* expr) = 0;
  virtual ReturnType visitLiteralExpr(Literal* expr) = 0;
  virtual ReturnType visitUnaryExpr(Unary* expr) = 0;
  virtual ReturnType visitVariableExpr(Variable* expr) = 0;
  virtual ReturnType visitAssignExpr(Assign* expr) = 0;

  ~Visitor() = default;
};

struct ASTPrinter : Visitor {
  // using raw ptr since we only take a look at this classes.
  ReturnType visitBinaryExpr(Binary* expr);
  ReturnType visitGroupingExpr(Grouping* expr);
  ReturnType visitLiteralExpr(Literal* expr);
  ReturnType visitUnaryExpr(Unary* expr);
  ReturnType prefix_expr(std::string name, std::initializer_list<Expr*> exprs);
  std::string print(Expr* expr);
  ReturnType visitVariableExpr(syntax::Variable* expr) {}
  ReturnType visitAssignExpr(syntax::Assign* expr) {}
};

struct Expr {
  using ReturnType = Visitor::ReturnType;
  virtual ReturnType accept(Visitor* visitor) = 0;
  ~Expr() = default;
};

struct Binary : Expr {
  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  ReturnType accept(Visitor* visitor) override {
    return visitor->visitBinaryExpr(this);
  }
};

struct Grouping : Expr {
  std::unique_ptr<Expr> expression;

  Grouping(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}
  ReturnType accept(Visitor* visitor) override {
    return visitor->visitGroupingExpr(this);
  }
};

struct Literal : Expr {
  using LiteralValue = std::variant<double, std::string, bool, std::monostate>;
  LiteralValue literal;
  Literal(LiteralValue literal) : literal(literal) {}
  Literal(Lexeme::Literal lexemeliteral) {
    std::visit(
        [&](auto&& args) {
          using T = std::decay_t<decltype(args)>;
          if constexpr (std::is_same_v<T, double> ||
                        std::is_same_v<T, std::monostate>) {
            literal = args;
          } else if constexpr (std::is_same_v<T, std::string_view>) {
            literal = std::string(args);
          } else {
            literal.emplace<bool>(args);
          }
        },
        lexemeliteral);
  }
  ReturnType accept(Visitor* visitor) override {
    return visitor->visitLiteralExpr(this);
  }
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right)
      : op(op), right(std::move(right)) {}
  ReturnType accept(Visitor* visitor) override {
    return visitor->visitUnaryExpr(this);
  }
  Token op;
  std::unique_ptr<Expr> right;
};

struct Variable : Expr {
  Variable(Token name) : name(name) {}
  ReturnType accept(Visitor* visitor) override {
    return visitor->visitVariableExpr(this);
  }
  Token name;
};

struct Assign : Expr {
  Assign(Token name, std::unique_ptr<Expr> value)
      : name(name), value(std::move(value)) {}
  ReturnType accept(Visitor* visitor) override {
    return visitor->visitAssignExpr(this);
  }
  Token name;
  std::unique_ptr<Expr> value;
};

std::string stringify(Visitor::ReturnType obj);

}  // namespace syntax
