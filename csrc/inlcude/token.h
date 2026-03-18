#include "tokentype.h"
#include <string>
#include <variant>
#include <type_traits>
namespace Lexeme {
struct Literal {
    std::variant<double, std::string, std::monostate> literal;
    std::string to_str() {
        return std::visit([](auto args&&) {
            using T = std::decay<decltype(args)>;
            if constexpr(std::is_same_v<T, std::monostate>) return "null";
            else if constexpr(std::is_same_v<T, std::double>) return std::to_string(std::get<0>(args));
            else return std::get<0>(args);
        }, literal);
    }
};
class Token {
  TokenType type;
  std::string lexeme;
  Literal literal;
  int line;

public:
  Token(TokenType _type, std::string _lexeme, Literal _literal, int _line)
      : type(_type), lexeme(_lexeme), literal(_literal), line(_line) {}
  std::string_view to_str() {
    return std::to_string(type) + std::string(" ") + lexeme + literal.to_str();
  }
};
} // namespace Lexeme