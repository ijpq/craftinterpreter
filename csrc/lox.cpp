#include "include/scanner.h"

namespace Lexeme {

void Lox::run(const std::string &contents) {
  scanner = std::make_unique<Scanner>(contents);
  tokens = scanner->scanTokens();

  for (auto token : tokens) {
    std::cout << token << std::endl;
  }
}

void Lox::runPrompt() {
  std::string line;

  for (;;) {
    std::cout << "> " << std::flush;

    if (!std::getline(std::cin, line)) {
      break;
    }

    run(line);
    hadError = false;
  }
}
} // namespace Lexeme

int main(int argc, char *argv[]) {
  using namespace Lexeme;
  Lox lox;
  
  if (argc > 2) {
    std::cerr << "Usage: craftinginterpreter [script]" << std::endl;
    std::exit(64);
  } else if (argc == 2) {
    lox.runFile(argv[1]);  // argv[1] 是脚本文件
  } else {
    lox.runPrompt();  // argc == 1,进入交互模式
  }
  
  return 0;
}
