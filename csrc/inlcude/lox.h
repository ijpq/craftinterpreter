#include <iostream>
#include <fstream>
#include <functional>
#include <sstream>
#include <memory>
#include "scanner.h"



class Lox {
    bool hadError = false;
    std::vector<Lexeme::Token> tokens;
public:

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
        hadError = true;
        std::exit(-1);    
    }
}

void run(const std::string& contents) {
    Lexeme::Scanner scanner(contents);
    tokens = scanner.scanTokens();

    for (auto token : tokens) {
        std::cout << token << std::endl;
    }

}
};

