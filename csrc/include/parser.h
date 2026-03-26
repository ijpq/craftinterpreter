#include "expr.h"
#include "token.h"
#include "tokentype.h"
#include <exception>
#include <initializer_list>
#include <memory>
#include <vector>

using namespace Lexeme;
using namespace syntax;
using Lexeme::Token;
using Lexeme::TokenType;
using syntax::Binary;
using syntax::Expr;
struct Parser {
  int current = 0;
  std::vector<Token> tokens;

  Parser(std::vector<Token> tokens) : tokens(tokens) {}

  inline void advance() { current++; }
  bool match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
      if (tokens[current].type == type) {
        advance();
        return true;
      }
    }
    return false;
  }

  Token &previous() { return tokens[current - 1]; }

  std::unique_ptr<Expr> expression() { return std::move(equality()); }

  std::unique_ptr<Expr> equality() {
    auto expr = std::unique_ptr<Expr>(comparison());
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
      Token op = previous();
      auto right = std::unique_ptr<Expr>(comparison());
      std::unique_ptr<Expr> binary =
          std::make_unique<Binary>(std::move(expr), op, std::move(right));

      expr = std::move(binary);
    }
    return std::move(expr);
  }

  std::unique_ptr<Expr> comparison() {
    auto expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS,
                  TokenType::LESS_EQUAL})) {
      Token op = previous();
      auto right = term();
      auto binary =
          std::make_unique<Binary>(std::move(expr), op, std::move(right));
      expr = std::move(binary);
    }
    return std::move(expr);
  }

  std::unique_ptr<Expr> term() {
    auto expr = factor();
    while (match({TokenType::MINUS, TokenType::PLUS})) {
      Token op = previous();
      auto right = factor();
      auto binary =
          std::make_unique<Binary>(std::move(expr), op, std::move(right));
      expr = std::move(binary);
    }
    return std::move(expr);
  }

  std::unique_ptr<Expr> factor() {
    auto expr = unary();
    while (match({TokenType::SLASH, TokenType::STAR})) {
      Token op = previous();
      auto right = unary();
      auto binary =
          std::make_unique<Binary>(std::move(expr), op, std::move(right));
      expr = std::move(binary);
    }
    return std::move(expr);
  }

  std::unique_ptr<Expr> unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
      Token op = previous();
      auto right = unary();
      auto unary = std::make_unique<Unary>(std::move(op), std::move(right));
      return std::move(unary);
    } else {
      auto expr = primary();
      return std::move(expr);
    }
  }

  std::unique_ptr<Expr> primary() {
    if (match({TokenType::NUMBER, TokenType::STRING, TokenType::TRUE,
               TokenType::FALSE, TokenType::NIL})) {
      Token op = previous();
      switch (op.type) {
      case (TokenType::NUMBER):
        return std::move(
            std::make_unique<syntax::Literal>(std::get<double>(op.literal)));
      case (TokenType::STRING):
        return std::move(
            std::make_unique<syntax::Literal>(std::string{op.lexeme}));
      case (TokenType::TRUE):
        return std::move(std::make_unique<syntax::Literal>(true));
      case (TokenType::FALSE):
        return std::move(std::make_unique<syntax::Literal>(false));
      case (TokenType::NIL):
        return std::move(std::make_unique<syntax::Literal>(std::monostate{}));
      default:
        throw std::exception();
      }
    } else if (match({TokenType::LEFT_PAREN})) {
      auto expr = expression();
      consume(TokenType::RIGHT_PAREN, "expect right parenthesis after group");
      auto group = std::make_unique<Grouping>(std::move(expr));
      return std::move(group);
    }
  }

  void consume(TokenType expected, std::string err_msg) {
    advance();
    if (expected == tokens[current].type) {advance(); return;}
    throw std::exception();

  }


};