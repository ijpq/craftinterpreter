#include "include/token.h"
namespace Lexeme {

std::string literal_to_str(Literal literal) {
  return std::visit(
      [](auto &&args) -> std::string {
        using T = std::decay_t<decltype(args)>;
        if constexpr (std::is_same_v<T, std::monostate>)
          return "null";
        else if constexpr (std::is_same_v<T, double>)
          return std::to_string(args);
        else if constexpr (std::is_same_v<T, std::string_view>)
          return std::string(args);
      },
      literal);
}
std::string tokentype_to_string(TokenType type) {
    static const std::string strings[] = {
        "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
        "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR",

        "BANG", "BANG_EQUAL",
        "EQUAL", "EQUAL_EQUAL",
        "GREATER", "GREATER_EQUAL",
        "LESS", "LESS_EQUAL",

        "IDENTIFIER", "STRING", "NUMBER",

        "AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR",
        "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE",

        "EOF"
    };

    int index = static_cast<int>(type);
    
    if (index < 0 || index >= (sizeof(strings) / sizeof(strings[0]))) {
        return "UNKNOWN_TOKEN";
    }

    return strings[index];
}
std::ostream &operator<<(std::ostream &os, const Token &token) {
  os << token.to_str();
  return os;
}

} // namespace Lexeme
