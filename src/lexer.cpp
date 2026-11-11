#include "lexer.h"

#include <utility>
#include <unordered_map>
#include <iostream>

Lexer::Lexer(std::string source) : _source(std::move(source)) {}

TokenType Lexer::identifier_type() const {
	static const std::unordered_map<std::string_view, TokenType> keywords = {
		{ "and", TokenType::And },{ "or", TokenType::Or },{ "let", TokenType::Let },
		{ "fn", TokenType::Fn },{ "if", TokenType::If },{ "else", TokenType::Else },
		{ "while", TokenType::While },{ "return", TokenType::Return },{ "print", TokenType::Print },
		{ "true", TokenType::True },{ "false", TokenType::False },{ "nil", TokenType::Nil }
	};
	std::string_view sv(_source.data() + start, current - start);
	auto it = keywords.find(sv);

	if (it != keywords.end()) return it->second;
	return TokenType::Identifier;
}

char Lexer::advance() {
	char c = _source[current];
	current++;
	return c;
}

bool is_digit(char c) {
	return (c >= '0' && c <= '9');
}

bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_');
}

void Lexer::skip_whitespace() {
	for (;;) {
		char c = peek();

		switch (c) {
			case ' ':
			case '\t': 
			case '\r': advance(); break;
			case '\n': advance(); line_number++; break;
			case '/': 
				if (peek_next() == '/') {
					while (peek() != '\n' && !is_at_end()) {
						advance();
					}
				}
				else
					return;
				break;
			default: return;
		}
	}
}

Token Lexer::number() {
	while (is_digit(peek())) advance();

	if (peek() == '.' && is_digit(peek_next())) {
		advance();
		while (is_digit(peek())) advance();
	}
	return make_token(TokenType::Number);
}

Token Lexer::string_literal() {
	while (peek() != '"' && !is_at_end()) {
		if (peek() == '\n') line_number++;
		advance();
	}
	if (is_at_end()) return error_token("Unterminated string.");
	else {
		advance();
		return make_token(TokenType::String);
	}
}

Token Lexer::identifier() {
	while (is_alpha(peek()) || is_digit(peek())) advance();
	return make_token(identifier_type());
}

Token Lexer::next_token() {
	skip_whitespace();
	start = current;
	if (is_at_end()) return make_token(TokenType::EndOfFile);
	char c = advance();

	if (is_digit(c)) return number();
	if (is_alpha(c)) return identifier();
	if (c == '"') return string_literal();

	switch (c) {
		case '(': return make_token(TokenType::LeftParen);
		case ')': return make_token(TokenType::RightParen);
		case ';': return make_token(TokenType::Semicolon);
		case ',': return make_token(TokenType::Comma);
		case '+': return make_token(TokenType::Plus);
		case '-': return make_token(TokenType::Minus);
		case '*': return make_token(TokenType::Star);
		case '/': return make_token(TokenType::Slash);
		case '{': return make_token(TokenType::LeftBrace);
		case '}': return make_token(TokenType::RightBrace);

		case '!': return make_token(match('=') ? TokenType::BangEqual : TokenType::Bang);
		case '=': return make_token(match('=') ? TokenType::EqualEqual : TokenType::Equal);
		case '>': return make_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
		case '<': return make_token(match('=') ? TokenType::LessEqual : TokenType::Less);
	}
	return error_token("Unexpected character.");
}
 
char Lexer::peek() const {
	if (is_at_end()) return '\0';
	return _source[current];
}

char Lexer::peek_next() const {
	if (current + 1 >= _source.size()) return '\0';
	return _source[current + 1];
}

bool Lexer::match(char c) {
	if (is_at_end()) return false;
	if (c != peek()) return false;

	current++;
	return true;
}

bool Lexer::is_at_end() const {
	if (current == _source.size()) return true;
	return false;
}

Token Lexer::make_token(TokenType type) const {
	Token token = {type, std::string_view(_source.data() + start, current - start), line_number};
	return token;
}

Token Lexer::error_token(std::string_view message) const {
	Token token = { TokenType::Error, message, line_number };
	return token;
}
