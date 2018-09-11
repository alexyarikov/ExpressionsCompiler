#pragma once
#include <string>
#include <memory>
#include <stack>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "common.h"

// This is to parse an infix logical expression into the abstract syntax tree by calling ExpressionParser::Parse.
// Example of an expression:
// ((IN.CURRENCY != "985") && (IN.BIN_ISSUEING_COUNTRY == "616") && (IN.MT==1) && (SUBSTR{3,1,IN.TID} == ["9", "2", "L", "V", "U"]))
// At the moment the only supported function is SUBSTR, 1st arg - <from> 0-based position, 2nd arg - length of substring to take, 3rd arg - source string.
// Functions without arguments are not supported now.
// Beside of simple logical operators, a value might be compared with an array by using equal operator and it works like "IN" SQL operator.
// There are also variables, all variable values are passed via a hashtable into the ExpressionParser::Evaluate method.
using namespace boost::property_tree;

namespace Renaissance
{
class ExpressionParser
{
public:
   ExpressionParser() = default;
   ExpressionParser(const ExpressionParser&) = delete;
   ExpressionParser(ExpressionParser&&) = delete;
   ExpressionParser& operator =(const ExpressionParser&) = delete;
   ExpressionParser& operator =(ExpressionParser&&) = delete;
   ~ExpressionParser() = default;

   bool Parse(const std::string& expression, ExpressionTree& expression_tree);
   void PrintOutputTree() const;

private:
   std::string _expression;
   std::string::const_iterator _current;
   std::stack<Token> _operators;
   std::stack<uint16_t> _args_number;
   ExpressionTree _expression_tree;

   void Clear();
   void SkipWhiteSpaces();
   bool MoveToOutput(const uint16_t operands_number, const bool is_function);
   inline bool IsCurrentToken(const TokenType& token_type) const { return !_operators.empty() && _operators.top()._type == token_type; }

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

   bool CheckOutputNode(const std::shared_ptr<ExpressionNode>& node, const uint16_t operands_number) const;
   bool CheckFunctionNode(const std::shared_ptr<ExpressionNode>& node) const;
   bool CheckArrayNode(const std::shared_ptr<ExpressionNode>& node, const uint16_t operands_number) const;

   void PrintOutputTree(const std::shared_ptr<ExpressionNode>& node, const size_t level) const;
};
}

