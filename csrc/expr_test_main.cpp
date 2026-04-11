#include "include/expr.h"
#include "include/token.h"
#include "include/tokentype.h"

int main(int argc, char *argv[]) {
  using namespace syntax;
  auto minus_token =
      Lexeme::Token(Lexeme::TokenType::MINUS, "-", std::monostate{}, 1);
  auto literal_123 =
      std::make_unique<Literal>(syntax::Literal::LiteralValue("123"));
  auto star = Lexeme::Token(Lexeme::TokenType::STAR, "*", std::monostate{}, 1);
  auto literal_4567 =
      std::make_unique<Literal>(syntax::Literal::LiteralValue("45.67"));
  auto unary_minus =
      std::make_unique<Unary>(minus_token, std::move(literal_123));
  auto grouping = std::make_unique<Grouping>(std::move(literal_4567));
  Binary expression = Binary(std::move(unary_minus), star, std::move(grouping));
  auto printer = ASTPrinter();
  std::cout << printer.print(&expression) << std::endl;
}
