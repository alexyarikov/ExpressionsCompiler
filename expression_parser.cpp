#include "expression_parser.h"
#include <algorithm>
#include <iostream>

namespace Renaissance
{
// parse the specified expression, returns true if successful
bool ExpressionParser::Parse(const std::string& expression, ExpressionTree& expression_tree)
{
   while (!expression_tree.empty())
      expression_tree.pop();

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
      if (!MoveToOutput(2, false))
         return false;
      _operators.pop();
   }

   expression_tree.swap(_expression_tree);
   return true;
}

// print the syntax tree to stdout
void ExpressionParser::PrintOutputTree() const
{
   if (!_expression_tree.empty())
      PrintOutputTree(static_cast<std::shared_ptr<ExpressionNode>>(_expression_tree.top()), 0);
}


// this is to clear everything to prepare a new parsing
void ExpressionParser::Clear()
{
   _expression.clear();
   while (!_expression_tree.empty()) _expression_tree.pop();
   while (!_operators.empty())       _operators.pop();
   while (!_args_number.empty())     _args_number.pop();
}

// skip whitespaces in the parsed string
void ExpressionParser::SkipWhiteSpaces()
{
   while (_current != _expression.cend() && isspace(*_current))
      ++_current;
}

// retrieve specified number of operands from the tree and build a new tree node
bool ExpressionParser::MoveToOutput(const uint16_t operands_number, const bool is_function)
{
   if (operands_number == 0 || _expression_tree.empty())
      return false;

   // create a parent tree node, add the first child
   auto parent_node = std::make_shared<ExpressionNode>();

   // for all token types we retrieve a token from current operator on stack
   // except for functions - here we can't do it as current operator on stack is left brace, so we do it later
   if (!is_function)
   {
      if (_operators.empty())
         return false;
      parent_node->_token = _operators.top();
   }

   parent_node->_child = _expression_tree.top();
   _expression_tree.pop();

   // retrieve other operands, add them into the tree in reversed order (to keep original arguments order in syntax tree)
   for (uint16_t i = 0; i < operands_number - 1; i++)
   {
      if (_expression_tree.empty())
         return false; // some operands are missed
      auto old_child = parent_node->_child;
      parent_node->_child = _expression_tree.top();
      parent_node->_child->_sibling = old_child;
      _expression_tree.pop();
   }

   _expression_tree.push(parent_node);

   return CheckOutputNode(parent_node, operands_number);
}

// reads a token from the parsed string into the 'token' output parameter
// moves current parsing position
// returns true if a token was parsed
bool ExpressionParser::ReadToken(Token& token)
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

// reads a single character token of specified token type into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadCharToken(const char ch, const TokenType enTokenType, Token& token)
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

// reads a two chars token of specified token type into the 'token' output parameter
// returns true if successful
bool ExpressionParser::Read2CharsToken(const std::pair<char, char>& ch, const TokenType enTokenType, Token& token)
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

// reads a left bracket token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadLBracket(Token& token)
{
   return ReadCharToken('(', TokenType::LBracket, token);
}

// reads an right bracket token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadRBracket(Token& token)
{
   return ReadCharToken(')', TokenType::RBracket, token);
}

// reads an argument separator into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadArgSep(Token& token)
{
   return ReadCharToken(',', TokenType::ArgSep, token);
}

// reads an operator token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadOperator(Token& token)
{
   if (Read2CharsToken(std::make_pair('<', '='), TokenType::OperatorLessOrEqual, token) ||
       Read2CharsToken(std::make_pair('>', '='), TokenType::OperatorMoreOrEqual, token) ||
       Read2CharsToken(std::make_pair('=', '='), TokenType::OperatorEqual, token)       ||
       Read2CharsToken(std::make_pair('!', '='), TokenType::OperatorNotEqual, token)    ||
       Read2CharsToken(std::make_pair('&', '&'), TokenType::OperatorLogicalAnd, token)  ||
       Read2CharsToken(std::make_pair('|', '|'), TokenType::OperatorLogicalOr, token)   ||
       ReadCharToken('<', TokenType::OperatorLess, token)                               ||
       ReadCharToken('>', TokenType::OperatorMore, token))
   {
      return true;
   }
   return false;
}

// reads a number token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadNumber(Token& token)
{
   if (std::isdigit(*_current))
   {
      // this is a number, search for its end
      auto it = std::find_if(_current + 1, _expression.cend(), [](const char ch) { return !std::isdigit(ch); });
      if (it != _expression.cend())
      {
         token._type  = TokenType::Scalar;
         token._begin = _current;
         token._end   = it;
         _current     = it;
         return true;
      }
   }

   return false;
}

// reads a string token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadString(Token& token)
{
   if (*_current == '\"')
   {
      // this is a string, search for its end
      auto it = std::find(_current + 1, _expression.cend(), '\"');
      if (it != _expression.cend())
      {
         token._type  = TokenType::Scalar;
         token._begin = _current + 1;
         token._end   = it;
         _current     = it + 1;
         return true;
      }
   }

   return false;
}

// reads a function or variable token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadFunctionOrVariable(Token& token)
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

// reads a left square bracket token (array) into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadLSquareBracket(Token& token)
{
   return ReadCharToken('[', TokenType::LSquareBracket, token);
}

// reads a right square bracket token (array) into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadRSquareBracket(Token& token)
{
   return ReadCharToken(']', TokenType::RSquareBracket, token);
}

// reads a left brace token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadLBrace(Token& token)
{
   return ReadCharToken('{', TokenType::LBrace, token);
}

// reads a right brace token into the 'token' output parameter
// returns true if successful
bool ExpressionParser::ReadRBrace(Token& token)
{
   return ReadCharToken('}', TokenType::RBrace, token);
}

// process the specified token, returns true if successful
bool ExpressionParser::ProcessToken(const Token& token)
{
   switch (token._type)
   {
      case TokenType::Scalar:
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
         if (token.IsOperator())
            return ProcessOperator(token);
   }
   return false; // unknown token
}

bool ExpressionParser::ProcessScalar(const Token& token)
{
   _expression_tree.push(std::make_shared<ExpressionNode>(token));
   return true;
}

bool ExpressionParser::ProcessArgSep(const Token& token)
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

bool ExpressionParser::ProcessFunc(const Token& token)
{
   _operators.push(token);
   return true;
}

bool ExpressionParser::ProcessLBracket(const Token& token)
{
   _operators.push(token);
   return true;
}

bool ExpressionParser::ProcessRBracket(const Token& token)
{
   while (!_operators.empty() && !IsCurrentToken(TokenType::LBracket))
   {
      if (!MoveToOutput(2, false))
         return false;
      _operators.pop();
   }

   // empty operator stack means that there was no pair left bracket token, so this is a parsing error
   if (_operators.empty())
      return false;

   _operators.pop(); // pop left bracket
   return true;
}

bool ExpressionParser::ProcessOperator(const Token& token)
{
   // process all previous operators with more priority
   while (!_operators.empty() &&
          _operators.top().IsOperator() &&
          token._type <= _operators.top()._type)
   {
      if (!MoveToOutput(2, false))
         return false;
      _operators.pop();
   }

   _operators.push(token);
   return true;
}

bool ExpressionParser::ProcessLSquareBracket(const Token& token)
{
   _operators.push(token);
   _args_number.push(1);
   return true;
}

bool ExpressionParser::ProcessRSquareBracket(const Token& token)
{
   if (!IsCurrentToken(TokenType::LSquareBracket) || _args_number.empty())
      return false;

   if (!MoveToOutput(_args_number.top(), false))
      return false;

   _operators.pop();
   _args_number.pop();
   return true;
}

bool ExpressionParser::ProcessLBrace(const Token& token)
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

bool ExpressionParser::ProcessRBrace(const Token& token)
{
   if (!IsCurrentToken(TokenType::LBrace) || _args_number.empty())
      return false;

   if (!MoveToOutput(_args_number.top(), true))
      return false;

   _operators.pop(); // pop left brace

   // check and pop function token, update function node token in the tree
   if (IsCurrentToken(TokenType::Func))
   {
      _expression_tree.top()->_token = _operators.top();
      _operators.pop();
      return true;
   }
   else
      return false;
}

bool ExpressionParser::CheckOutputNode(const std::shared_ptr<ExpressionNode>& node, const uint16_t operands_number) const
{
   if (!node)
      return false;

   switch (node->_token._type)
   {
      case TokenType::RBrace:
         return CheckFunctionNode(node);
      case TokenType::RSquareBracket:
         return CheckArrayNode(node, operands_number);
      // TODO: check other token types
      default:
         return true;
   }
}

bool ExpressionParser::CheckFunctionNode(const std::shared_ptr<ExpressionNode>& node) const
{
   if (!node)
      return false;

   if (std::string(node->_token._begin, node->_token._end) == "SUBSTR")
   {
      if (!node->_child || !node->_child->_sibling || !node->_child->_sibling->_sibling ||
          node->_child->_token._type != TokenType::Variable ||
          node->_child->_sibling->_token._type != TokenType::Scalar ||
          node->_child->_sibling->_sibling->_token._type != TokenType::Scalar)
      {
         return false;
      }
   }
   return true;
}

bool ExpressionParser::CheckArrayNode(const std::shared_ptr<ExpressionNode>& node, const uint16_t operands_number) const
{
   if (!node)
      return false;

   // check that all array items are scalar
   auto child_node = node->_child;

   for (uint16_t i = 0; i < operands_number && child_node; i++)
   {
      if (child_node->_token._type != TokenType::Scalar)
         return false;
      child_node = child_node->_sibling;
   }

   return true;
}


// syntax tree print helper function
// recursively traverses the syntax tree and prints every node to stdout
// 'level' the nesting level of current node, is used for indentation while printing
void ExpressionParser::PrintOutputTree(const std::shared_ptr<ExpressionNode>& node, const size_t level) const
{
   if (!node)
      return;

   std::cout << std::string(level * 2, ' ') << node->_token.ToString() << std::endl;

   auto child = node->_child;
   while (child)
   {
      PrintOutputTree(child, level + 1);
      child = child->_sibling;
   }
}
}

