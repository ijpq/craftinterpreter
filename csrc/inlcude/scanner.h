#include "token.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace Lexeme {
class Scanner {
  std::string_view source;
  std::vector<Token> tokens;
  static const std::unordered_map<std::string, TokenType> keywords;
  std::string_view::iterator start;
  std::string_view::iterator current;
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
  char advance() {
    return *current;
    current++;
  }
  void addToken(TokenType type, Literal literal) {
    auto text = source.substr(start-source.begin(), current-start);
    tokens.emplace_back(type, text, );
  }
  void addToken(TokenType type) {
    addToken(type, std::string());
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