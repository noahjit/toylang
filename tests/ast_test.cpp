#include "lexer.h"
#include <gtest/gtest.h>
#include "ast.h"

TEST(ASTtest, HandBuiltOnePlusTwoTimesThree) {
	Token one{ TokenType::Number, "1", 1 };
	Token two{ TokenType::Number, "2", 1 };
	Token three{ TokenType::Number, "3", 1 };
	Token plus{ TokenType::Plus, "+", 1 };
	Token times{ TokenType::Star, "*", 1 };

	auto product = std::make_unique<Binary>(std::make_unique<Literal>(two), times, std::make_unique<Literal>(three));

	auto expr = std::make_unique<Binary>(std::make_unique<Literal>(one), plus, std::move(product));

	auto* root = dynamic_cast<Binary*>(expr.get());
	ASSERT_NE(root, nullptr);
	EXPECT_EQ(root->op._type, TokenType::Plus);

	auto* right = dynamic_cast<Binary*>(root->right.get());
	ASSERT_NE(right, nullptr);
	EXPECT_EQ(right->op._type, TokenType::Star);
}