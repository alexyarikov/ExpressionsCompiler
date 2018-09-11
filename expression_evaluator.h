#pragma once
#include "common.h"
#include "expression_parser.h"

namespace Renaissance
{
class ExpressionEvaluator
{
public:
   ExpressionEvaluator() = default;
   ExpressionEvaluator(const ExpressionEvaluator&) = delete;
   ExpressionEvaluator(ExpressionEvaluator&&) = delete;
   ExpressionEvaluator& operator =(const ExpressionEvaluator&) = delete;
   ExpressionEvaluator& operator =(ExpressionEvaluator&&) = delete;
   ~ExpressionEvaluator() = default;

	bool Evaluate(const std::string& expression, const VariableValues& variable_values, bool& result);

private:
	enum class ExpressionType
	{
		Boolean,
		String,
		StringArray
	};

	struct ExpressionValue
	{
		ExpressionType _type;
		bool _bool_value;
		std::string _string_value;
		std::vector<std::string> _array_value;
	};

	ExpressionParser _parser;

	bool Evaluate(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
	bool EvaluateScalar(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
	bool EvaluateVariable(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
	bool EvaluateFunction(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
	bool EvaluateArray(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
	bool EvaluateOperator(const VariableValues& variable_values, const std::shared_ptr<ExpressionNode>& expression_node, ExpressionValue& expression_value) const;
};
}

