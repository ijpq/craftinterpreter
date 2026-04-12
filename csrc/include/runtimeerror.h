#pragma once
#include <stdexcept>

#include "loxvalue.h"
#include "token.h"
#include "tokentype.h"
namespace interpreter {
struct InterpreterRuntimeError : std::runtime_error {
  Lexeme::Token token;
  InterpreterRuntimeError(const Lexeme::Token& token, std::string msg)
      : std::runtime_error(msg), token(token) {}

  std::string getMessage() { return this->what(); }
};

struct Return : std::runtime_error {
  Lexeme::Token token;
  LoxValueType value;
  Return(const Lexeme::Token& token, LoxValueType value, std::string msg)
      : value(value), std::runtime_error(msg), token(token) {}
  std::string getMessage() { return this->what(); }
};
}  // namespace interpreter