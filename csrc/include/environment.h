#pragma once
#include <stdexcept>
#include <unordered_map>

#include "loxvalue.h"
#include "runtimeerror.h"
#include "token.h"
namespace interpreter {

struct Environment {
  using Key = std::string;
  using Value = LoxValueType;
  using value_type = std::pair<Key, Value>;
  std::unordered_map<std::string, LoxValueType> map;
  void define(std::string name, LoxValueType value) {
    map.emplace(std::make_pair(name, value));
  }

  // in order to throw InterpreterRuntimeError which show line number
  LoxValueType get(const Lexeme::Token& name) {
    try {
      return map.at(std::string{name.lexeme});
    } catch (std::out_of_range& e) {
      throw InterpreterRuntimeError(
          name, "Undefined variable '" + std::string{name.lexeme} + "'.");
    }
  }

  void assign(const Lexeme::Token& name, LoxValueType value) {
    if (map.find(std::string{name.lexeme}) != map.end()) {
      map.emplace(std::make_pair(std::string{name.lexeme}, value));
    }

    throw InterpreterRuntimeError(
        name, "Undefined variable '" + std::string{name.lexeme} + "'.");
  }
};
}  // namespace interpreter
