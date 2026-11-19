#include "parser.h"

Parser::Parser(Lexer& lexer) : lexer(lexer) { advance(); }

Token Parser::advance() {
	previous = current;
	current = lexer.next_token();
	if (current._type == TokenType::Error) throw ParseError(std::string(current.lexeme), current.line_number);
	return previous;
}

bool Parser::check(TokenType type) const {
	return (current._type == type);
}

bool Parser::match(TokenType type) {
	if (check(type)) {
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(TokenType type, std::string_view message) {
	if (check(type)) return advance();
	else throw ParseError(std::string(message), current.line_number);
}

std::unique_ptr<Expression> Parser::primary() {
	switch (current._type) {
	case TokenType::Number:
	case TokenType::String:
	case TokenType::True:
	case TokenType::False:

	case TokenType::Nil: 
		return std::make_unique<Literal>(advance()); 

	case TokenType::LeftParen: {
		advance();
		auto expr = parse_expression();
		consume(TokenType::RightParen, "Expected ')' after expression.");
		return expr;
	}
	case TokenType::Identifier: {
		return std::make_unique<Variable>(advance());
	}
	default: throw ParseError(std::string("Expected expression."), current.line_number);
	}
}

std::unique_ptr<Expression> Parser::unary() {
	if (match(TokenType::Bang) || match(TokenType::Minus)) {
		Token op = previous;
		auto operand = unary();
		return std::make_unique<Unary>(op, std::move(operand));
	}
	return primary();
}

std::unique_ptr<Expression> Parser::parse_expression() {
	return logic_or();
}

std::unique_ptr<Expression> Parser::parse() {
	auto expr = parse_expression();
	consume(TokenType::EndOfFile, "Expected end of input.");
	return expr;
}

std::unique_ptr<Expression> Parser::factor() {
	std::unique_ptr<Expression> left = unary();

	while (match(TokenType::Star) || match(TokenType::Slash)) {
		Token op = previous;
		std::unique_ptr<Expression> right = unary();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}
	
	return left;
}

std::unique_ptr<Expression> Parser::term() {
	std::unique_ptr<Expression> left = factor();

	while (match(TokenType::Plus) || match(TokenType::Minus)) {
		Token op = previous;
		std::unique_ptr<Expression> right = factor();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::comparison() {
	std::unique_ptr<Expression> left = term();

	while (match(TokenType::Less) || match(TokenType::Greater) || match(TokenType::LessEqual) || match(TokenType::GreaterEqual)) {
		Token op = previous;
		std::unique_ptr<Expression> right = term();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::equality() {
	std::unique_ptr<Expression> left = comparison();

	while (match(TokenType::EqualEqual) || match(TokenType::BangEqual)) {
		Token op = previous;
		std::unique_ptr<Expression> right = comparison();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::logic_and() {
	std::unique_ptr<Expression> left = equality();

	while (match(TokenType::And)) {
		Token op = previous;
		std::unique_ptr<Expression> right = equality();
		left = std::make_unique<Logical>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::logic_or() {
	std::unique_ptr<Expression> left = logic_and();

	while (match(TokenType::Or)) {
		Token op = previous;
		std::unique_ptr<Expression> right = logic_and();
		left = std::make_unique<Logical>(std::move(left), op, std::move(right));
	}

	return left;
}