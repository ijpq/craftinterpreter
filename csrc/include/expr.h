#pragma once
#include "token.h"
#include <initializer_list>
#include <memory>
#include <variant>
namespace syntax {
class Binary;
class Grouping;
class Literal;
class Unary;
using Lexeme::Token;
class Expr;

struct Visitor {

  using ReturnType = std::string;

public:
  virtual ReturnType visitBinaryExpr(Binary *expr) = 0;
  virtual ReturnType visitGroupingExpr(Grouping *expr) = 0;
  virtual ReturnType visitLiteralExpr(Literal *expr) = 0;
  virtual ReturnType visitUnaryExpr(Unary *expr) = 0;
};

struct ASTPrinter : Visitor {
  // using raw ptr since we only take a look at this classes.
  std::string visitBinaryExpr(Binary *expr);
  std::string visitGroupingExpr(Grouping *expr);
  std::string visitLiteralExpr(Literal *expr);
  std::string visitUnaryExpr(Unary *expr);
  std::string prefix_expr(std::string name,
                          std::initializer_list<Expr *> exprs);
  std::string print(Expr *expr);
};
struct Expr {

  virtual std::string accept(Visitor *visitor) = 0;
};

struct Binary : Expr {

  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  std::string accept(Visitor *visitor) override {
    return visitor->visitBinaryExpr(this);
  }
};

struct Grouping : Expr {
  std::unique_ptr<Expr> expression;

  Grouping(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}
  Visitor::ReturnType accept(Visitor *visitor) override {
    return visitor->visitGroupingExpr(this);
  }
};

struct Literal : Expr {
  using LiteralValue = std::variant<double, std::string, bool, std::monostate>;
  LiteralValue literal;
  Literal(LiteralValue literal) : literal(literal) {}
  Literal(Lexeme::Literal lexemeliteral) {
    std::visit([&](auto&& args) {
      using T = std::decay_t<decltype(args)>;
      if constexpr (std::is_same_v<T, double> || std::is_same_v<T, std::monostate>) {
        literal = args;
      } else {
        literal = std::string(args);
      }
    }, lexemeliteral);
  }
  Visitor::ReturnType accept(Visitor *visitor) override {
    return visitor->visitLiteralExpr(this);
  }
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right)
      : op(op), right(std::move(right)) {}
  Visitor::ReturnType accept(Visitor *visitor) override {
    return visitor->visitUnaryExpr(this);
  }
  Token op;
  std::unique_ptr<Expr> right;
};
} // namespace syntax
