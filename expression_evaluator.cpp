#include "expression_evaluator.h"

namespace Renaissance
{
// evaluate an expression with variables values given in 'variable_values' parameter
// evaluation result will be returned in the output parameter 'result'
// method returns true if successful
bool ExpressionEvaluator::Evaluate(const std::string& expression, const VariableValues& variable_values, bool& result)
{
	result = false;

	// parse
	ExpressionTree expression_tree;
	if (!_parser.Parse(expression, expression_tree))
		return false;

   if (expression_tree.empty()) // treat empty tree as a true statement
	{
		result = true;
		return true;
	}

	// evaluate
	// create root expression value, it must have boolean type
	ExpressionValue value;
	bool res = Evaluate(variable_values, expression_tree.top(), value);

	if (res && value._type == ExpressionType::Boolean)
	{
		result = value._bool_value;
		return true;
	}
	else
		return false;
}

bool ExpressionEvaluator::Evaluate(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	if (!expression_node)
		return true;

	switch (expression_node->_token._type)
	{
		case TokenType::Scalar:
			return EvaluateScalar(variable_values, expression_node, expression_value);
		case TokenType::Variable:
			return EvaluateVariable(variable_values, expression_node, expression_value);
		case TokenType::Func:
			return EvaluateFunction(variable_values, expression_node, expression_value);
		case TokenType::LSquareBracket:
			return EvaluateArray(variable_values, expression_node, expression_value);
		default:
			if (expression_node->_token.IsOperator())
				return EvaluateOperator(variable_values, expression_node, expression_value);
	}
	return false;
}

bool ExpressionEvaluator::EvaluateScalar(const VariableValues& /*not used*/, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	std::string val = std::string(expression_node->_token._begin, expression_node->_token._end);
	expression_value._type = ExpressionType::String;
	expression_value._string_value = std::move(val);
	return true;
}

bool ExpressionEvaluator::EvaluateVariable(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	std::string val = std::string(expression_node->_token._begin, expression_node->_token._end);
	auto variable_value = variable_values.find(val);
	if (variable_value != variable_values.end())
	{
		expression_value._type = ExpressionType::String;
		expression_value._string_value = variable_value->second;
		return true;
	}
	return false; // variable is mentioned in the expression but no corresponding value is passed
}

bool ExpressionEvaluator::EvaluateFunction(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	// assuming here that the only supported function is substring and it has 3 arguments
	if (!expression_node->_child || !expression_node->_child->_sibling || !expression_node->_child->_sibling->_sibling)
		return false; // no corresponding nodes for function arguments in the syntax tree

	// retrieve and evaluate 3 function arguments
	ExpressionValue arg1;
	if (!Evaluate(variable_values, expression_node->_child, arg1) || arg1._type != ExpressionType::String)
		return false;

	ExpressionValue arg2;
	if (!Evaluate(variable_values, expression_node->_child->_sibling, arg2) || arg2._type != ExpressionType::String)
		return false;

	ExpressionValue arg3;
	if (!Evaluate(variable_values, expression_node->_child->_sibling->_sibling, arg3) || arg3._type != ExpressionType::String)
		return false;

	// evaluate function
	expression_value._type = ExpressionType::String;
	try
	{
		expression_value._string_value = arg1._string_value.substr(std::stoi(arg2._string_value), std::stoi(arg3._string_value));
	}
	catch (const std::out_of_range&)
	{
		expression_value._string_value.clear();
	}
	return true;
}

bool ExpressionEvaluator::EvaluateArray(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	if (!expression_node || !expression_node->_child) // at least one array item should exist in the syntax tree
		return false;

	// evaluate first array item
	ExpressionValue first_array_item;
	if (!Evaluate(variable_values, expression_node->_child, first_array_item))
		return false;

	expression_value._type = ExpressionType::StringArray;
	expression_value._array_value.push_back(std::move(first_array_item._string_value));

	auto node = expression_node->_child->_sibling;
	while (node)
	{
		ExpressionValue next_array_item;
		if (!Evaluate(variable_values, node, next_array_item) || next_array_item._type != first_array_item._type)
			return false;
		expression_value._array_value.push_back(std::move(next_array_item._string_value));
		node = node->_sibling;
	}
	return true;
}

bool ExpressionEvaluator::EvaluateOperator(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const
{
	// two operator arguments must exist
	if (!expression_node->_child || !expression_node->_child->_sibling)
		return false;

	// retrieve and evaluate 2 operator arguments
	ExpressionValue arg1;
	if (!Evaluate(variable_values, expression_node->_child, arg1))
		return false;

	ExpressionValue arg2;
	if (!Evaluate(variable_values, expression_node->_child->_sibling, arg2))
		return false;

	// argument types must be the same or there is comparison with array
	if (arg1._type != arg2._type && (arg1._type != ExpressionType::String || arg2._type != ExpressionType::StringArray))
		return false;

	expression_value._type = ExpressionType::Boolean;

	switch (expression_node->_token._type)
	{
		case TokenType::OperatorLogicalOr:
			expression_value._bool_value = (arg1._bool_value || arg2._bool_value);
			break;
		case TokenType::OperatorLogicalAnd:
			expression_value._bool_value = (arg1._bool_value && arg2._bool_value);
			break;
		case TokenType::OperatorEqual:
			if (arg2._type == ExpressionType::StringArray)
				expression_value._bool_value = (std::find(arg2._array_value.cbegin(), arg2._array_value.cend(), arg1._string_value) != arg2._array_value.cend());
			else if (arg1._type == ExpressionType::Boolean)
				expression_value._bool_value = (arg1._bool_value == arg2._bool_value);
			else
				expression_value._bool_value = (arg1._string_value == arg2._string_value);
			break;
		case TokenType::OperatorNotEqual:
			if (arg2._type == ExpressionType::StringArray)
				expression_value._bool_value = (std::find(arg2._array_value.cbegin(), arg2._array_value.cend(), arg1._string_value) == arg2._array_value.cend());
			else if (arg1._type == ExpressionType::Boolean)
				expression_value._bool_value = (arg1._bool_value != arg2._bool_value);
			else
				expression_value._bool_value = (arg1._string_value != arg2._string_value);
			break;
		case TokenType::OperatorLess:
			expression_value._bool_value = (arg1._bool_value < arg2._bool_value);
			break;
		case TokenType::OperatorLessOrEqual:
			expression_value._bool_value = (arg1._bool_value <= arg2._bool_value);
			break;
		case TokenType::OperatorMore:
			expression_value._bool_value = (arg1._bool_value > arg2._bool_value);
			break;
		case TokenType::OperatorMoreOrEqual:
			expression_value._bool_value = (arg1._bool_value >= arg2._bool_value);
			break;
		default:
			return false;
	}

	return true;
}
}

