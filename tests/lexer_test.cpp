#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "lexer.h"

static std::vector<TokenType> types_of(const std::string& src) {
    Lexer lexer(src);
    std::vector<TokenType> types;
    for (;;) {
        Token t = lexer.next_token();
        types.push_back(t._type);
        if (t._type == TokenType::EndOfFile) break;
    }
    return types;
}

TEST(LexerTest, LexemesMatchSource) {
    Lexer lexer("(+)");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::LeftParen);
    EXPECT_EQ(a.lexeme, "(");

    Token b = lexer.next_token();
    EXPECT_EQ(b._type, TokenType::Plus);
    EXPECT_EQ(b.lexeme, "+");

    Token c = lexer.next_token();
    EXPECT_EQ(c._type, TokenType::RightParen);
    EXPECT_EQ(c.lexeme, ")");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, Punctuation) {
    std::vector<TokenType> expected = {
        TokenType::LeftParen, TokenType::RightParen,
        TokenType::LeftBrace, TokenType::RightBrace,
        TokenType::Comma, TokenType::Semicolon,
        TokenType::Plus, TokenType::Minus,
        TokenType::Star, TokenType::Slash,
        TokenType::EndOfFile,
    };
    EXPECT_EQ(types_of("(){},;+-*/"), expected);
}

TEST(LexerTest, BangBangEqual) {
    std::vector<TokenType> expected = {
        TokenType::Bang, TokenType::BangEqual, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("!!="), expected);
}

TEST(LexerTest, ErrorToken) {
    Lexer lexer("@");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::Error);
    EXPECT_EQ(a.lexeme, "Unexpected character.");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, EqualEqualEqual) {
    std::vector<TokenType> expected = {
        TokenType::EqualEqual, TokenType::Equal, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("==="), expected);
}

TEST(LexerTest, EndOfInput1) {
    std::vector<TokenType> expected = {
        TokenType::Bang, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("!"), expected);
}

TEST(LexerTest, EndOfInput2) {
    std::vector<TokenType> expected = {
        TokenType::Equal, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("="), expected);
}

TEST(LexerTest, GreaterEqual) {
    std::vector<TokenType> expected = {
        TokenType::GreaterEqual, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of(">="), expected);
}

TEST(LexerTest, LessEqual) {
    std::vector<TokenType> expected = {
        TokenType::LessEqual, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("<="), expected);
}

TEST(LexerTest, Greater) {
    std::vector<TokenType> expected = {
        TokenType::Greater, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of(">"), expected);
}

TEST(LexerTest, Less) {
    std::vector<TokenType> expected = {
        TokenType::Less, TokenType::EndOfFile
    };
    EXPECT_EQ(types_of("<"), expected);
}

TEST(LexerTest, LexemesMatchSource2) {
    Lexer lexer("!=");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::BangEqual);
    EXPECT_EQ(a.lexeme, "!=");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}
