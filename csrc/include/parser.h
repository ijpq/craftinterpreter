#include <exception>
#include <initializer_list>
#include <memory>
#include <vector>

#include "expr.h"
#include "lox.h"
#include "stmt.h"
#include "token.h"
#include "tokentype.h"

using namespace Lexeme;
using namespace syntax;
using Lexeme::Token;
using Lexeme::TokenType;
using syntax::Binary;
using syntax::Expr;
namespace syntax {
// clang-format off
/*
expression     → literal
               | unary
               | binary
               | grouping ;

literal        → NUMBER | STRING | "true" | "false" | "nil" ;
grouping       → "(" expression ")" ;
unary          → ( "-" | "!" ) expression ;
binary         → expression operator expression ;
operator       → "==" | "!=" | "<" | "<=" | ">" | ">="
               | "+"  | "-"  | "*" | "/" ;
*/
/*
expression     → equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;
*/
// clang-format on
struct ParserError : std::exception {
  ParserError(Token token, std::string error) { Lox::error(token, error); }
};

struct Parser {
  int current = 0;
  std::vector<Token> tokens;

  Parser(std::vector<Token> tokens) : tokens(tokens) {}

  bool isAtEnd() { return peek().type == TokenType::_EOF; }
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
  Token& peek() { return tokens[current]; }
  Token& previous() { return tokens[current - 1]; }

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
          return std::move(std::make_unique<syntax::Literal>(
              syntax::Literal::LiteralValue(std::get<double>(op.literal))));
        case (TokenType::STRING):
          return std::move(
              std::make_unique<syntax::Literal>(std::get<1>(op.literal)));

        case (TokenType::TRUE):
          return std::move(std::make_unique<syntax::Literal>(true));
        case (TokenType::FALSE):
          return std::move(std::make_unique<syntax::Literal>(false));
        case (TokenType::NIL):
          return std::move(std::make_unique<syntax::Literal>(
              syntax::Literal::LiteralValue(std::monostate{})));
        default:
          throw ParserError(tokens[current], "unexpected token");
      }
    } else if (match({TokenType::LEFT_PAREN})) {
      auto expr = expression();
      consume(TokenType::RIGHT_PAREN, "expect right parenthesis after group");
      auto group = std::make_unique<Grouping>(std::move(expr));
      return std::move(group);
    } else {
      throw ParserError(peek(), " Expect expression.");
    }
  }

  void consume(TokenType expected, std::string err_msg) {
    if (expected == tokens[current].type) {
      advance();
      return;
    }
    throw ParserError(tokens[current], "unfinished token");
  }

  void synchronize() {
    advance();
    while (!isAtEnd()) {
      if (previous().type == TokenType::SEMICOLON) return;
      switch (peek().type) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
          return;
        default:
          advance();
      }
    }
  }

  std::vector<std::unique_ptr<SST::Stmt>> parse() {
    std::vector<std::unique_ptr<SST::Stmt>> statements;
    while (!isAtEnd()) {
      statements.push_back(statement());
    }
    return statements;
  }

  std::unique_ptr<SST::Stmt> statement() {
    if (match({TokenType::PRINT})) return printStatement();
    return expressionStatement();
  }

  std::unique_ptr<SST::Print> printStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(Lexeme::TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<SST::Print>(std::move(value));
  }

  std::unique_ptr<SST::Expression> expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    consume(Lexeme::TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<SST::Expression>(std::move(expr));
  }
};
}  // namespace syntax
