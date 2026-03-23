#include "token.h"
#include <initializer_list>
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
  std::string visitBinaryExpr(Binary *expr);
  std::string visitGroupingExpr(Grouping *expr);
  std::string visitLiteralExpr(Literal *expr);
  std::string visitUnaryExpr(Unary *expr);
  std::string prefix_expr(std::string name,
                          std::initializer_list<Expr *> exprs);
 std::string print(Expr* expr);
};
struct Expr {

  virtual std::string accept(Visitor *visitor) = 0;
};

struct Binary : Expr {

  Expr *left;
  Token op;
  Expr *right;
  Binary(Expr *left, Token op, Expr *right)
      : left(left), op(op), right(right) {}

  std::string accept(Visitor *visitor) override {
    return visitor->visitBinaryExpr(this);
  }
};

struct Grouping : Expr {
  Expr *expression;

  Grouping(Expr *expression) : expression(expression) {}
  Visitor::ReturnType accept(Visitor *visitor) override {
    return visitor->visitGroupingExpr(this);
  }
};

struct Literal : Expr {
  Lexeme::Literal literal;
  Literal(Lexeme::Literal literal) : literal(literal) {}
  Visitor::ReturnType accept(Visitor *visitor) override {
    return visitor->visitLiteralExpr(this);
  }
};

struct Unary : Expr {
	Unary(Token op, Expr* right) :
	op(op), right(right) {}
	Visitor::ReturnType accept(Visitor* visitor) override {
		return visitor->visitUnaryExpr(this);
	}
	Token op;
	Expr* right;
};
} // namespace syntax
