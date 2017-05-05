#include "stdafx.h"
#include "ExpressionsCompiler.h"

ExpressionsCompiler::ExpressionsCompiler()
{
}

ExpressionsCompiler::~ExpressionsCompiler()
{
}

bool ExpressionsCompiler::Parse(const std::string& strExpression)
{
	if (strExpression.cbegin() == strExpression.cend())
		return false;

	Clear();

	m_strExpression = "(" + strExpression + ")";
	m_itCurrent = m_strExpression.cbegin();

	Token token;
	while (ReadToken(token))
	{
		if (!ProcessToken(token))
			return false;
	}

	while (!m_OperatorStack.empty())
	{
		if (m_OperatorStack.top().enType == TokenType::ttLBracket)
			return false;
		MoveToken();
	}

	return true;
}

bool ExpressionsCompiler::Evaluate(const std::unordered_map<std::string, std::string>& msg, bool& result)
{
	bool bResult = false;

	if (m_OutputQueue.empty())
		return bResult;

	while (!m_EvaluationStack.empty())
		m_EvaluationStack.pop();

	while (!m_OutputQueue.empty())
	{
		const Token& token = m_OutputQueue.front();

		switch (token.enType)
		{
		case TokenType::ttNumber:
			bResult = EvaluateNumber(token);
			break;
		case TokenType::ttString:
			bResult = EvaluateString(token);
			break;
		case TokenType::ttMsgField:
			bResult = EvaluateMsgField(token, msg);
			break;
		case TokenType::ttFunc:
			bResult = EvaluateFunction(token);
			break;
		default:
			if (IsOperatorToken(token))
				EvaluateOperator(token);
		}
		m_OutputQueue.pop();
	}

	// stack should contain the expression result
	if (m_EvaluationStack.size() == 1)
	{
		Variable var = m_EvaluationStack.top();
		if (var.enType == VariableType::vtBool)
		{
			result = var.bValue;
			return true;
		}
	}

	return false;
}

void ExpressionsCompiler::Clear()
{
	while (!m_OperatorStack.empty())
		m_OperatorStack.pop();
	while (!m_OutputQueue.empty())
		m_OutputQueue.pop();
}

void ExpressionsCompiler::SkipWhiteSpaces()
{
	while (m_itCurrent != m_strExpression.cend() && isspace(*m_itCurrent))
		m_itCurrent++;
}

void ExpressionsCompiler::MoveToken()
{
	m_OutputQueue.push(std::move(m_OperatorStack.top()));
	m_OperatorStack.pop();
}

bool ExpressionsCompiler::ReadToken(Token& token)
{
	if (m_itCurrent == m_strExpression.cend())
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

	if (ReadFunc(token))
		return true;

	if (ReadMsgField(token))
		return true;

	if (ReadLSquareBracket(token))
		return true;

	if (ReadRSquareBracket(token))
		return true;

	return false;
}

bool ExpressionsCompiler::ReadCharToken(const char ch, const TokenType enTokenType, Token& token)
{
	if (*m_itCurrent == ch)
	{
		token.enType = enTokenType;
		m_itCurrent++;
		return true;
	}
	else
		return false;
}

bool ExpressionsCompiler::Read2CharsToken(const std::pair<char, char>& ch, const TokenType enTokenType, Token& token)
{
	if (*m_itCurrent == ch.first && *(m_itCurrent + 1) == ch.second)
	{
		token.enType = enTokenType;
		m_itCurrent += 2;
		return true;
	}
	else
		return false;
}

bool ExpressionsCompiler::ReadLBracket(Token& token)
{
	return ReadCharToken('(', TokenType::ttLBracket, token);
}

bool ExpressionsCompiler::ReadRBracket(Token& token)
{
	return ReadCharToken(')', TokenType::ttRBracket, token);
}

bool ExpressionsCompiler::ReadArgSep(Token& token)
{
	return ReadCharToken(',', TokenType::ttArgSep, token);
}

bool ExpressionsCompiler::ReadOperator(Token& token)
{
	if (ReadCharToken('<', TokenType::ttOperatorLess, token) ||
		 ReadCharToken('>', TokenType::ttOperatorMore, token) ||
		 Read2CharsToken(std::make_pair('<', '='), TokenType::ttOperatorLessOrEqual, token) ||
		 Read2CharsToken(std::make_pair('>', '='), TokenType::ttOperatorMoreOrEqual, token) ||
		 Read2CharsToken(std::make_pair('=', '='), TokenType::ttOperatorEqual, token)       ||
		 Read2CharsToken(std::make_pair('!', '='), TokenType::ttOperatorNotEqual, token)    ||
		 Read2CharsToken(std::make_pair('&', '&'), TokenType::ttOperatorLogicalAnd, token)  ||
		 Read2CharsToken(std::make_pair('|', '|'), TokenType::ttOperatorLogicalOr, token))
	{
		return true;
	}
	return false;
}

bool ExpressionsCompiler::ReadNumber(Token& token)
{
	if (std::isdigit(*m_itCurrent))
	{
		// this is a number, search for its end
		auto it = std::find_if(m_itCurrent + 1, m_strExpression.cend(), [](const char ch) { return !std::isdigit(ch); });
		if (it != m_strExpression.cend())
		{
			token.enType = TokenType::ttNumber;
			token.itBegin = m_itCurrent;
			token.itEnd = it;
			m_itCurrent = it;
			return true;
		}
	}

	return false;
}

bool ExpressionsCompiler::ReadString(Token& token)
{
	if (*m_itCurrent == '\"')
	{
		// this is a string, search for its end
		auto it = std::find(m_itCurrent + 1, m_strExpression.cend(), '\"');
		if (it != m_strExpression.cend())
		{
			token.enType = TokenType::ttString;
			token.itBegin = m_itCurrent + 1;
			token.itEnd = it;
			m_itCurrent = it + 1;
			return true;
		}
	}

	return false;
}

bool ExpressionsCompiler::ReadFunc(Token& token)
{
	const std::string strFuncName = "SUBSTR";
	if (std::equal(m_itCurrent, m_itCurrent + strFuncName.size() , strFuncName.cbegin(), strFuncName.cend()))
	{
		token.enType = TokenType::ttFunc;
		token.itBegin = m_itCurrent;
		m_itCurrent += strFuncName.size();
		token.itEnd = m_itCurrent;
		return true;
	}

	return false;
}

bool ExpressionsCompiler::ReadMsgField(Token& token)
{
	// first character should be a letter
	if (std::isalpha(*m_itCurrent))
	{
		// this is message field, search for its end - any non-alphanumeric character
		auto it = std::find_if(m_itCurrent + 1, m_strExpression.cend(), [](const char ch) { return !std::isalnum(ch) && ch != '.' && ch != '_'; });
		if (it != m_strExpression.cend())
		{
			token.enType = TokenType::ttMsgField;
			token.itBegin = m_itCurrent;
			token.itEnd = it;
			m_itCurrent = it;
			return true;
		}
	}

	return false;
}

bool ExpressionsCompiler::ReadLSquareBracket(Token& token)
{
	return ReadCharToken('[', TokenType::ttLBracket, token);
}

bool ExpressionsCompiler::ReadRSquareBracket(Token& token)
{
	return ReadCharToken(']', TokenType::ttLBracket, token);
}

bool ExpressionsCompiler::ProcessToken(const Token& token)
{
	switch (token.enType)
	{
	case TokenType::ttNumber:
	case TokenType::ttString:
	case TokenType::ttMsgField:
		return ProcessScalar(token);
	case TokenType::ttFunc:
		return ProcessFunc(token);
	case TokenType::ttArgSep:
		return ProcessArgSep(token);
	case TokenType::ttLBracket:
		return ProcessLBracket(token);
	case TokenType::ttRBracket:
		return ProcessRBracket(token);
	case TokenType::ttLSquareBracket:
		return ProcessLSquareBracket(token);
	case TokenType::ttRSquareBracket:
		return ProcessRSquareBracket(token);
	default:
		if (IsOperatorToken(token))
			return ProcessOperator(token);
	}
	return false; // unknown token
}

bool ExpressionsCompiler::ProcessScalar(const Token& token)
{
	m_OutputQueue.push(token);
	return true;
}

bool ExpressionsCompiler::ProcessFunc(const Token& token)
{
	m_OperatorStack.push(token);
	return true;
}

bool ExpressionsCompiler::ProcessArgSep(const Token& token)
{
	while (!m_OperatorStack.empty() && m_OperatorStack.top().enType != TokenType::ttLBracket)
		MoveToken();

	// if operator stack is empty, it means that there was no function arguments begin token '{' or function arguments separator missed
	// anyway - this is an parsing error
	return !m_OperatorStack.empty();
}

bool ExpressionsCompiler::ProcessLBracket(const Token& token)
{
	m_OperatorStack.push(token);
	return true;
}

bool ExpressionsCompiler::ProcessRBracket(const Token& token)
{
	while (!m_OperatorStack.empty() && m_OperatorStack.top().enType != TokenType::ttLBracket)
		MoveToken();

	// if operator stack is empty, it means that there was no left bracket token, this is an parsing error
	if (m_OperatorStack.empty())
		return false;

	// pop left bracket
	m_OperatorStack.pop();

	// if this right bracket is related to function arguments add it into the output queue
	if (!m_OperatorStack.empty() && m_OperatorStack.top().enType == TokenType::ttFunc)
		MoveToken();

	return true;
}

bool ExpressionsCompiler::ProcessOperator(const Token& token)
{
	while (!m_OperatorStack.empty() &&
			 IsOperatorToken(m_OperatorStack.top()) &&
			 token.enType <= m_OperatorStack.top().enType)
	{
		MoveToken();
	}

	m_OperatorStack.push(token);

	return true;
}

bool ExpressionsCompiler::ProcessLSquareBracket(const Token& token)
{
	m_OperatorStack.push(token);
	return true;
}

bool ExpressionsCompiler::ProcessRSquareBracket(const Token& token)
{
	while (!m_OperatorStack.empty() && m_OperatorStack.top().enType != TokenType::ttLSquareBracket)
		MoveToken();

	// if operator stack is empty, it means that there was no left square bracket token, this is an parsing error
	if (m_OperatorStack.empty())
		return false;

	// pop left square bracket
	m_OperatorStack.pop();

	return true;
}

bool ExpressionsCompiler::EvaluateNumber(const Token& token)
{
	Variable var;
	var.enType = VariableType::vtNumber;
	var.iValue = std::stoi(std::string(token.itBegin, token.itEnd));

	m_EvaluationStack.push(var);
	return true;
}

bool ExpressionsCompiler::EvaluateString(const Token& token)
{
	Variable var;
	var.enType = VariableType::vtString;
	var.strValue = std::string(token.itBegin, token.itEnd);

	m_EvaluationStack.push(var);
	return true;
}

bool ExpressionsCompiler::EvaluateMsgField(const Token& token, const std::unordered_map<std::string, std::string>& msg)
{
	auto it = msg.find(std::string(token.itBegin, token.itEnd));
	if (it != msg.cend())
	{
		Variable var;
		var.enType = VariableType::vtString;
		var.strValue = it->second;

		m_EvaluationStack.push(var);
		return true;
	}
	else
		return false;
}

bool ExpressionsCompiler::EvaluateFunction(const Token&)
{
	// at the moment, only one function is supported - SUBSTR
	// so we expect 3 arguments on stack, pop them
	if (m_EvaluationStack.size() < 3)
		return false;

	Variable arg3 = std::move(m_EvaluationStack.top());
	m_EvaluationStack.pop();

	Variable arg2 = std::move(m_EvaluationStack.top());
	m_EvaluationStack.pop();

	Variable arg1 = std::move(m_EvaluationStack.top());
	m_EvaluationStack.pop();

	// check the arguments types
	if (arg1.enType != VariableType::vtNumber || arg2.enType != VariableType::vtNumber || arg3.enType != VariableType::vtString)
		return false;

	// evaluate the function, create string variable and push it to stack
	Variable var;
	var.enType = VariableType::vtString;
	var.strValue = std::move(arg3.strValue.substr(arg1.iValue, arg2.iValue));

	m_EvaluationStack.push(var);
	return true;
}

bool ExpressionsCompiler::EvaluateOperator(const Token& token)
{
	// expect 2 arguments on stack, pop them
	if (m_EvaluationStack.size() < 2)
		return false;

	Variable arg2 = std::move(m_EvaluationStack.top());
	m_EvaluationStack.pop();

	Variable arg1 = std::move(m_EvaluationStack.top());
	m_EvaluationStack.pop();

	// variables should have same type
	if (arg1.enType != arg2.enType)
		return false;

	Variable var;
	var.enType = VariableType::vtBool;

	bool bRes = false;

	switch (arg1.enType)
	{
	case VariableType::vtBool:
		bRes = EvaluateOperator<bool>(token.enType, arg1.bValue, arg2.bValue, var.bValue);
		break;
	case VariableType::vtString:
		bRes = EvaluateOperator<std::string>(token.enType, arg1.strValue, arg2.strValue, var.bValue);
		break;
	case VariableType::vtNumber:
		bRes = EvaluateOperator<int>(token.enType, arg1.iValue, arg2.iValue, var.bValue);
		break;
	}

	if (bRes)
		m_EvaluationStack.push(var);

	return bRes;
}
