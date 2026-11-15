#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include "parser.h"

static void expect_literal(const std::string& src, TokenType type, std::string_view lexeme) {
    Lexer lexer(src);
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* lit = dynamic_cast<Literal*>(expr.get());
    ASSERT_NE(lit, nullptr) << "not a Literal: " << src;
    EXPECT_EQ(lit->value._type, type) << src;
    EXPECT_EQ(lit->value.lexeme, lexeme) << src;
}

static void expect_parse_error(const std::string& src, const std::string& fragment, int line) {
    Lexer lexer(src);
    try {
        Parser parser(lexer);   
        parser.parse_expression();
        FAIL() << "expected ParseError for: " << src;
    }
    catch (const ParseError& e) {
        EXPECT_NE(std::string(e.what()).find(fragment), std::string::npos)
            << "message was: " << e.what();
        EXPECT_EQ(e.line, line) << "wrong line for: " << src;
    }
}

TEST(ParserTest, Number) {
    expect_literal("3", TokenType::Number, "3");
}

TEST(ParserTest, DecimalNumber) {
    expect_literal("3.14", TokenType::Number, "3.14");
}

TEST(ParserTest, String) {
    expect_literal("\"hi\"", TokenType::String, "\"hi\"");
}

TEST(ParserTest, True) {
    expect_literal("true", TokenType::True, "true");
}

TEST(ParserTest, False) {
    expect_literal("false", TokenType::False, "false");
}

TEST(ParserTest, Nil) {
    expect_literal("nil", TokenType::Nil, "nil");
}

TEST(ParserTest, GroupedLiteralReturnsInnerNode) {
    expect_literal("(5)", TokenType::Number, "5");
}

TEST(ParserTest, NestedGrouping) {
    expect_literal("((5))", TokenType::Number, "5");
}

TEST(ParserTest, DeeplyNestedGrouping) {
    expect_literal("((((true))))", TokenType::True, "true");
}

TEST(ParserTest, WhitespaceAndCommentsAreIgnored) {
    expect_literal("  ( // note\n 7 )  ", TokenType::Number, "7");
}

TEST(ParserErrors, MissingClosingParen) {
    expect_parse_error("(5", "')'", 1);
}

TEST(ParserErrors, MissingClosingParenNested) {
    expect_parse_error("((5)", "')'", 1);
}

TEST(ParserErrors, StrayClosingParen) {
    expect_parse_error(")", "Expected expression", 1);
}

TEST(ParserErrors, EmptyInput) {
    expect_parse_error("", "Expected expression", 1);
}

TEST(ParserErrors, EmptyGroup) {
    expect_parse_error("()", "Expected expression", 1);
}

TEST(ParserErrors, OperatorWhereExpressionExpected) {
    expect_parse_error("+", "Expected expression", 1);
}

TEST(ParserErrors, LexerErrorIsReported) {
    expect_parse_error("@", "Unexpected character", 1);
}

TEST(ParserErrors, UnterminatedStringIsReported) {
    expect_parse_error("\"abc", "Unterminated string", 1);
}

TEST(ParserErrors, LexerErrorInsideGroup) {
    expect_parse_error("(@)", "Unexpected character", 1);
}

TEST(ParserErrors, ErrorLineNumberAfterNewlines) {
    expect_parse_error("\n\n)", "Expected expression", 3);
}

TEST(ParserErrors, MissingParenReportedAtEndOfInput) {
    expect_parse_error("(1\n\n", "')'", 3);
}