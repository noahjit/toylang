#pragma once

#include <memory>
#include <utility>
#include "lexer.h"

struct Expression {
	virtual ~Expression() = default; // without this virtual destructor , deleting a Binary through ExpressionPointer wont clean up properly.
};

struct Binary : Expression {
	Binary(std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right) : left(std::move(left)), op(op), right(std::move(right)) {} // move() transfers ownership into the node

	std::unique_ptr<Expression> left; // children are this type so each node owns its children.
	Token op;
	std::unique_ptr<Expression> right;
};

struct Literal : Expression {
	explicit Literal(Token value) : value(value) {}
	Token value;
};

struct Unary : Expression {
	Unary(Token op, std::unique_ptr<Expression> operand) : op(op), operand(std::move(operand)) {}
	Token op;
	std::unique_ptr<Expression> operand;
};