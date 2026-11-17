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

TEST(ParserTest, UnaryMinus) {
    Lexer lexer("-3");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* unary = dynamic_cast<Unary*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op._type, TokenType::Minus);

    auto* operand = dynamic_cast<Literal*>(unary->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value._type, TokenType::Number);
    EXPECT_EQ(operand->value.lexeme, "3");
}

TEST(ParserTest, UnaryNot) {
    Lexer lexer("!true");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* unary = dynamic_cast<Unary*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op._type, TokenType::Bang);

    auto* operand = dynamic_cast<Literal*>(unary->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value._type, TokenType::True);
}

TEST(ParserTest, ChainedUnaryOperators) {
    Lexer lexer("--3");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* outer = dynamic_cast<Unary*>(expr.get());
    ASSERT_NE(outer, nullptr);
    EXPECT_EQ(outer->op._type, TokenType::Minus);

    auto* inner = dynamic_cast<Unary*>(outer->operand.get());
    ASSERT_NE(inner, nullptr);
    EXPECT_EQ(inner->op._type, TokenType::Minus);

    auto* operand = dynamic_cast<Literal*>(inner->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value.lexeme, "3");
}

TEST(ParserTest, UnaryMinusGroupedExpression) {
    Lexer lexer("-(5)");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* unary = dynamic_cast<Unary*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op._type, TokenType::Minus);

    auto* operand = dynamic_cast<Literal*>(unary->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value.lexeme, "5");
}

TEST(ParserErrors, UnaryOperatorWithoutOperand) {
    expect_parse_error("-", "Expected expression", 1);
}

TEST(ParserTest, Multiplication) {
    Lexer lexer("1 * 2");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* binary = dynamic_cast<Binary*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->op._type, TokenType::Star);

    auto* left = dynamic_cast<Literal*>(binary->left.get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->value.lexeme, "1");

    auto* right = dynamic_cast<Literal*>(binary->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.lexeme, "2");
}

TEST(ParserTest, MultiplicationIsLeftAssociative) {
    Lexer lexer("1 * 2 * 3");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* outer = dynamic_cast<Binary*>(expr.get());
    ASSERT_NE(outer, nullptr);
    EXPECT_EQ(outer->op._type, TokenType::Star);

    auto* inner = dynamic_cast<Binary*>(outer->left.get());
    ASSERT_NE(inner, nullptr);
    EXPECT_EQ(inner->op._type, TokenType::Star);

    auto* first = dynamic_cast<Literal*>(inner->left.get());
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->value.lexeme, "1");

    auto* second = dynamic_cast<Literal*>(inner->right.get());
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->value.lexeme, "2");

    auto* third = dynamic_cast<Literal*>(outer->right.get());
    ASSERT_NE(third, nullptr);
    EXPECT_EQ(third->value.lexeme, "3");
}

TEST(ParserTest, Division) {
    Lexer lexer("6 / 2");
    Parser parser(lexer);
    auto expr = parser.parse_expression();

    auto* binary = dynamic_cast<Binary*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->op._type, TokenType::Slash);

    auto* left = dynamic_cast<Literal*>(binary->left.get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->value.lexeme, "6");

    auto* right = dynamic_cast<Literal*>(binary->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.lexeme, "2");
}