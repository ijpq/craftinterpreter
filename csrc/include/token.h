#pragma once
#include "tokentype.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
namespace Lexeme {
class Token;

using Literal = std::variant<double, std::string_view, std::monostate>;

std::ostream &operator<<(std::ostream &os, const Token &token);
std::string double_to_string(double value);
std::string literal_to_str(Literal literal);

struct Token {
  TokenType type;
  std::string_view lexeme; // include quotes
  Literal literal;         // drop quotes
  int line;
  std::string output_str;

public:
  Token(TokenType _type, std::string_view _lexeme, Literal _literal, int _line)
      : type(_type), lexeme(_lexeme), literal(_literal), line(_line) {
    output_str = tokenTypeToString(type) + " " + std::string(lexeme) + " " +
                 literal_to_str(literal);
  }

  std::string_view to_str() const { return output_str; }
};

} // namespace Lexeme
