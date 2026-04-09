#pragma once
#include <exception>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "environment.h"
#include "expr.h"
#include "helper/guard.h"
#include "helper/object.h"
#include "loxfunction.h"
#include "loxvalue.h"
#include "runtimeerror.h"
#include "stmt.h"
#include "token.h"
#include "tokentype.h"
namespace interpreter {

struct Interpreter : syntax::Visitor, SST::Stmt::Visitor<SST::StmtVisitorType> {
  /*
  interpreter class act as visitor to AST, and compute each tree's value
  */
  Environment env;
  Environment* current_env = &env;

  // class ClockCallable : LoxCallable {
  //   template <typename T>
  //   ClockCallable(T t) : callable(t) {}
  //   std::function<LoxValueType(const std::vector<LoxValueType>&)> callable;

  //   LoxValueType call(interpreter::Interpreter* interpreter,
  //                     const std::vector<LoxValueType>& args) override {
  //     callable(args);
  //   }
  // };
  Interpreter();

  void interpret(syntax::Expr* expr);

  void interpret(std::vector<std::unique_ptr<SST::Stmt>>& statements);

  LoxValueType visitLiteralExpr(syntax::Literal* expr) override {
    return LoxValueType(expr->literal);
  }

  LoxValueType visitGroupingExpr(syntax::Grouping* expr) override {
    return evaluate(expr->expression.get());
  }

  LoxValueType visitLogicalExpr(syntax::Logical* expr) override {
    auto left_val = evaluate(expr->left.get());
    if (expr->op.type == Lexeme::TokenType::AND) {
      if (!isTruthy(left_val)) return left_val;
    } else if (expr->op.type == Lexeme::TokenType::OR) {
      if (isTruthy(left_val)) return left_val;
    }

    return evaluate(expr->right.get());
  }

  // evaluate is for expression, create value
  LoxValueType evaluate(syntax::Expr* expr) { return expr->accept(this); }

  // execute is for statement, create side-effect
  void execute(SST::Stmt* stmt) { stmt->accept(this); }

  void executeBlock(std::vector<std::unique_ptr<SST::Stmt>>&& statements) {
    Environment previous = std::move(this->env);  // save previous scope
    this->env.parent_env = &previous;
    ScopeGuard g([&]() { this->env = std::move(previous); });  // restore
    for (auto&& stmt : statements) execute(stmt.get());
  }

  LoxValueType visitAssignExpr(syntax::Assign* expr) override {
    LoxValueType value = evaluate(expr->value.get());
    env.assign(expr->name, value);
    return value;
  }

  /*
  f(args1...)(args2...)...
  = Call(f, ')', args1...)(args2...)...
  = Call(Call(f, ')', args1...), ')', ...)...
  */
  LoxValueType visitCallExpr(syntax::Call* expr) override {
    auto func = evaluate(expr->callee.get());

    std::vector<LoxValueType> args;
    for (auto&& expr : expr->arguments) {
      args.push_back(evaluate(expr.get()));
    }
    if (!func.hold_alternative<Function*>()) {
      throw InterpreterRuntimeError(expr->paren,
                                    "can only call function and class.");
    }
    Function* p = func.get<Function*>();

    if (args.size() != p->arity()) {
      throw InterpreterRuntimeError(
          expr->paren, "Expected " + std::to_string(p->arity()) +
                           " arguments but got" + std::to_string(args.size()) +
                           ".");
    }
    return p->call(this, args);
  }

  LoxValueType visitUnaryExpr(syntax::Unary* expr) override {
    auto right = evaluate(expr->right.get());
    switch (expr->op.type) {
      case Lexeme::TokenType::MINUS:
        checkNumberOperand(expr->op, right);
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

  LoxValueType visitBinaryExpr(syntax::Binary* expr) override {
    LoxValueType left = evaluate(expr->left.get());
    LoxValueType right = evaluate(expr->right.get());
    switch (expr->op.type) {
      case Lexeme::TokenType::MINUS:
        checkNumberOperand(expr->op, left, right);
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
    if (object.hold_alternative<std::monostate>()) return false;
    if (object.hold_alternative<bool>()) return object.get<bool>();
    return true;
  }

  LoxValueType visitVariableExpr(syntax::Variable* expr) override {
    return env.get(expr->name);
  }
  /*
grab statement , call accept() of AST inside the statement. Since param visitor
is interpreter, it defined methods that calculate value from AST
  */
  SST::StmtVisitorType visitExpressionStmt(SST::Expression* stmt) override {
    evaluate(stmt->expression.get());  // discard returned value
    return;
  }

  SST::StmtVisitorType visitPrintStmt(SST::Print* stmt) override {
    LoxValueType value = evaluate(stmt->expression.get());
    std::cout << syntax::stringify(value) << std::endl;
  }

  SST::StmtVisitorType visitVarStmt(SST::Var* stmt) override {
    LoxValueType value = stmt->init ? evaluate(stmt->init.get())
                                    : LoxValueType(std::monostate{});
    // bind name and value
    env.define(Environment::Key(stmt->identifier.lexeme), value);
  }

  SST::StmtVisitorType visitBlockStmt(SST::Block* stmt) override {
    executeBlock(std::move(stmt->statements));
  }

  SST::StmtVisitorType visitIfStmt(SST::If* stmt) override {
    auto cond_val = evaluate(stmt->condition.get());
    if (isTruthy(cond_val))
      execute(stmt->then.get());
    else if (stmt->elsebranch != nullptr)
      execute(stmt->elsebranch.get());
  }

  SST::StmtVisitorType visitWhileStmt(SST::While* stmt) override {
    while (isTruthy(evaluate(stmt->condition.get()))) {
      execute(stmt->body.get());
    }
  }
};

}  // namespace interpreter
