#pragma once
#include <cmath>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <variant>

#include "expr.h"
#include "helper/object.h"
#include "runtimeerror.h"
#include "token.h"
#include "tokentype.h"
namespace interpreter {

using LoxValueType = syntax::Visitor::ReturnType;

struct Interpreter : syntax::Visitor {
  /*
  interpreter class act as visitor to AST, and compute each tree's value
  */
  void interpret(syntax::Expr* expr);

  LoxValueType visitLiteralExpr(syntax::Literal* expr) override {
    return LoxValueType(expr->literal);
  }

  LoxValueType visitGroupingExpr(syntax::Grouping* expr) override {
    return evaluate(expr->expression.get());
  }

  LoxValueType evaluate(syntax::Expr* expr) { return expr->accept(this); }

  LoxValueType visitUnaryExpr(syntax::Unary* expr) override {
    auto right = evaluate(expr->right.get());
    switch (expr->op.type) {
      case Lexeme::TokenType::MINUS:
        return -1 * right.get<double>();
      case Lexeme::TokenType::BANG:
        return !isTruthy(right);
      default:
        throw std::exception();
    }
    return std::monostate{};
  }
  bool isEqual(LoxValueType& left, LoxValueType& right) {
    return left == right ? true : false;
  }

  void checkNumberOperand(Lexeme::Token op, LoxValueType operand) {
    if (operand.hold_alternative<double>()) return;
    throw InterpreterRuntimeError(op, "Operands must be numbers");
  }
  void checkNumberOperand(Lexeme::Token op, LoxValueType left,
                          LoxValueType right) {
    if (left.hold_alternative<double>() && right.hold_alternative<double>())
      return;
    throw InterpreterRuntimeError(op, "Operands must be numbers");
  }
  LoxValueType visitBinaryExpr(syntax::Binary* expr) {
    LoxValueType left = evaluate(expr->left.get());
    LoxValueType right = evaluate(expr->right.get());
    switch (expr->op.type) {
      case Lexeme::TokenType::MINUS:
        checkNumberOperand(expr->op, right);
        return left.get<double>() - right.get<double>();
      case Lexeme::TokenType::SLASH:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() / right.get<double>();
      case Lexeme::TokenType::STAR:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() * right.get<double>();
      case Lexeme::TokenType::PLUS:
        if (left.hold_alternative<double>() &&
            right.hold_alternative<double>()) {
          return left.get<double>() + right.get<double>();
        }
        if (left.hold_alternative<std::string>() &&
            right.hold_alternative<std::string>()) {
          return left.get<std::string>() + right.get<std::string>();
        }
        throw InterpreterRuntimeError(
            expr->op, "Operands must be two numbers or two strings");
        break;
      case Lexeme::TokenType::GREATER:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() > right.get<double>();
      case Lexeme::TokenType::GREATER_EQUAL:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() >= right.get<double>();
      case Lexeme::TokenType::LESS:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() < right.get<double>();
      case Lexeme::TokenType::LESS_EQUAL:
        checkNumberOperand(expr->op, left, right);
        return left.get<double>() <= right.get<double>();
      case Lexeme::TokenType::BANG_EQUAL:
        return !isEqual(left, right);
      case Lexeme::TokenType::EQUAL_EQUAL:
        return isEqual(left, right);
    }
    return std::monostate{};
  }

  bool isTruthy(LoxValueType object) {
    if (object.get<std::monostate>() == std::monostate{}) return false;
    return object.get<bool>();
  }
};

}  // namespace interpreter
