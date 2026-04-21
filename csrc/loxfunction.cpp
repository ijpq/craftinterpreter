#include "include/loxfunction.h"

#include <functional>
#include <variant>

#include "include/environment.h"
#include "include/expr.h"
#include "include/interpreter.h"
#include "include/loxvalue.h"
#include "include/runtimeerror.h"
LoxValueType LoxFunction::call(interpreter::Interpreter* interpreter,
                               const std::vector<LoxValueType>& args) {
  // create a new env with declaration_env as parent
  auto new_env =
      std::make_shared<interpreter::Environment>(this->declaration_env);

  // bind arguments to parameters
  auto size = args.size();
  for (decltype(size) i = 0; i < size; i++) {
    new_env->define(std::string{this->declaration->params[i].lexeme}, args[i]);
  }

  // execute function body
  interpreter->executeBlock(declaration->body, new_env);
  return std::monostate{};
}
