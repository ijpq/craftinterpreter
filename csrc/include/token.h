#pragma once
#include "tokentype.h"
#include <string>
#include <type_traits>
#include <variant>
#include <iostream>
namespace Lexeme {
class Token;

using Literal = std::variant<double, std::string_view, std::monostate>;

std::string tokentype_to_string(TokenType token);
std::ostream &operator<<(std::ostream &os, const Token &token);
std::string literal_to_str(Literal literal);  // 修复:返回 std::string
class Token {
  TokenType type;
  std::string_view lexeme;
  Literal literal;
  int line;
  std::string output_str;

public:
  Token(TokenType _type, std::string_view _lexeme, Literal _literal, int _line)
      : type(_type), lexeme(_lexeme), literal(_literal), line(_line) {
    output_str = tokentype_to_string(type) + " " + 
                 std::string(lexeme) + " " +
                 literal_to_str(literal);  // 现在返回 std::string,安全
  }

  std::string_view to_str() const { return output_str; }
};

} // namespace Lexeme
