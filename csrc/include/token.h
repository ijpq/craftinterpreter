
#pragma once
#include "tokentype.h"
#include <string>
#include <type_traits>
#include <variant>
namespace Lexeme {
class Token;

using Literal = std::variant<double, std::string_view, std::monostate>;

std::ostream &operator<<(std::ostream &os, const Token &token);
std::string_view literal_to_str(Literal literal);
// struct Literal {
//     std::variant<double, std::string, std::monostate> literal;
//     std::string to_str() {
//         return std::visit([](auto&& args) -> std::string {
//             using T = std::decay<decltype(args)>;
//             if constexpr(std::is_same_v<T, std::monostate>) return "null";
//             else if constexpr(std::is_same_v<T, double>) return
//             std::to_string(args); else if constexpr(std::is_same_v<T,
//             std::string>) return args;
//         }, literal);
//     }
// };
class Token {
  TokenType type;
  std::string lexeme;
  Literal literal;
  int line;
  std::string output_str;

public:
  Token(TokenType _type, std::string _lexeme, Literal _literal, int _line)
      : type(_type), lexeme(_lexeme), literal(_literal), line(_line) {

    output_str = std::to_string(type) + std::string(" ") + lexeme + " " +
                 std::string(literal_to_str(literal));
  }

  std::string_view to_str() const { return output_str; }
};

} // namespace Lexeme