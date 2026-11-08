#pragma once

#include <string_view>
#include <string>
#include <cstddef>

enum class TokenType {
	LeftParen, RightParen, LeftBrace, RightBrace, Comma, Semicolon, Plus, Minus, Star, Slash,

	Bang, BangEqual, Equal, EqualEqual, Less, LessEqual, Greater, GreaterEqual,

	Identifier, Number, String,

	And, Or, Let, Fn, If, Else, While, Return, Print, True, False, Nil,

	EndOfFile, Error
};

struct Token {
	TokenType _type;
	std::string_view lexeme;
	int line_number;
};

class Lexer {
	public:
		Lexer(std::string source);
		[[nodiscard]] Token next_token();
	private:
		char peek() const;  // returns the current character without consuming it
		char peek_next() const; // returns character 1 ahead
		char advance(); // consumes a character and returns it
		bool match(char c); // consumes the next character only if it equals the expected one, and tells me wether it did hence bool
		bool is_at_end() const; // bool that checks if we at the end of token
		Token make_token(TokenType type) const;
		Token error_token(std::string_view message) const;
		void skip_whitespace();

		std::string _source;
		int line_number = 1;
		std::size_t current = 0;
		std::size_t start = 0;
};