#include "include/expr.h"
#include "include/token.h"
#include "include/tokentype.h"
#include <variant>

namespace syntax {

std::string ASTPrinter::prefix_expr(std::string name,
                                    std::initializer_list<Expr *> exprs) {
  std::string res;
  res += "(" + name;
  for (auto &&expr : exprs)
    res += " " + expr->accept(this);

  res += ")";
  return res;
}

Visitor::ReturnType ASTPrinter::visitBinaryExpr(Binary *expr) {
  return prefix_expr(std::string{expr->op.lexeme},
                     {expr->left.get(), expr->right.get()});
}

Visitor::ReturnType ASTPrinter::visitGroupingExpr(Grouping *expr) {
  return prefix_expr("group", {expr->expression.get()});
}

std::string literalval_to_str(Literal *literal) {
  return std::visit(
      [](auto &&args) -> std::string {
        using T = std::decay_t<decltype(args)>;
        if constexpr (std::is_same_v<T, bool>) {
          return args ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          return "nil";
        } else if constexpr (std::is_same_v<T, std::string>) {
          return args;
        } else if constexpr (std::is_same_v<T, double>) {
          return Lexeme::double_to_string(args);
        }
      },
      literal->literal);
}

Visitor::ReturnType ASTPrinter::visitLiteralExpr(Literal *expr) {
  return literalval_to_str(expr);
}

Visitor::ReturnType ASTPrinter::visitUnaryExpr(Unary *expr) {
  return prefix_expr(std::string{expr->op.lexeme}, {expr->right.get()});
}

std::string ASTPrinter::print(Expr *expr) { return expr->accept(this); }

} // namespace syntax
