#include <deque>
#include <iterator>
#include <unordered_map>
#include <variant>

#include "FunctionTypeEnum.h"
#include "expr.h"
#include "interpreter.h"
#include "lox.h"
#include "stmt.h"
#include "token.h"
struct Resolver : syntax::Visitor, SST::Stmt::Visitor<SST::StmtVisitorType> {
  interpreter::Interpreter* interpreter;
  std::deque<std::unordered_map<std::string, bool>> scopes;
  FunctionType currentFunction = FunctionType::NONE;

  Resolver(interpreter::Interpreter* interpreter) {
    this->interpreter = interpreter;
  }

  SST::StmtVisitorType visitBlockStmt(SST::Block* stmt) override {
    beginscope();
    resolve(stmt->statements);
    endscope();
    return;
  }

  SST::StmtVisitorType visitClassStmt(SST::Class* stmt) override {
    declare(stmt->name);
    define(stmt->name);
  }

  SST::StmtVisitorType visitFunctionStmtHelper(SST::Function* stmt,
                                               FunctionType type) {
    FunctionType enclosingFunction = currentFunction;
    currentFunction = type;
    beginscope();
    for (auto&& p : stmt->params) {
      declare(p);
      define(p);
    }
    resolve(stmt->body);

    endscope();
    currentFunction = enclosingFunction;
  }

  SST::StmtVisitorType visitFunctionStmt(SST::Function* stmt) override {
    declare(stmt->name);  // declare function name in current scope
    define(stmt->name);

    visitFunctionStmtHelper(stmt, FunctionType::FUNCTION);
  }

  SST::StmtVisitorType visitExpressionStmt(SST::Expression* expr) override {
    resolve(expr->expression.get());
  }

  SST::StmtVisitorType visitIfStmt(SST::If* expr) override {
    resolve(expr->condition.get());
    resolve(expr->then.get());
    if (expr->elsebranch) resolve(expr->elsebranch.get());
  }

  SST::StmtVisitorType visitPrintStmt(SST::Print* expr) override {
    resolve(expr->expression.get());
  }

  SST::StmtVisitorType visitWhileStmt(SST::While* stmt) override {
    resolve(stmt->condition.get());
    resolve(stmt->body.get());
  }

  syntax::Visitor::ReturnType visitBinaryExpr(syntax::Binary* expr) override {
    resolve(expr->left.get());
    resolve(expr->right.get());
    return std::monostate{};
  }

  syntax::Visitor::ReturnType visitCallExpr(syntax::Call* expr) override {
    resolve(expr->callee.get());
    for (auto&& arg : expr->arguments) {
      resolve(arg.get());
    }
    return std::monostate{};
  }

  SST::StmtVisitorType visitReturnStmt(SST::Return* expr) override {
    if (currentFunction == FunctionType::NONE) {
      Lexeme::Lox::error(expr->keyword, "Can't return from top-level code.");
    }
    if (expr->value) resolve(expr->value.get());
  }

  void beginscope() {
    scopes.push_front(std::unordered_map<std::string, bool>{});
  }

  void resolve(const std::vector<std::unique_ptr<SST::Stmt>>& statements) {
    for (auto&& statement : statements) {
      resolve(statement.get());
    }
  }

  syntax::Visitor::ReturnType visitGroupingExpr(
      syntax::Grouping* expr) override {
    resolve(expr->expression.get());
    return std::monostate{};
  }

  syntax::Visitor::ReturnType visitLiteralExpr(syntax::Literal* expr) override {
    return std::monostate{};
  }

  syntax::Visitor::ReturnType visitLogicalExpr(syntax::Logical* expr) override {
    resolve(expr->left.get());
    resolve(expr->right.get());
    return std::monostate{};
  }

  syntax::Visitor::ReturnType visitUnaryExpr(syntax::Unary* expr) override {
    resolve(expr->right.get());
    return std::monostate{};
  }

  void resolve(SST::Stmt* stmt) { stmt->accept(this); }

  void resolve(syntax::Expr* expr) { expr->accept(this); }

  void endscope() { scopes.pop_front(); }

  void visitVarStmt(SST::Var* stmt) override {
    declare(stmt->identifier);
    if (stmt->init != nullptr) {
      resolve(stmt->init.get());
    }
    define(stmt->identifier);
  }

  void declare(Lexeme::Token name) {
    if (scopes.empty()) return;
    auto&& scope = scopes.front();
    if (scope.find(std::string{name.lexeme}) != scope.end()) {
      Lexeme::Lox::error(name,
                         "Already variable with this name in this scope.");
    }
    scopes.front()[std::string{name.lexeme}] = false;
  }

  void define(Lexeme::Token name) {
    if (scopes.empty()) return;
    scopes.front()[std::string{name.lexeme}] = true;
  }

  void resolvelocal(syntax::Expr* expr, Lexeme::Token token) {
    for (size_t i = 0; i < scopes.size(); i++) {
      if (scopes[i].find(std::string{token.lexeme}) != scopes[i].end()) {
        interpreter->resolve(expr, static_cast<int>(i));
        return;
      }
    }
  }

  syntax::Visitor::ReturnType visitVariableExpr(
      syntax::Variable* expr) override {
    // if a var hadn't ready , put in other words, is false, but is used in
    // initializer.
    if (!scopes.empty()) {
      auto it = scopes.front().find(std::string{expr->name.lexeme});
      if (it != scopes.front().end() && it->second == false) {
        Lexeme::Lox::error(expr->name,
                           "Can't read local variable in its own initializer.");
      }
    }

    resolvelocal(expr, expr->name);

    return std::monostate{};  // to align with Visitor's return type. return
                              // dummy
  }

  syntax::Visitor::ReturnType visitAssignExpr(syntax::Assign* expr) override {
    resolve(expr->value.get());
    resolvelocal(expr, expr->name);
    return std::monostate{};
  }
};