#include "core/shuntingYard.hpp"

ExpressionEvaluator::ExpressionEvaluator(const std::string& expr) {
	std::stack<Token> opStack;
	int i = 0;

	while (i < expr.length()) {
		if (isspace(expr[i])) {
			i++;
			continue;
		}

		// parse numbers
		if (isdigit(expr[i]) || expr[i] == '.') {
			std::string numStr;
			while (i < expr.length() && (
				isdigit(expr[i]) ||
				expr[i] == '.' ||
				expr[i] == 'e' || expr[i] == 'E' ||
				((expr[i] == '-' || expr[i] == '+') && !numStr.empty() && (numStr.back() == 'e' || numStr.back() == 'E'))
				)) {
				numStr += expr[i++];
			}
			rpn.push_back({ TokenType::Number, std::stod(numStr), "" });
		}

		// parse variables and functions
		else if (isalpha(expr[i])) {
			std::string name;
			while (i < expr.length() && isalnum(expr[i])) {
				name += expr[i++];
			}

			if (name == "x" || name == "T") {
				rpn.push_back({ TokenType::Variable, 0, name });
			}
			else {
				opStack.push({ TokenType::Function, 0, name });
			}
		}

		// parse left parentheses
		else if (expr[i] == '(') {
			opStack.push({ TokenType::LeftPar, 0, "(" });
			i++;
		}

		// parse right parentheses
		else if (expr[i] == ')') {
			while (!opStack.empty() && opStack.top().type != TokenType::LeftPar) {
				rpn.push_back(opStack.top());
				opStack.pop();
			}
			if (opStack.empty()) throw std::runtime_error("Mismatched parentheses");
			opStack.pop();

			if (!opStack.empty() && opStack.top().type == TokenType::Function) {
				rpn.push_back(opStack.top());
				opStack.pop();
			}
			i++;
		}

		// parse operators
		else {
			std::string op(1, expr[i]);
			while (!opStack.empty() && opStack.top().type == TokenType::Operator) {
				std::string topOp = opStack.top().name;
				if ((!isRightAssociative(op) && getPrecedence(op) <= getPrecedence(topOp)) ||
					(isRightAssociative(op) && getPrecedence(op) < getPrecedence(topOp))) {
					rpn.push_back(opStack.top());
					opStack.pop();
				}
				else {
					break;
				}
			}
			opStack.push({ TokenType::Operator, 0, op });
			i++;
		}
	}

	while (!opStack.empty()) {
		if (opStack.top().type == TokenType::LeftPar) {
			throw std::runtime_error("Mismatched parentheses");
		}
		rpn.push_back(opStack.top());
		opStack.pop();
	}
}

double ExpressionEvaluator::evaluate(double x) const {
	std::stack<double> evalStack;

	for (const auto& token : rpn) {
		if (token.type == TokenType::Number) {
			evalStack.push(token.value);
		}

		else if (token.type == TokenType::Variable) {
			evalStack.push(x);
		}

		// evaluate unary functions
		else if (token.type == TokenType::Function) {
			if (evalStack.empty()) throw std::runtime_error("Invalid expression: Missing function argument.");

			double arg = evalStack.top();
			evalStack.pop();

			if (token.name == "sin") evalStack.push(std::sin(arg));
			else if (token.name == "cos") evalStack.push(std::cos(arg));
			else if (token.name == "tan") evalStack.push(std::tan(arg));
			else if (token.name == "exp") evalStack.push(std::exp(arg));
			else if (token.name == "log") evalStack.push(std::log(arg));
			else if (token.name.length() > 3 && token.name.substr(0, 3) == "log") {
				try {
					double base = std::stod(token.name.substr(3));
					evalStack.push(std::log(arg) / std::log(base));
				}
				catch (const std::invalid_argument&) {
					throw std::runtime_error("Invalid log base in function: " + token.name);
				}
			}
			else throw std::runtime_error("Unknown function: " + token.name);
		}

		// evaluate binary operators
		else if (token.type == TokenType::Operator) {
			if (evalStack.size() < 2) throw std::runtime_error("Invalid expression.");

			double right = evalStack.top(); evalStack.pop();
			double left = evalStack.top(); evalStack.pop();

			if (token.name == "+") evalStack.push(left + right);
			else if (token.name == "-") evalStack.push(left - right);
			else if (token.name == "*") evalStack.push(left * right);
			else if (token.name == "/") {
				if (right == 0) throw std::runtime_error("Division by zero.");
				evalStack.push(left / right);
			}
			else if (token.name == "^") evalStack.push(std::pow(left, right));
		}
	}

	if (evalStack.size() != 1) throw std::runtime_error("Invalid expression evaluation.");
	return evalStack.top();
}

int ExpressionEvaluator::getPrecedence(const std::string& op) const {
	if (op == "+" || op == "-") return 1;
	if (op == "*" || op == "/") return 2;
	if (op == "^") return 3;
	return 0;
}

bool ExpressionEvaluator::isRightAssociative(const std::string& op) const {
	return op == "^";
}