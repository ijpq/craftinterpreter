#include "include/interpreter.h"

#include <ctime>

#include "include/expr.h"
#include "include/loxvalue.h"

namespace interpreter {
using syntax::stringify;
void Interpreter::interpret(syntax::Expr* expr) {
  LoxValueType v = evaluate(expr);
  std::cout << stringify(v) << std::endl;
}

void Interpreter::interpret(
    std::vector<std::unique_ptr<SST::Stmt>>& statements) {
  // doesnt requre ownership
  for (auto& statement : statements) execute(statement.get());
  // LoxValueType v = evaluate(expr);
  // std::cout << stringify(v) << std::endl;
}

// by wrapper class
Interpreter::Interpreter() {
  std::shared_ptr<LoxCallable> clock_func = std::make_shared<WrapperCallable>(
      [&](const std::vector<LoxValueType>& args) {
        return static_cast<double>(clock()) / CLOCKS_PER_SEC;
      },
      0, "<native fn>");
  env.define("clock", clock_func);
}

}  // namespace interpreter