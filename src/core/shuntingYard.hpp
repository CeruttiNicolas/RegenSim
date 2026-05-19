#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cctype>
#include <cmath>
#include <stdexcept>

enum class TokenType {
	Number,
	Variable,
	Operator,
	Function,
	LeftPar,
	RightPar
};

struct Token {
	TokenType type;
	double value;
	std::string name;
};

class ExpressionEvaluator {
public:
	ExpressionEvaluator(const std::string& expr);
	double evaluate(double x) const;

private:
	std::vector<Token> rpn;

	int getPrecedence(const std::string& op) const;
	bool isRightAssociative(const std::string& op) const;
};