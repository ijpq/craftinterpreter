
#pragma once
#include "token.h"
#include "tokentype.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

namespace Lexeme {
class Scanner;
class Lox {
  bool hadError = false;
  std::vector<Lexeme::Token> tokens;
  std::unique_ptr<Scanner> scanner;

public:
  void run(const std::string &contents);
  void runPrompt();
  void runFile(const char *path) {
    std::ifstream inputfile(path);
    std::stringstream buffer;
    std::string contents;

    if (inputfile.is_open()) {
      buffer << inputfile.rdbuf();
      contents = buffer.str();
      inputfile.close();
      run(contents);
    } else {
      hadError = true;
      std::exit(-1);
    }
  }

  static void error(int line, std::string message) {
    report(line, "", message);
  }

  static void error(Token token, std::string msg) {
    if (token.type == TokenType::_EOF)
      report(token.line, " at end", msg);
    else
      report(token.line, " at '" + std::string{token.lexeme} + "'", msg);
  }

  static void report(int line, std::string where, std::string message) {
    std::cout << "[line " << line << "] Error" << where << ":" << message
              << std::endl;
  }
};

} // namespace Lexeme
