
#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

#include "interpreter.h"
#include "runtimeerror.h"
#include "token.h"
#include "tokentype.h"

namespace Lexeme {
class Scanner;
class Lox {
  static bool hadError;
  static bool hadRuntimeError;
  std::vector<Lexeme::Token> tokens;
  std::unique_ptr<Scanner> scanner;
  static interpreter::Interpreter interpreter_;

 public:
  void run(const std::string& contents);
  void runPrompt();
  void runFile(const char* path) {
    std::ifstream inputfile(path);
    std::stringstream buffer;
    std::string contents;

    if (inputfile.is_open()) {
      buffer << inputfile.rdbuf();
      contents = buffer.str();
      inputfile.close();
      run(contents);
    } else {
      std::exit(-1);
    }

    if (hadError) std::exit(65);
    if (hadRuntimeError) std::exit(70);
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

  static void runtimeError(interpreter::InterpreterRuntimeError& error) {
    std::cout << error.getMessage() << "\n [Line " << error.token.line << "]"
              << std::endl;
    hadRuntimeError = true;
  }

  static void report(int line, std::string where, std::string message) {
    std::cout << "[line " << line << "] Error" << where << ":" << message
              << std::endl;
  }
};

}  // namespace Lexeme
