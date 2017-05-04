#pragma once

class ExpressionsCompiler
{
public:
	ExpressionsCompiler();
	ExpressionsCompiler(const ExpressionsCompiler&) = delete;
	ExpressionsCompiler(ExpressionsCompiler&&) = delete;
	ExpressionsCompiler& operator =(const ExpressionsCompiler&) = delete;
	ExpressionsCompiler& operator =(ExpressionsCompiler&&) = delete;
	~ExpressionsCompiler();

	bool Parse(const std::string& strExpression);
	bool Evaluate(const std::unordered_map<std::string, std::string>& msg, bool& result);

private:
	enum class TokenType
	{
		ttFunc						= 0,
		ttFuncArgSep				= 1,
		ttLeftBracket				= 2,
		ttRightBracket				= 3,
		ttMsgField					= 4,
		ttNumber						= 5,
		ttString						= 6,
		ttArray						= 7,
		// operator enumerations are listed by their precedence
		ttOperatorLogicalOr		= 8,	// ||
		ttOperatorLogicalAnd		= 9,	// &&
		ttOperatorEqual			= 10,	// ==
		ttOperatorNotEqual		= 11,	// !=
		ttOperatorLess				= 12,	// <
		ttOperatorLessOrEqual	= 13,	// <=
		ttOperatorMore				= 14,	// >
		ttOperatorMoreOrEqual	= 15,	// >=
		ttOperatorFirst			= ttOperatorLogicalOr,
		ttOperatorLast				= ttOperatorMoreOrEqual
	};

	enum class VariableType { vtBool, vtString, vtNumber, vtArray };

	struct Token
	{
		TokenType enType;
		std::string::const_iterator itBegin;
		std::string::const_iterator itEnd;
	};

	struct Variable
	{
		VariableType enType;
		std::string strValue;
		int iValue;
		bool bValue;
	};

	std::string m_strExpression;
	std::string::const_iterator m_itCurrent;
	std::queue<Token> m_OutputQueue;
	std::stack<Token> m_OperatorStack;
	std::stack<Variable> m_EvaluationStack;

	void Clear();
	void SkipWhiteSpaces();
	void MoveToken();
	inline bool IsOperatorToken(const Token& token) const { return token.enType >= TokenType::ttOperatorFirst && token.enType <= TokenType::ttOperatorLast; }

	bool ReadToken(Token& token);
	bool ReadCharToken(const char ch, const TokenType enTokenType, Token& token);
	bool Read2CharsToken(const std::pair<char, char>& ch, const TokenType enTokenType, Token& token);
	bool ReadLeftBracket(Token& token);
	bool ReadRightBracket(Token& token);
	bool ReadFuncArgSep(Token& token);
	bool ReadOperator(Token& token);
	bool ReadNumber(Token& token);
	bool ReadString(Token& token);
	bool ReadFunc(Token& token);
	bool ReadMsgField(Token& token);
	bool ReadArray(Token& token);

	bool ProcessToken(const Token& token);
	bool ProcessScalar(const Token& token);
	bool ProcessFunc(const Token& token);
	bool ProcessFuncArgSep(const Token& token);
	bool ProcessLeftBracket(const Token& token);
	bool ProcessRightBracket(const Token& token);
	bool ProcessOperator(const Token& token);

	bool EvaluateNumber(const Token& token);
	bool EvaluateString(const Token& token);
	bool EvaluateMsgField(const Token& token, const std::unordered_map<std::string, std::string>& msg);
	bool EvaluateFunction(const Token& token);
	bool EvaluateOperator(const Token& token);

	template <typename T> bool EvaluateOperator(const TokenType enTokenType, const T& var1, const T& var2, bool& bResult) const
	{
		switch (enTokenType)
		{
		case TokenType::ttOperatorLogicalOr:
			bResult = (var1 || var2);
			return true;
		case TokenType::ttOperatorLogicalAnd:
			bResult = (var1 && var2);
			return true;
		case TokenType::ttOperatorEqual:
			bResult = (var1 == var2);
			return true;
		case TokenType::ttOperatorNotEqual:
			bResult = (var1 != var2);
			return true;
		case TokenType::ttOperatorLess:
			bResult = (var1 < var2);
			return true;
		case TokenType::ttOperatorLessOrEqual:
			bResult = (var1 <= var2);
			return true;
		case TokenType::ttOperatorMore:
			bResult = (var1 > var2);
			return true;
		case TokenType::ttOperatorMoreOrEqual:
			bResult = (var1 >= var2);
			return true;
		}
		return false;
	}

	template <> bool EvaluateOperator<std::string>(const TokenType enTokenType, const std::string& var1, const std::string& var2, bool& bResult) const
	{
		switch (enTokenType)
		{
		case TokenType::ttOperatorEqual:
			bResult = (var1 == var2);
			return true;
		case TokenType::ttOperatorNotEqual:
			bResult = (var1 != var2);
			return true;
		}
		return false;
	}
};
