#pragma once
#include <ctime>
#include <functional>
#include <vector>

#include "environment.h"
#include "loxvalue.h"
#include "stmt.h"
namespace interpreter {
class Interpreter;
}

struct LoxCallable {
  virtual size_t arity() const = 0;
  virtual LoxValueType call(interpreter::Interpreter* interpreter,
                            const std::vector<LoxValueType>& args) = 0;
  virtual std::string toString() = 0;
};

struct ClockCallable : LoxCallable {
  size_t arity() const override { return 0; }

  LoxValueType call(interpreter::Interpreter* interpreter,
                    const std::vector<LoxValueType>& args) override {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
  }

  std::string toString() override { return "<native fn>"; }
};

struct WrapperCallable : LoxCallable {
  std::function<LoxValueType(const std::vector<LoxValueType>&)> callable;
  size_t mem_arity;
  std::string mem_string;

  size_t arity() const override { return mem_arity; }

  std::string toString() override { return mem_string; }

  template <typename F>
  WrapperCallable(F&& f, size_t arity, std::string str)
      : callable(f), mem_arity(arity), mem_string(str) {}

  LoxValueType call(interpreter::Interpreter* interpreter,
                    const std::vector<LoxValueType>& args) override {
    return callable(args);
  }
};

struct LoxFunction : LoxCallable {
  SST::Function* declaration;
  std::shared_ptr<interpreter::Environment>
      declaration_env;  // a environment when function is declared, in order to
                        // capture this env.

  LoxFunction(SST::Function* declaration,
              std::shared_ptr<interpreter::Environment> env)
      : declaration(declaration), declaration_env(env) {}

  LoxValueType call(interpreter::Interpreter* interpreter,
                    const std::vector<LoxValueType>& args) override;

  size_t arity() const override { return declaration->params.size(); }

  std::string toString() override {
    return "<fn " + std::string{declaration->name.lexeme} + ">";
  }
};
