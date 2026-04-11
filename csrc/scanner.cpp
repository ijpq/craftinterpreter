#include "include/scanner.h"

#include <unordered_map>

namespace Lexeme {
const TokenMap Scanner::keywords{
    TokenPair{"and", TokenType::AND},
    TokenPair("class", TokenType::CLASS),
    TokenPair("else", TokenType::ELSE),
    TokenPair("false", TokenType::FALSE),
    TokenPair("for", TokenType::FOR),
    TokenPair("fun", TokenType::FUN),
    TokenPair("if", TokenType::IF),
    TokenPair("nil", TokenType::NIL),
    TokenPair("or", TokenType::OR),
    TokenPair("print", TokenType::PRINT),
    TokenPair("return", TokenType::RETURN),
    TokenPair("super", TokenType::SUPER),
    TokenPair("this", TokenType::THIS),
    TokenPair("true", TokenType::TRUE),
    TokenPair("var", TokenType::VAR),
    TokenPair("while", TokenType::WHILE),
};
}