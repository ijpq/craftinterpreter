
#include "include/token.h"
namespace Lexeme {

std::string_view literal_to_str(Literal literal) {
  return std::visit(
      [](auto &&args) -> std::string {
        using T = std::decay<decltype(args)>;
        if constexpr (std::is_same_v<T, std::monostate>)
          return "null";
        else if constexpr (std::is_same_v<T, double>)
          return std::to_string(args);
        else if constexpr (std::is_same_v<T, std::string_view>)
          return args;
      },
      literal);
}

  std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.to_str() ;
    return os;
  }
}