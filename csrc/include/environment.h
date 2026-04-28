#pragma once
#include <memory>
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
  std::shared_ptr<Environment> parent_env = nullptr;

  Environment() = default;  // used for global scope
  Environment(std::shared_ptr<Environment> parent_env)
      : parent_env(parent_env) {}

  // init value
  void define(std::string name, LoxValueType value) {
    map.emplace(std::make_pair(name, value));
  }

  LoxValueType getAt(int depth, const Lexeme::Token& token) {
    Environment* env = this;
    while (depth--) {
      env = env->parent_env.get();
    }
    return env->get(token);
  }

  void assignAt(int depth, const Lexeme::Token& token, LoxValueType value) {
    Environment* env = this;
    while (depth--) {
      env = env->parent_env.get();
    }
    env->assign(token, value);
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

  // update value only, along linked env node.
  void assign(const Lexeme::Token& name, LoxValueType value) {
    if (map.find(std::string{name.lexeme}) != map.end()) {
      map.insert_or_assign(std::string{name.lexeme}, value);
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
