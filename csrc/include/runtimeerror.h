#pragma once
#include <stdexcept>

#include "token.h"
namespace interpreter {
struct InterpreterRuntimeError : std::runtime_error {
  Lexeme::Token token;
  InterpreterRuntimeError(Lexeme::Token& token, std::string msg)
      : std::runtime_error(msg), token(token) {}

  std::string getMessage() { return this->what(); }
};
}  // namespace interpreter