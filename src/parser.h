#pragma once

#include "lexer.h"
#include "ast.h"
#include <stdexcept>
#include <memory>
#include <string>
#include <string_view>

struct ParseError : std::runtime_error {
	ParseError(const std::string& message, int line) : std::runtime_error(message), line(line) {}
	int line;
};

class Parser {
public:
	explicit Parser(Lexer& lexer);
	std::unique_ptr<Expression> parse_expression();
private:
	Token advance();
	bool check(TokenType type) const;
	bool match(TokenType type);
	Token consume(TokenType type, std::string_view message);
	std::unique_ptr<Expression> primary();
	std::unique_ptr<Expression> unary();
	std::unique_ptr<Expression> factor();

	Lexer& lexer;
	Token current{ TokenType::EndOfFile, "", 1 };
	Token previous{ TokenType::EndOfFile, "", 1 };
};