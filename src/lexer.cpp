#include "lexer.h"

#include <utility>

Lexer::Lexer(std::string source) : _source(std::move(source)) {}

char Lexer::advance() {
	char c = _source[current];
	current++;
	return c;
}

Token Lexer::next_token() {
	start = current;
	if (is_at_end()) return make_token(TokenType::EndOfFile);
	char c = advance();

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
	}
	return error_token("Unexpected character.");
}
 
char Lexer::peek() const {
	if (is_at_end()) return '\0';
	return _source[current];
}

char Lexer::peek_next() const {

}

bool Lexer::match(char c) {

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
