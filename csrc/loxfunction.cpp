#include "include/loxfunction.h"

#include <functional>
#include <variant>

#include "include/interpreter.h"
#include "include/loxvalue.h"
#include "include/runtimeerror.h"
LoxValueType LoxFunction::call(interpreter::Interpreter* interpreter,
                               const std::vector<LoxValueType>& args) {
  // create a env that binds arguments into parameters.
  interpreter::Environment new_env(
      this->declaration_env);  // parent env points to a env when function was
                               // being declaring.

  // sanity check for arguments and parameters
  if (args.size() != this->declaration->params.size()) {
    throw interpreter::InterpreterRuntimeError(this->declaration->name, "!");
  }

  // bind
  auto size = args.size();
  for (auto i = 0; i < size; i++) {
    new_env.define(std::string{this->declaration->params[i].lexeme}, args[i]);
  }

  // execute function body
  interpreter->executeBlock(declaration->body, &new_env);
  return LoxValueType{std::monostate{}};
}
