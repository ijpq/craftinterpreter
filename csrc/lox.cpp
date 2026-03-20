
#include "include/scanner.h"

namespace Lexeme {

void Lox::run(const std::string &contents) {
  scanner = std::make_unique<Scanner>(contents);
  tokens = scanner->scanTokens();

  for (auto token : tokens) {
    std::cout << token << std::endl;
  }
}


} // namespace Lexeme
int main(int argc, char *argv[]) {
    using namespace Lexeme;
  Lox lox;
  if (argc > 1) {

  } else if (argc == 1) {
    lox.runFile(argv[0]);
  } else {
    lox.runPrompt();
  }
}