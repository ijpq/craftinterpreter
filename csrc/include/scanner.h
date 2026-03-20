#pragma once
#include "token.h"
#include <memory>
#include <string>
#include <unordered_map>
#include "lox.h"
namespace Lexeme {
class Scanner {
  std::string_view source;
  std::vector<Token> tokens;
  static const std::unordered_map<std::string_view, TokenType> keywords;
  std::string_view::iterator start;
  std::string_view::iterator current; // is going to be dealt
  int line;

public:
  Scanner(const std::string &_source)
      : source(_source), start(source.begin()), current(source.begin()),
        line(1) {}
  bool isAtEnd() { return current != source.end(); }
  std::vector<Token> scanTokens() {
    while (!isAtEnd()) {
      start = current;
      scanToken();
    }

    tokens.emplace_back(EOF, "", std::string(), line);
    return tokens;
  }
  char advance() { return *current++; }

  // It has meaningful literal, like fp or str
  void addToken(TokenType type, Literal literal) {
    auto text = source.substr(start - source.begin(), current - start);
    tokens.emplace_back(type, text, literal, line);
  }

  // It doesn't have meaningful literal, i.e. monostate
  void addToken(TokenType type) { addToken(type, Literal(std::monostate{})); }

  bool isDigit(char c) { return c >= '0' && c <= '9' ? true : false; }
  bool match(const char &expected) {
    if (isAtEnd())
      return false;
    if (*current != expected)
      return false;
    current++;
    return true;
  }
  char peek() {
    if (isAtEnd())
      return '\0';
    return *current;
  }
  void number() {
    while ((isDigit(peek()) || '.' == peek()) && !isAtEnd()) {
      advance();
    }

    if (isAtEnd()) {
      Lox::error(line, "unterminated");
      return;
    }
    advance();
    double num = std::stod(
        std::string(source.substr(start - source.begin(), current - start)));
  }
  bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || ( c >= 'A' && c <= 'Z') || c == '_';
  }
  void identifier() {
    while (isAlphaNumeric(peek())) advance();
    auto text = source.substr(start - source.begin(), current - start);
    TokenType type;

    if (keywords.find(text) == keywords.end()) {
      type = TokenType::IDENTIFIER;
    } else type = keywords[text];
    addToken(type);
  }
  bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
  }
  void string() {
    while (peek() != '"' && !isAtEnd()) {
      if (peek() == '\n')
        line++;
      advance();
    }
    // if current is quote or meet end.
    // if is quote, it's normal str. but if it's end here, str isn't finished.
    if (isAtEnd()) {
      Lox.error(line, "unterminated string");
      return;
    }

    // now, current pointer is at termination quote. move it to next pos.
    advance();

    addToken(TokenType::STRING,
             source.substr(start - source.begin(), current - 1 - start));
  }
  void scanToken() {
    char c = advance();
    switch (c) {
    case '(':
      addToken(LEFT_PAREN);
      break;
    case ')':
      addToken(RIGHT_PAREN);
      break;
    case '{':
      addToken(LEFT_BRACE);
      break;
    case '}':
      addToken(RIGHT_BRACE);
      break;
    case ',':
      addToken(COMMA);
      break;
    case '.':
      addToken(DOT);
      break;
    case '-':
      addToken(MINUS);
      break;
    case '+':
      addToken(PLUS);
      break;
    case ';':
      addToken(SEMICOLON);
      break;
    case '*':
      addToken(STAR);
      break;
    case '!':
      addToken(match('=') ? BANG_EQUAL : BANG);
      break;
    case '=':
      addToken(match('=') ? EQUAL_EQUAL : EQUAL);
      break;
    case '<':
      addToken(match('=') ? LESS_EQUAL : LESS);
      break;
    case '>':
      addToken(match('=') ? GREATER_EQUAL : GREATER);
      break;
    case '/':
      if (match('/')) {
        // A comment goes until the end of the line.
        while (peek() != '\n' && !isAtEnd())
          advance();
      } else {
        addToken(SLASH);
      }
      break;
    case ' ':
    case '\r':
    case '\t':
      // Ignore whitespace.
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
        Lox.error(line, "Unexpected character.");
      }
      break;
    }
  }
};
} // namespace Lexeme