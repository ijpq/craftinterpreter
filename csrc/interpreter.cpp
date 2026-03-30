#include "include/interpreter.h"

#include "include/lox.h"
namespace interpreter {

void Interpreter::interpret(syntax::Expr* expr) {
  try {
    LoxValueType v = evaluate(expr);
    std::cout << std::endl;
  } catch (InterpreterRuntimeError& e) {
    Lexeme::Lox::runtimeError(e);
  }
}
}  // namespace interpreter