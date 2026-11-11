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

struct Expected {
    TokenType type;
    std::string_view lexeme;
};

static void expect_tokens(const std::string& src, const std::vector<Expected>& expected) {
    Lexer lexer(src);
    for (std::size_t i = 0; i < expected.size(); ++i) {
        Token t = lexer.next_token();
        EXPECT_EQ(t._type, expected[i].type) << "token " << i << " of: " << src;
        EXPECT_EQ(t.lexeme, expected[i].lexeme) << "token " << i << " of: " << src;
    }
    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile) << "extra tokens in: " << src;
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

TEST(LexerTest, LeftParenSpaceRightParen) {
    Lexer lexer("( )");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::LeftParen);
    EXPECT_EQ(a.lexeme, "(");

    Token b = lexer.next_token();
    EXPECT_EQ(b._type, TokenType::RightParen);
    EXPECT_EQ(b.lexeme, ")");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, SpaceOnly) {
    Lexer lexer("  \t  ");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::EndOfFile);
    EXPECT_EQ(a.lexeme, "");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, LineCounting) {
    Lexer lexer("(\n)\n+");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::LeftParen);
    EXPECT_EQ(a.lexeme, "(");
    EXPECT_EQ(a.line_number, 1);

    Token b = lexer.next_token();
    EXPECT_EQ(b._type, TokenType::RightParen);
    EXPECT_EQ(b.lexeme, ")");
    EXPECT_EQ(b.line_number, 2);

    Token c = lexer.next_token();
    EXPECT_EQ(c._type, TokenType::Plus);
    EXPECT_EQ(c.lexeme, "+");
    EXPECT_EQ(c.line_number, 3);

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, CommentedHello) {
    Lexer lexer("// hello");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::EndOfFile);
    EXPECT_EQ(a.lexeme, "");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, SingleSlash) {
    Lexer lexer("/");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::Slash);
    EXPECT_EQ(a.lexeme, "/");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, LineCountingWithComments) {
    Lexer lexer("+ // note\n-");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::Plus);
    EXPECT_EQ(a.lexeme, "+");

    Token b = lexer.next_token();
    EXPECT_EQ(b._type, TokenType::Minus);
    EXPECT_EQ(b.lexeme, "-");
    EXPECT_EQ(b.line_number, 2);

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, SingleSlashCommentStart) {
    Lexer lexer("/ /");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::Slash);
    EXPECT_EQ(a.lexeme, "/");

    Token b = lexer.next_token();
    EXPECT_EQ(b._type, TokenType::Slash);
    EXPECT_EQ(b.lexeme, "/");

    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerTest, BackToBackComments) {
    Lexer lexer("//a\n//b\n+");

    Token a = lexer.next_token();
    EXPECT_EQ(a._type, TokenType::Plus);
    EXPECT_EQ(a.lexeme, "+");
    EXPECT_EQ(a.line_number, 3);
    
    EXPECT_EQ(lexer.next_token()._type, TokenType::EndOfFile);
}

TEST(LexerNumbers, SingleDigit) {
    expect_tokens("7", { { TokenType::Number, "7" } });
}

TEST(LexerNumbers, MultiDigitInteger) {
    expect_tokens("123", { { TokenType::Number, "123" } });
}

TEST(LexerNumbers, LeadingZerosAccepted) {
    expect_tokens("007", { { TokenType::Number, "007" } });
}

TEST(LexerNumbers, Decimal) {
    expect_tokens("3.14", { { TokenType::Number, "3.14" } });
}

TEST(LexerNumbers, TrailingDotIsNotPartOfNumber) {
    expect_tokens("3.", {
        { TokenType::Number, "3" },
        { TokenType::Error, "Unexpected character." },
        });
}

TEST(LexerNumbers, LeadingDotIsNotANumber) {
    expect_tokens(".5", {
        { TokenType::Error, "Unexpected character." },
        { TokenType::Number, "5" },
        });
}

TEST(LexerNumbers, OnlyOneDecimalPoint) {
    expect_tokens("1.5.5", {
        { TokenType::Number, "1.5" },
        { TokenType::Error, "Unexpected character." },
        { TokenType::Number, "5" },
        });
}

TEST(LexerNumbers, NumberFollowedByOperator) {
    expect_tokens("12+3", {
        { TokenType::Number, "12" },
        { TokenType::Plus, "+" },
        { TokenType::Number, "3" },
        });
}

TEST(LexerNumbers, NumbersSeparatedBySpace) {
    expect_tokens("1 22.5", {
        { TokenType::Number, "1" },
        { TokenType::Number, "22.5" },
        });
}

TEST(LexerNumbers, DivisionIsNotAComment) {
    expect_tokens("10/2", {
        { TokenType::Number, "10" },
        { TokenType::Slash, "/" },
        { TokenType::Number, "2" },
        });
}

TEST(LexerStrings, SimpleString) {
    expect_tokens("\"hello\"", { { TokenType::String, "\"hello\"" } });
}

TEST(LexerStrings, EmptyString) {
    expect_tokens("\"\"", { { TokenType::String, "\"\"" } });
}

TEST(LexerStrings, SpacesPreservedInside) {
    expect_tokens("\"a b  c\"", { { TokenType::String, "\"a b  c\"" } });
}

TEST(LexerStrings, OperatorsAndCommentsInsideAreNotLexed) {
    expect_tokens("\"+ - // x\"", { { TokenType::String, "\"+ - // x\"" } });
}

TEST(LexerStrings, StringFollowedByTokens) {
    expect_tokens("\"a\"+1", {
        { TokenType::String, "\"a\"" },
        { TokenType::Plus, "+" },
        { TokenType::Number, "1" },
        });
}

TEST(LexerStrings, TwoStrings) {
    expect_tokens("\"a\" \"b\"", {
        { TokenType::String, "\"a\"" },
        { TokenType::String, "\"b\"" },
        });
}

TEST(LexerStrings, UnterminatedString) {
    expect_tokens("\"abc", { { TokenType::Error, "Unterminated string." } });
}

TEST(LexerStrings, LoneQuoteIsUnterminated) {
    expect_tokens("\"", { { TokenType::Error, "Unterminated string." } });
}

TEST(LexerStrings, MultilineStringKeepsNewline) {
    expect_tokens("\"a\nb\"", { { TokenType::String, "\"a\nb\"" } });
}

TEST(LexerStrings, NewlineInsideStringCountsLines) {
    Lexer lexer("\"a\nb\" +");

    Token s = lexer.next_token();
    EXPECT_EQ(s._type, TokenType::String);

    Token p = lexer.next_token();
    EXPECT_EQ(p._type, TokenType::Plus);
    EXPECT_EQ(p.line_number, 2);
}