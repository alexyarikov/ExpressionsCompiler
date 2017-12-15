#pragma once
#include <string>
#include <memory>
#include <stack>
#include <unordered_map>

// This is to parse an infix logical expression into the abstract syntax tree by calling ExpressionCompiler::Parse.
// Example of an expression:
// ((SUBSTR{4, 2, IN.TID} == "IC") && (IN.MT == [1, 2, 4, 5]))
// At the moment the only supported function is SUBSTR, 1st arg - <from> 0-based position, 2nd arg - length of substring to take, 3rd arg - source string.
// Functions without arguments are not supported now.
// Beside of simple logical operators, a value might be compared with an array by using equal operator and it works like "IN" SQL operator.
// There are also variables, all variable values are passed via a hashtable into the ExpressionCompiler::Evaluate method.
class ExpressionCompiler
{
public:
	ExpressionCompiler();
	ExpressionCompiler(const ExpressionCompiler&) = delete;
	ExpressionCompiler(ExpressionCompiler&&) = delete;
	ExpressionCompiler& operator =(const ExpressionCompiler&) = delete;
	ExpressionCompiler& operator =(ExpressionCompiler&&) = delete;
	~ExpressionCompiler();

	bool Parse(const std::string& strExpression);
	bool Evaluate(const std::unordered_map<std::string, std::string>& variable_values, bool& result);
	bool TransformIntoPrairieRule(/*reference to a json object or json string*/) const;

private:
	enum class TokenType
	{
		Func						= 0,	// e.g. SUBSTR{1, 3, VAR}
		ArgSep					= 1,	// ,
		LBracket					= 2,	// (
		RBracket					= 3,	// )
		Variable					= 4,	// e.g. IN.MT
		Number					= 5,	// e.g. 8
		String					= 6,	// e.g. "something"
		LSquareBracket			= 7,	// [
		RSquareBracket			= 8,	// ]
		LBrace					= 9,	// {
		RBrace					= 10, // }
		// operator enumerations are listed by their precedence
		OperatorLogicalOr		= 11,	// ||
		OperatorLogicalAnd	= 12,	// &&
		OperatorEqual			= 13,	// ==
		OperatorNotEqual		= 14,	// !=
		OperatorLess			= 15,	// <
		OperatorLessOrEqual	= 16,	// <=
		OperatorMore			= 17,	// >
		OperatorMoreOrEqual	= 18,	// >=
		OperatorFirst			= OperatorLogicalOr,
		OperatorLast			= OperatorMoreOrEqual
	};

	struct Token
	{
		TokenType _type;
		std::string::const_iterator _begin;
		std::string::const_iterator _end;
		Token() = default;
		explicit Token(const TokenType& type) : _type(type) {}
	};

	struct ExpressionNode
	{
	   Token _token;
	   std::shared_ptr<ExpressionNode> _child;
	   std::shared_ptr<ExpressionNode> _sibling;
		ExpressionNode() = default;
	   ExpressionNode(const Token& token, const std::shared_ptr<ExpressionNode>& child = nullptr, const std::shared_ptr<ExpressionNode>& sibling = nullptr)
	      : _token(token), _child(child), _sibling(sibling) {}
	};

	std::string _expression;
	std::string::const_iterator _current;
	std::stack<std::shared_ptr<ExpressionNode>> _output;
	std::stack<Token> _operators;
	std::stack<uint16_t> _args_number;

	void Clear();
	void SkipWhiteSpaces();
	bool MoveToOutput(const uint16_t operands_number, const bool is_function = false);
	inline bool IsCurrentToken(const TokenType& token_type) const { return !_operators.empty() && _operators.top()._type == token_type; }
	inline bool IsOperatorToken(const Token& token) const { return token._type >= TokenType::OperatorFirst && token._type <= TokenType::OperatorLast; }

	bool ReadToken(Token& token);
	bool ReadCharToken(const char ch, const TokenType enTokenType, Token& token);
	bool Read2CharsToken(const std::pair<char, char>& ch, const TokenType enTokenType, Token& token);
	bool ReadLBracket(Token& token);
	bool ReadRBracket(Token& token);
	bool ReadArgSep(Token& token);
	bool ReadOperator(Token& token);
	bool ReadNumber(Token& token);
	bool ReadString(Token& token);
	bool ReadFunctionOrVariable(Token& token);
	bool ReadLSquareBracket(Token& token);
	bool ReadRSquareBracket(Token& token);
	bool ReadLBrace(Token& token);
	bool ReadRBrace(Token& token);

	bool ProcessToken(const Token& token);
	bool ProcessScalar(const Token& token);
	bool ProcessArgSep(const Token& token);
	bool ProcessFunc(const Token& token);
	bool ProcessLBracket(const Token& token);
	bool ProcessRBracket(const Token& token);
	bool ProcessOperator(const Token& token);
	bool ProcessLSquareBracket(const Token& token);
	bool ProcessRSquareBracket(const Token& token);
	bool ProcessLBrace(const Token& token);
	bool ProcessRBrace(const Token& token);
};

