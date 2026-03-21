#include "include/token.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Lexeme {

std::string double_to_string(double value) {
  if (std::floor(value) == value && std::isfinite(value)) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value;
    return oss.str();
  }
  
  std::ostringstream oss;
  oss << value;
  std::string result = oss.str();
  
  if (result.find('.') != std::string::npos) {
    while (result.length() > 1 && result.back() == '0' && 
           result[result.length() - 2] != '.') {
      result.pop_back();
    }
  }
  
  return result;
}

std::string literal_to_str(Literal literal) {
  return std::visit(
      [](auto &&args) -> std::string {
        using T = std::decay_t<decltype(args)>;
        if constexpr (std::is_same_v<T, std::monostate>)
          return "null";
        else if constexpr (std::is_same_v<T, double>)
          return double_to_string(args);
        else if constexpr (std::is_same_v<T, std::string_view>)
          return std::string(args);
      },
      literal);
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
  os << token.to_str();
  return os;
}

} // namespace Lexeme
