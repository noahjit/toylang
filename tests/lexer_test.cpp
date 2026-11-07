#include <gtest/gtest.h>
#include <vector>
#include "lexer.h"

static std::vector<Token> lex_all(const std::string& src) {
    Lexer lexer(src);
    std::vector<Token> tokens;
    while (true) {
        Token t = lexer.next_token();
        tokens.push_back(t);
        if (t._type == TokenType::EndOfFile) break;
    }
    return tokens;
}