#pragma once
#include "lox.h"
#include "token.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace Lexeme {
using TokenPair = std::pair<std::string_view, TokenType>;
using TokenMap = std::unordered_map<std::string_view, TokenType>;
class Scanner {
  std::string source;
  std::vector<Token> tokens;
  static const TokenMap keywords;
  size_t start;
  size_t current;
  int line;

public:
  Scanner(const std::string &_source)
      : source(_source), start(0), current(0), line(1) {}

  bool isAtEnd() { return current >= source.length(); }

  std::vector<Token> scanTokens() {
    while (!isAtEnd()) {
      start = current;
      scanToken();
    }

    tokens.emplace_back(TokenType::_EOF, "", std::monostate{}, line);
    return tokens;
  }

  char advance() { return source[current++]; }

  void addToken(TokenType type, Literal literal) {
    std::string_view text(source.data() + start, current - start);
    tokens.emplace_back(type, text, literal, line);
  }

  void addToken(TokenType type) { addToken(type, std::monostate{}); }

  bool isDigit(char c) { return c >= '0' && c <= '9'; }

  bool match(char expected) {
    if (isAtEnd())
      return false;
    if (source[current] != expected)
      return false;
    current++;
    return true;
  }

  char peek() {
    if (isAtEnd())
      return '\0';
    return source[current];
  }

  char peekNext() {
    if (current + 1 >= source.length())
      return '\0';
    return source[current + 1];
  }

  void number() {
    while (isDigit(peek())) {
      advance();
    }

    if (peek() == '.' && isDigit(peekNext())) {
      advance();
      while (isDigit(peek())) {
        advance();
      }
    }

    double num = std::stod(source.substr(start, current - start));
    addToken(TokenType::NUMBER, num);
  }

  bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  bool isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

  void identifier() {
    while (isAlphaNumeric(peek()))
      advance();

    std::string_view text(source.data() + start, current - start);
    TokenType type = TokenType::IDENTIFIER;

    auto it = keywords.find(text);
    if (it != keywords.end()) {
      type = it->second;
    }
    addToken(type);
  }

  void string() {
    while (peek() != '"' && !isAtEnd()) {
      if (peek() == '\n')
        line++;
      advance();
    }

    if (isAtEnd()) {
      Lox::error(line, "unterminated string");
      return;
    }

    advance();

    std::string_view value(source.data() + start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
  }

  void scanToken() {
    char c = advance();
    switch (c) {
    case '(':
      addToken(TokenType::LEFT_PAREN);
      break;
    case ')':
      addToken(TokenType::RIGHT_PAREN);
      break;
    case '{':
      addToken(TokenType::LEFT_BRACE);
      break;
    case '}':
      addToken(TokenType::RIGHT_BRACE);
      break;
    case ',':
      addToken(TokenType::COMMA);
      break;
    case '.':
      addToken(TokenType::DOT);
      break;
    case '-':
      addToken(TokenType::MINUS);
      break;
    case '+':
      addToken(TokenType::PLUS);
      break;
    case ';':
      addToken(TokenType::SEMICOLON);
      break;
    case '*':
      addToken(TokenType::STAR);
      break;
    case '!':
      addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
      break;
    case '=':
      addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
      break;
    case '<':
      addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
      break;
    case '>':
      addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
      break;
    case '/':
      if (match('/')) {
        while (peek() != '\n' && !isAtEnd())
          advance();
      } else {
        addToken(TokenType::SLASH);
      }
      break;
    case ' ':
    case '\r':
    case '\t':
      break;
    case '"':
      string();
      break;
    case '\n':
      line++;
      break;
    default:
      if (isDigit(c)) {
        number();
      } else if (isAlpha(c)) {
        identifier();
      } else {
        Lox::error(line, "Unexpected character.");
      }
      break;
    }
  }
};
} // namespace Lexeme
