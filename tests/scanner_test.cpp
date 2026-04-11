#include "../csrc/include/scanner.h"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: scanner_test <input_file>" << std::endl;
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Could not open file: " << argv[1] << std::endl;
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  file.close();

  Lexeme::Scanner scanner(source);
  auto tokens = scanner.scanTokens();

  for (const auto& token : tokens) {
    std::cout << token << std::endl;
  }

  return 0;
}
