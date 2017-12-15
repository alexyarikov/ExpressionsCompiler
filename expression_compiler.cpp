#include "expression_compiler.h"
#include <algorithm>

ExpressionCompiler::ExpressionCompiler()
{
}

ExpressionCompiler::~ExpressionCompiler()
{
}

bool ExpressionCompiler::Parse(const std::string& expression)
{
	if (expression.cbegin() == expression.cend())
		return false;

	Clear();

	_expression = "(" + expression + ")";
	_current = _expression.cbegin();

	Token token;
	while (ReadToken(token))
	{
		if (!ProcessToken(token))
			return false;
	}

	while (!_operators.empty())
	{
		if (_operators.top()._type == TokenType::LBracket)
			return false;
		if (!MoveToOutput(2))
			return false;
		_operators.pop();
	}

	return true;
}

bool ExpressionCompiler::Evaluate(const std::unordered_map<std::string, std::string>& variable_values, bool& result)
{
	result = false;

	if (_output.empty()) // syntax tree is not built, nothing to do
		return false;

	// TODO: here we should evaluate the expression:
	// walk through the syntax tree and evaluate every node

	return false;
}

bool ExpressionCompiler::TransformIntoPrairieRule(/*reference to a json object or json string*/) const
{
	if (_output.empty()) // syntax tree is not built, nothing to do
		return false;

	// TODO: here we should walk through the syntax tree and transform every tree node into a prairie rule json node
	// as a result, we could get a json object (hence we should have some json parser), or just a json string

	return false;
}

void ExpressionCompiler::Clear()
{
	while (!_operators.empty())
		_operators.pop();
	while (!_output.empty())
		_output.pop();
	while (!_args_number.empty())
		_args_number.pop();
}

void ExpressionCompiler::SkipWhiteSpaces()
{
	while (_current != _expression.cend() && isspace(*_current))
		++_current;
}

// retrieve specified number of operands from the tree and build a new tree node
bool ExpressionCompiler::MoveToOutput(const uint16_t operands_number, const bool is_function/*=false*/)
{
	if (operands_number == 0 || _output.empty())
		return false;

	// create a parent tree node, add the first child
	auto parent_node = std::make_shared<ExpressionNode>();

	// for all token types we retrieve a token from current operator on stack
	// except for the functions - here we can't do it as current operator on stack is left brace, so we do it later
	if (!is_function)
	{
		if (_operators.empty())
			return false;
		parent_node->_token = _operators.top();
	}

	auto arg_node = parent_node->_child = _output.top();
	_output.pop();

	// retrieve other operands, add them into the tree 
	for (uint16_t i = 0; i < operands_number - 1; i++)
	{
		if (_output.empty())
			return false; // some operands are missed
		arg_node->_sibling = _output.top();
		arg_node = arg_node->_sibling;
		_output.pop();
	}

	_output.push(parent_node);
	return true;
}

bool ExpressionCompiler::ReadToken(Token& token)
{
	if (_current == _expression.cend())
		return false;

	SkipWhiteSpaces();

	if (ReadLBracket(token))
		return true;

	if (ReadRBracket(token))
		return true;

	if (ReadOperator(token))
		return true;

	if (ReadArgSep(token))
		return true;

	if (ReadNumber(token))
		return true;

	if (ReadString(token))
		return true;

	if (ReadFunctionOrVariable(token))
		return true;

	if (ReadLSquareBracket(token))
		return true;

	if (ReadRSquareBracket(token))
		return true;

	if (ReadLBrace(token))
		return true;

	if (ReadRBrace(token))
		return true;

	return false;
}

bool ExpressionCompiler::ReadCharToken(const char ch, const TokenType enTokenType, Token& token)
{
	if (*_current == ch)
	{
		token._type = enTokenType;
		++_current;
		return true;
	}
	else
		return false;
}

bool ExpressionCompiler::Read2CharsToken(const std::pair<char, char>& ch, const TokenType enTokenType, Token& token)
{
	if (*_current == ch.first && *(_current + 1) == ch.second)
	{
		token._type = enTokenType;
		_current += 2;
		return true;
	}
	else
		return false;
}

bool ExpressionCompiler::ReadLBracket(Token& token)
{
	return ReadCharToken('(', TokenType::LBracket, token);
}

bool ExpressionCompiler::ReadRBracket(Token& token)
{
	return ReadCharToken(')', TokenType::RBracket, token);
}

bool ExpressionCompiler::ReadArgSep(Token& token)
{
	return ReadCharToken(',', TokenType::ArgSep, token);
}

bool ExpressionCompiler::ReadOperator(Token& token)
{
	if (ReadCharToken('<', TokenType::OperatorLess, token) ||
		 ReadCharToken('>', TokenType::OperatorMore, token) ||
		 Read2CharsToken(std::make_pair('<', '='), TokenType::OperatorLessOrEqual, token) ||
		 Read2CharsToken(std::make_pair('>', '='), TokenType::OperatorMoreOrEqual, token) ||
		 Read2CharsToken(std::make_pair('=', '='), TokenType::OperatorEqual, token)       ||
		 Read2CharsToken(std::make_pair('!', '='), TokenType::OperatorNotEqual, token)    ||
		 Read2CharsToken(std::make_pair('&', '&'), TokenType::OperatorLogicalAnd, token)  ||
		 Read2CharsToken(std::make_pair('|', '|'), TokenType::OperatorLogicalOr, token))
	{
		return true;
	}
	return false;
}

bool ExpressionCompiler::ReadNumber(Token& token)
{
	if (std::isdigit(*_current))
	{
		// this is a number, search for its end
		auto it = std::find_if(_current + 1, _expression.cend(), [](const char ch) { return !std::isdigit(ch); });
		if (it != _expression.cend())
		{
			token._type      = TokenType::Number;
			token._begin = _current;
			token._end   = it;
			_current     = it;
			return true;
		}
	}

	return false;
}

bool ExpressionCompiler::ReadString(Token& token)
{
	if (*_current == '\"')
	{
		// this is a string, search for its end
		auto it = std::find(_current + 1, _expression.cend(), '\"');
		if (it != _expression.cend())
		{
			token._type = TokenType::String;
			token._begin = _current + 1;
			token._end = it;
			_current = it + 1;
			return true;
		}
	}

	return false;
}

bool ExpressionCompiler::ReadFunctionOrVariable(Token& token)
{
	// figure out whether token is function or variable
	// function name contains alphanumeric characters + '{', variable - without a brace in the end
	// either way first character should be a letter
	if (std::isalpha(*_current))
	{
		// search for the token end - any non-alpha character
		auto it = std::find_if(_current + 1, _expression.cend(), [](const char ch) { return !std::isalnum(ch) && ch != '.' && ch != '_'; });
		if (it != _expression.cend())
		{
			// function should have a brace after the name
			if (it != _expression.cend() && *it == '{')
				token._type = TokenType::Func;
			else
				token._type = TokenType::Variable;

			token._begin = _current;
			token._end   = it;
			_current     = it;
			return true;
		}
	}
	return false;
}

bool ExpressionCompiler::ReadLSquareBracket(Token& token)
{
	return ReadCharToken('[', TokenType::LSquareBracket, token);
}

bool ExpressionCompiler::ReadRSquareBracket(Token& token)
{
	return ReadCharToken(']', TokenType::RSquareBracket, token);
}

bool ExpressionCompiler::ReadLBrace(Token& token)
{
	return ReadCharToken('{', TokenType::LBrace, token);
}

bool ExpressionCompiler::ReadRBrace(Token& token)
{
	return ReadCharToken('}', TokenType::RBrace, token);
}

bool ExpressionCompiler::ProcessToken(const Token& token)
{
	switch (token._type)
	{
	case TokenType::Number:
	case TokenType::String:
	case TokenType::Variable:
		return ProcessScalar(token);
	case TokenType::ArgSep:
		return ProcessArgSep(token);
	case TokenType::LBracket:
		return ProcessLBracket(token);
	case TokenType::LSquareBracket:
		return ProcessLSquareBracket(token);
	case TokenType::LBrace:
		return ProcessLBrace(token);
	case TokenType::Func:
		return ProcessFunc(token);
	case TokenType::RBracket:
		return ProcessRBracket(token);
	case TokenType::RSquareBracket:
		return ProcessRSquareBracket(token);
	case TokenType::RBrace:
		return ProcessRBrace(token);
	default:
		if (IsOperatorToken(token))
			return ProcessOperator(token);
	}
	return false; // unknown token
}

bool ExpressionCompiler::ProcessScalar(const Token& token)
{
	_output.push(std::make_shared<ExpressionNode>(token));
	return true;
}

bool ExpressionCompiler::ProcessArgSep(const Token& token)
{
	// it could belong to a function or an array, otherwise - parsing error
	if (!_args_number.empty() &&
		(IsCurrentToken(TokenType::LBrace) ||
		 IsCurrentToken(TokenType::LSquareBracket)))
	{
		_args_number.top()++;
		return true;
	}
	else
		return false;
}

bool ExpressionCompiler::ProcessFunc(const Token& token)
{
	_operators.push(token);
	return true;
}

bool ExpressionCompiler::ProcessLBracket(const Token& token)
{
	_operators.push(token);
	return true;
}

bool ExpressionCompiler::ProcessRBracket(const Token& token)
{
	while (!_operators.empty() && !IsCurrentToken(TokenType::LBracket))
	{
		if (!MoveToOutput(2))
			return false;
		_operators.pop();
	}

	// empty operator stack means that there was no pair left bracket token, so this is a parsing error
	if (_operators.empty())
		return false;

	_operators.pop(); // pop left bracket
	return true;
}

bool ExpressionCompiler::ProcessOperator(const Token& token)
{
	// process all previous operators with more priority
	while (!_operators.empty() &&
			 IsOperatorToken(_operators.top()) &&
			 token._type <= _operators.top()._type)
	{
		if (!MoveToOutput(2))
			return false;
		_operators.pop();
	}

	_operators.push(token);
	return true;
}

bool ExpressionCompiler::ProcessLSquareBracket(const Token& token)
{
	_operators.push(token);
	_args_number.push(1);
	return true;
}

bool ExpressionCompiler::ProcessRSquareBracket(const Token& token)
{
	if (!IsCurrentToken(TokenType::LSquareBracket) || _args_number.empty())
		return false;

	if (!MoveToOutput(_args_number.top()))
		return false;

	_operators.pop();
	_args_number.pop();
	return true;
}

bool ExpressionCompiler::ProcessLBrace(const Token& token)
{
	// this left brace should belong to the function, otherwise - parsing error
	if (IsCurrentToken(TokenType::Func))
	{
		_operators.push(token);
		// store function arguments number
		// function without arguments are not supported so far,
		// this special case will need adding a fake void argument or something like that
		_args_number.push(1);
		return true;
	}
	else
		return false;
}

bool ExpressionCompiler::ProcessRBrace(const Token& token)
{
	if (!IsCurrentToken(TokenType::LBrace) || _args_number.empty())
		return false;

	if (!MoveToOutput(_args_number.top(), true))
		return false;

	_operators.pop(); // pop left brace

	// check and pop function token, update function node token in the tree
	if (IsCurrentToken(TokenType::Func) && !_output.empty() && _output.top())
	{
		_output.top()->_token = _operators.top();
		_operators.pop();
		return true;
	}
	else
		return false;
}

