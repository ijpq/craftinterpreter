#include "include/expr.h"

#include <cmath>
#include <cstdio>
#include <variant>

#include "include/loxfunction.h"
#include "include/token.h"

namespace syntax {

std::string stringify(Visitor::ReturnType obj) {
  if (obj.hold_alternative<std::monostate>()) return "nil";
  if (obj.hold_alternative<bool>()) return obj.get<bool>() ? "true" : "false";
  if (obj.hold_alternative<std::string>()) return obj.get<std::string>();
  if (obj.hold_alternative<double>()) {
    double d = obj.get<double>();
    if (std::floor(d) == d && std::isfinite(d))
      return std::to_string((long long)d);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.14g", d);
    return buf;
  }
  if (obj.hold_alternative<std::shared_ptr<LoxCallable>>())
    return obj.get<std::shared_ptr<LoxCallable>>()->toString();
  return "";
}

Visitor::ReturnType ASTPrinter::prefix_expr(
    std::string name, std::initializer_list<Expr*> exprs) {
  std::string res;
  res += "(" + name;
  for (auto&& expr : exprs) res += " " + stringify(expr->accept(this));

  res += ")";
  return res;
}

Visitor::ReturnType ASTPrinter::visitBinaryExpr(Binary* expr) {
  return prefix_expr(std::string{expr->op.lexeme},
                     {expr->left.get(), expr->right.get()});
}

Visitor::ReturnType ASTPrinter::visitGroupingExpr(Grouping* expr) {
  return prefix_expr("group", {expr->expression.get()});
}

std::string literalval_to_str(Literal* literal) {
  return std::visit(
      [](auto&& args) -> std::string {
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

Visitor::ReturnType ASTPrinter::visitLiteralExpr(Literal* expr) {
  return literalval_to_str(expr);
}

Visitor::ReturnType ASTPrinter::visitUnaryExpr(Unary* expr) {
  return prefix_expr(std::string{expr->op.lexeme}, {expr->right.get()});
}

std::string ASTPrinter::print(Expr* expr) {
  return stringify(expr->accept(this));
}

}  // namespace syntax
