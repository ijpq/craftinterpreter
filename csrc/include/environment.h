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
  std::unique_ptr<Environment> parent_env;

  Environment() { parent_env = nullptr; }

  Environment(std::unique_ptr<Environment> env) { parent_env = std::move(env); }

  void define(std::string name, LoxValueType value) {
    map.emplace(std::make_pair(name, value));
  }

  // in order to throw InterpreterRuntimeError which show line number
  LoxValueType get(const Lexeme::Token& name) {
    if (map.find(std::string{name.lexeme}) != map.end())
      return map[std::string{name.lexeme}];

    if (parent_env != nullptr) {
      return parent_env->get(name);
    }

    throw InterpreterRuntimeError(
        name, "Undefined variable '" + std::string{name.lexeme} + "'.");
  }

  void assign(const Lexeme::Token& name, LoxValueType value) {
    if (map.find(std::string{name.lexeme}) != map.end()) {
      map.emplace(std::make_pair(std::string{name.lexeme}, value));
      return;
    }
    if (parent_env != nullptr) {
      parent_env->assign(name, value);
      return;
    }

    throw InterpreterRuntimeError(
        name, "Undefined variable '" + std::string{name.lexeme} + "'.");
  }
};
}  // namespace interpreter
