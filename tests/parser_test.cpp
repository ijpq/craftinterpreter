#include "../csrc/include/parser.h"
#include "../csrc/include/scanner.h"
#include "../csrc/include/expr.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: parser_test <input_file>" << std::endl;
        return 1;
    }

    // Read source file
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Scan tokens
    Lexeme::Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    std::cout << "=== Tokens ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token << std::endl;
    }
    std::cout << std::endl;

    // Parse expression
    syntax::Parser parser(tokens);
    auto expr = parser.parse();

    if (expr == nullptr) {
        std::cerr << "Parse error occurred." << std::endl;
        return 1;
    }

    // Print AST
    std::cout << "=== AST ===" << std::endl;
    syntax::ASTPrinter printer;
    std::cout << printer.print(expr.get()) << std::endl;

    return 0;
}
