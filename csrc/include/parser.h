#include <sys/stat.h>

#include <exception>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

#include "expr.h"
// #include "gtest/gtest.h"
#include "lox.h"
#include "loxvalue.h"
#include "stmt.h"
#include "token.h"
#include "tokentype.h"

using namespace Lexeme;
using namespace syntax;
using Lexeme::Token;
using Lexeme::TokenType;
using SST::Stmt;
using syntax::Binary;
using syntax::Expr;
namespace syntax {

using ArgumentsType = std::vector<std::unique_ptr<Expr>>;
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
expression     → assignment ;
assignment     → IDENTIFIER "=" assignment | logic_or;
logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | call ;
call           → primary ( "(" arguments? ")" )* ;
arguments      → expression ( "," expression )* ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" | IDENTIFIER ;
*/
// clang-format on
struct ParserError : std::exception {
  ParserError(Token token, std::string error) { Lox::error(token, error); }
};

struct Parser {
  int current = 0;
  std::vector<Token> tokens;

  Parser(std::vector<Token> tokens) : tokens(tokens) {}

  // helper to steps in tokens
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

  Lexeme::Token consume(TokenType expected, std::string err_msg) {
    if (expected == tokens[current].type) {
      advance();
      return previous();
    }
    throw ParserError(tokens[current], "unfinished token");
  }

  // grammar rules. Recursive descent along stratified grammar rules.
  std::unique_ptr<Expr> expression() { return assignment(); }

  std::unique_ptr<Expr> assignment() {
    std::unique_ptr<Expr> expr = logical_or();
    if (match({TokenType::EQUAL})) {
      Token equals = previous();
      std::unique_ptr<Expr> value = assignment();

      Expr* var = nullptr;
      if ((var = dynamic_cast<Variable*>(expr.get())) !=
          nullptr) {  // if left is variable
        Token name = dynamic_cast<Variable*>(expr.get())->name;
        return std::make_unique<Assign>(name, std::move(value));
      }
      // error(equals, "Invalid assignment target.");
    }
    return expr;
  }

  std::unique_ptr<Expr> logical_or() {
    std::unique_ptr<Expr> expr = logical_and();
    while (match({TokenType::OR})) {
      Token op = previous();  // "or" keyword
      std::unique_ptr<Expr> right = logical_and();
      expr = std::make_unique<syntax::Logical>(std::move(expr), op,
                                               std::move(right));
    }
    return expr;
  }

  std::unique_ptr<Expr> logical_and() {
    std::unique_ptr<Expr> expr = equality();
    while (match({TokenType::AND})) {
      Token op = previous();
      std::unique_ptr<Expr> right = equality();
      expr = std::make_unique<syntax::Logical>(std::move(expr), op,
                                               std::move(right));
    }
    return expr;
  }

  std::unique_ptr<Expr> equality() {
    auto expr = std::unique_ptr<Expr>(comparison());
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
      Token op = previous();
      auto right = std::unique_ptr<Expr>(comparison());
      std::unique_ptr<Expr> binary =
          std::make_unique<Binary>(std::move(expr), op, std::move(right));

      expr = std::move(binary);
    }
    return expr;
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
    return expr;
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
    return expr;
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
    return expr;
  }

  std::unique_ptr<Expr> unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
      Token op = previous();
      auto right = unary();
      auto unary = std::make_unique<Unary>(std::move(op), std::move(right));
      return std::move(unary);
    } else {
      auto expr = call();
      return expr;
    }
  }

  /*
  call() wraps linked invocation into embedded Call object.
  f(args1...)(args2...)...
  = Call(f, ')', args1...)(args2...)...
  = Call(Call(f, ')', args1...), ')', ...)...
  */
  std::unique_ptr<Expr> call() {
    std::unique_ptr<Expr> callee = primary();
    ArgumentsType args;
    while (1) {
      if (match({TokenType::LEFT_PAREN})) {
        callee = finishcall(std::move(callee));
      } else {
        break;
      }
    }
    return callee;
  }

  ArgumentsType arguments() {
    ArgumentsType args;
    std::unique_ptr<Expr> expr = expression();
    args.push_back(std::move(expr));
    while (match({TokenType::COMMA})) {
      std::unique_ptr<Expr> next_arg = expression();
      args.push_back(std::move(next_arg));
    }
    return std::move(args);
  }

  std::unique_ptr<Expr> primary() {
    if (match({TokenType::NUMBER, TokenType::STRING, TokenType::TRUE,
               TokenType::FALSE, TokenType::NIL})) {
      Token op = previous();
      switch (op.type) {
        case (TokenType::NUMBER):
          return std::make_unique<syntax::Literal>(
              syntax::Literal::LiteralValue(std::get<double>(op.literal)));
        case (TokenType::STRING):
          return std::make_unique<syntax::Literal>(std::get<1>(op.literal));

        case (TokenType::TRUE):
          return std::make_unique<syntax::Literal>(true);
        case (TokenType::FALSE):
          return std::make_unique<syntax::Literal>(false);
        case (TokenType::NIL):
          return std::make_unique<syntax::Literal>(
              syntax::Literal::LiteralValue(std::monostate{}));
        default:
          throw ParserError(tokens[current], "unexpected token");
      }
    } else if (match({TokenType::IDENTIFIER})) {
      Token identifier = previous();
      return std::make_unique<syntax::Variable>(identifier);
    } else if (match({TokenType::LEFT_PAREN})) {
      auto expr = expression();
      consume(TokenType::RIGHT_PAREN, "expect right parenthesis after group");
      auto group = std::make_unique<Grouping>(std::move(expr));
      return std::move(group);
    } else {
      throw ParserError(peek(), " Expect expression.");
    }
  }

  std::unique_ptr<Expr> finishcall(std::unique_ptr<Expr> callee) {
    ArgumentsType args;
    if (peek().type != TokenType::RIGHT_PAREN) {  // have arguments
      args = arguments();
      consume(TokenType::RIGHT_PAREN, "!");
      Token right_paren = previous();
      return std::make_unique<syntax::Call>(std::move(callee), right_paren,
                                            std::move(args));
    } else {  // empty arguments
      consume(TokenType::RIGHT_PAREN, "!");
      Token right_paren = previous();
      return std::make_unique<syntax::Call>(std::move(callee), right_paren,
                                            ArgumentsType{});
    }
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

  std::unique_ptr<Stmt> forstatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    std::unique_ptr<Stmt> initializer;
    if (match({TokenType::SEMICOLON})) {
      initializer = nullptr;
    } else if (match({TokenType::VAR})) {
      initializer = varDeclaration();
    } else {
      initializer = expressionStatement();
    }

    std::unique_ptr<Expr> cond;
    if (tokens[current].type != TokenType::SEMICOLON) {
      cond = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment;
    if (tokens[current].type != TokenType::RIGHT_PAREN) {
      increment = expression();
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    std::unique_ptr<Stmt> body = statement();
    if (increment != nullptr) {
      std::vector<std::unique_ptr<Stmt>> combine_body_incre;
      combine_body_incre.push_back(std::move(body));
      combine_body_incre.push_back(
          std::move(std::make_unique<SST::Expression>(std::move(increment))));
      body = std::make_unique<SST::Block>(std::move(combine_body_incre));
    }
    if (cond == nullptr) {
      cond = std::make_unique<Literal>(Literal::LiteralValue{true});
    }
    body = std::make_unique<SST::While>(std::move(cond), std::move(body));
    if (initializer != nullptr) {
      std::vector<std::unique_ptr<Stmt>> combine_init_body_incre;
      combine_init_body_incre.push_back(std::move(initializer));
      combine_init_body_incre.push_back(std::move(body));
      body = std::make_unique<SST::Block>(std::move(combine_init_body_incre));
    }
    return body;
  }

  std::unique_ptr<Stmt> whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after while.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
    std::unique_ptr<Stmt> body = statement();
    return std::make_unique<SST::While>(std::move(condition), std::move(body));
  }

  std::unique_ptr<SST::Stmt> returnstatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (tokens[current].type != TokenType::SEMICOLON) {
      value = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<SST::Return>(keyword, std::move(value));
  }

  std::unique_ptr<SST::Stmt> ifstatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(Lexeme::TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    std::unique_ptr<SST::Stmt> then = statement();
    std::unique_ptr<SST::Stmt> elsebranch = nullptr;
    if (match({TokenType::ELSE})) {
      elsebranch = statement();
    }

    return std::make_unique<SST::If>(std::move(condition), std::move(then),
                                     std::move(elsebranch));
  }

  std::unique_ptr<SST::Stmt> printStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(Lexeme::TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<SST::Print>(std::move(value));
  }

  std::unique_ptr<SST::Stmt> expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    consume(Lexeme::TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<SST::Expression>(std::move(expr));
  }

  std::vector<std::unique_ptr<SST::Stmt>> block() {
    std::vector<std::unique_ptr<SST::Stmt>> statements;
    while (!(TokenType::RIGHT_BRACE == tokens[current].type) && !isAtEnd()) {
      statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
  }

  // parameters     → IDENTIFIER ( "," IDENTIFIER )* ;
  std::vector<Token> parameter() {
    std::vector<Token> ret;
    Token identifier = consume(TokenType::IDENTIFIER, "!");
    ret.push_back(identifier);
    while (match({TokenType::COMMA})) {
      Token next_id = consume(TokenType::IDENTIFIER, "!");
      ret.push_back(next_id);
    }
    return ret;
  }

  /*
  build AST , use AST to build statement
  */
  std::unique_ptr<SST::Stmt> statement() {
    if (match({TokenType::RETURN})) return returnstatement();
    if (match({TokenType::IF})) return ifstatement();
    if (match({TokenType::FOR})) return forstatement();
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::LEFT_BRACE}))
      return std::make_unique<SST::Block>(block());
    return expressionStatement();
  }

  // funDecl        → "fun" function ;
  // function       → IDENTIFIER "(" parameters? ")" block ;
  std::unique_ptr<SST::Stmt> function(std::string kind) {
    Token identifier = consume(TokenType::IDENTIFIER, "!");
    consume(TokenType::LEFT_PAREN, "!");
    std::vector<Token> param;
    if (peek().type == TokenType::IDENTIFIER) {  // have parameter
      param = parameter();
      consume(TokenType::RIGHT_PAREN, "!");
    }
    auto body = block();
    return std::make_unique<SST::Function>(identifier, param, std::move(body));
  }

  // varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
  std::unique_ptr<SST::Var> varDeclaration() {
    if (match({TokenType::IDENTIFIER})) {
      Token identifier = previous();
      std::unique_ptr<syntax::Expr> initializer;
      if (match({TokenType::EQUAL})) {
        initializer = expression();
      } else {
        initializer = nullptr;
      }
      consume(Lexeme::TokenType::SEMICOLON, "Expect ';' after expression.");
      return std::make_unique<SST::Var>(identifier, std::move(initializer));
    }
  }

  // var/func declaration
  std::unique_ptr<SST::Stmt> declaration() {
    try {
      if (match({TokenType::VAR})) return varDeclaration();
      if (match({TokenType::FUN})) return function("function");
      return statement();
    } catch (ParserError& e) {
      synchronize();
      return nullptr;
    }
  }

  // parse source code, generate Stmts
  std::vector<std::unique_ptr<SST::Stmt>> parse() {
    std::vector<std::unique_ptr<SST::Stmt>> statements;
    while (!isAtEnd()) {
      statements.push_back(declaration());
    }
    return statements;
  }

  // std::unique_ptr<SST::Var> varStatement() {
  //   auto stmt = varDeclaration();
  //   return stmt;
  // }
};
}  // namespace syntax
