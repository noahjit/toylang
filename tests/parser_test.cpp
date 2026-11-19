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

static std::string show(const Expression* e) {
    if (auto* lit = dynamic_cast<const Literal*>(e))
        return std::string(lit->value.lexeme);
    if (auto* un = dynamic_cast<const Unary*>(e))
        return "(" + std::string(un->op.lexeme) + " " + show(un->operand.get()) + ")";
    if (auto* bin = dynamic_cast<const Binary*>(e))
        return "(" + std::string(bin->op.lexeme) + " " + show(bin->left.get()) + " "
        + show(bin->right.get()) + ")";
    if (auto* lg = dynamic_cast<const Logical*>(e))
        return "(" + std::string(lg->op.lexeme) + " " + show(lg->left.get()) + " "
        + show(lg->right.get()) + ")";
    if (auto* var = dynamic_cast<const Variable*>(e))
        return std::string(var->name.lexeme);
    return "?";
}

static std::string parse_to_string(const std::string& src) {
    Lexer lexer(src);
    Parser parser(lexer);
    auto expr = parser.parse();
    return show(expr.get());
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

TEST(ParserTerm, Simple) { EXPECT_EQ(parse_to_string("1 + 2"), "(+ 1 2)"); }
TEST(ParserTerm, PrecedenceMulOverAdd) { EXPECT_EQ(parse_to_string("1 + 2 * 3"), "(+ 1 (* 2 3))"); }
TEST(ParserTerm, PrecedenceMulFirst) { EXPECT_EQ(parse_to_string("1 * 2 + 3"), "(+ (* 1 2) 3)"); }
TEST(ParserTerm, LeftAssociativeSub) { EXPECT_EQ(parse_to_string("1 - 2 - 3"), "(- (- 1 2) 3)"); }
TEST(ParserTerm, MixedAddSub) { EXPECT_EQ(parse_to_string("1 - 2 + 3"), "(+ (- 1 2) 3)"); }
TEST(ParserTerm, GroupingOverrides) { EXPECT_EQ(parse_to_string("(1 + 2) * 3"), "(* (+ 1 2) 3)"); }
TEST(ParserTerm, UnaryBindsTighter) { EXPECT_EQ(parse_to_string("-1 + 2"), "(+ (- 1) 2)"); }
TEST(ParserTerm, BinaryThenUnary) { EXPECT_EQ(parse_to_string("1 - -2"), "(- 1 (- 2))"); }
TEST(ParserTerm, LongChain) { EXPECT_EQ(parse_to_string("1 + 2 * 3 - 4 / 5"), "(- (+ 1 (* 2 3)) (/ 4 5))"); }
TEST(ParserErrors, DanglingPlus) { expect_parse_error("1 +", "Expected expression", 1); }
TEST(ParserErrors, LeadingPlus) { expect_parse_error("+ 1", "Expected expression", 1); }

// comparison
TEST(ParserComparison, Less) { EXPECT_EQ(parse_to_string("1 < 2"), "(< 1 2)"); }
TEST(ParserComparison, GreaterEqual) { EXPECT_EQ(parse_to_string("1 >= 2"), "(>= 1 2)"); }
TEST(ParserComparison, LeftAssociative) { EXPECT_EQ(parse_to_string("1 < 2 < 3"), "(< (< 1 2) 3)"); }
TEST(ParserComparison, TermBindsTighter) { EXPECT_EQ(parse_to_string("1 + 2 < 3 * 4"), "(< (+ 1 2) (* 3 4))"); }

// equality
TEST(ParserEquality, EqualEqual) { EXPECT_EQ(parse_to_string("1 == 2"), "(== 1 2)"); }
TEST(ParserEquality, BangEqual) { EXPECT_EQ(parse_to_string("1 != 2"), "(!= 1 2)"); }
TEST(ParserEquality, LeftAssociative) { EXPECT_EQ(parse_to_string("1 == 2 == 3"), "(== (== 1 2) 3)"); }
TEST(ParserEquality, ComparisonBindsTighter) { EXPECT_EQ(parse_to_string("1 < 2 == 3 < 4"), "(== (< 1 2) (< 3 4))"); }
TEST(ParserEquality, BoolOperands) { EXPECT_EQ(parse_to_string("true == !false"), "(== true (! false))"); }

// logical
TEST(ParserLogic, And) { EXPECT_EQ(parse_to_string("true and false"), "(and true false)"); }
TEST(ParserLogic, Or) { EXPECT_EQ(parse_to_string("true or false"), "(or true false)"); }
TEST(ParserLogic, AndBindsTighterThanOr) { EXPECT_EQ(parse_to_string("a or b and c"), "(or a (and b c))"); }
TEST(ParserLogic, AndFirstThenOr) { EXPECT_EQ(parse_to_string("a and b or c"), "(or (and a b) c)"); }
TEST(ParserLogic, OrLeftAssociative) { EXPECT_EQ(parse_to_string("a or b or c"), "(or (or a b) c)"); }
TEST(ParserLogic, GroupingOverrides) { EXPECT_EQ(parse_to_string("(a or b) and c"), "(and (or a b) c)"); }
TEST(ParserLogic, FullPrecedenceChain) {
    EXPECT_EQ(parse_to_string("1 + 2 < 3 == true and false or nil"),
        "(or (and (== (< (+ 1 2) 3) true) false) nil)");
}

TEST(ParserErrors, DanglingAnd) { expect_parse_error("true and", "Expected expression", 1); }
TEST(ParserErrors, DanglingOr) { expect_parse_error("or true", "Expected expression", 1); }
TEST(ParserErrors, DanglingLess) { expect_parse_error("1 <", "Expected expression", 1); }