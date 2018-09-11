#pragma once
#include <memory>
#include <string>
#include <stack>
#include <unordered_map>

namespace Renaissance
{
enum class TokenType
{
   Func                 = 0,  // e.g. SUBSTR{1, 3, VAR}
   ArgSep               = 1,  // ,
   LBracket             = 2,  // (
   RBracket             = 3,  // )
   Variable             = 4,  // e.g. IN.MT
   Scalar               = 5,  // e.g. 8 or "something"
   LSquareBracket       = 7,  // [
   RSquareBracket       = 8,  // ]
   LBrace               = 9,  // {
   RBrace               = 10, // }
   // operator enumerations are listed by their precedence
   OperatorLogicalOr    = 11, // ||
   OperatorLogicalAnd   = 12, // &&
   OperatorEqual        = 13, // ==
   OperatorNotEqual     = 14, // !=
   OperatorLess         = 15, // <
   OperatorLessOrEqual  = 16, // <=
   OperatorMore         = 17, // >
   OperatorMoreOrEqual  = 18, // >=
   OperatorFirst        = OperatorLogicalOr,
   OperatorLast         = OperatorMoreOrEqual
};

struct Token
{
   TokenType _type;
   std::string::const_iterator _begin;
   std::string::const_iterator _end;

   Token() = default;
   explicit Token(const TokenType& type) : _type(type) {}
   inline bool IsOperator() const noexcept { return _type >= TokenType::OperatorFirst && _type <= TokenType::OperatorLast; }
   std::string ToString() const
   {
      switch (_type)
      {
      case TokenType::ArgSep:
         return ",";
      case TokenType::LBracket:
         return "(";
      case TokenType::RBracket:
         return ")";
      case TokenType::Scalar:
      case TokenType::Variable:
      case TokenType::Func:
         return std::string(_begin, _end);
      case TokenType::LSquareBracket:
         return "ARRAY";
      case TokenType::RSquareBracket:
         return "]";
      case TokenType::LBrace:
         return "{";
      case TokenType::RBrace:
         return "}";
      case TokenType::OperatorLogicalOr:
         return "or";
      case TokenType::OperatorLogicalAnd:
         return "and";
      case TokenType::OperatorEqual:
         return "==";
      case TokenType::OperatorNotEqual:
         return "!=";
      case TokenType::OperatorLess:
         return "<";
      case TokenType::OperatorLessOrEqual:
         return "<=";
      case TokenType::OperatorMore:
         return ">";
      case TokenType::OperatorMoreOrEqual:
         return ">=";
      default:
         return "";
      }
   }
};

struct ExpressionNode
{
   Token _token;
   std::shared_ptr<ExpressionNode> _child;
   std::shared_ptr<ExpressionNode> _sibling;
   ExpressionNode() = default;
   explicit ExpressionNode(const Token& token) : _token(token) {}
};

typedef std::stack<std::shared_ptr<ExpressionNode>> ExpressionTree;
typedef std::unordered_map<std::string, std::string> VariableValues;
}

