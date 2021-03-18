#include "ExprParser.h"

#include <assert.h>

#include <list>
#include <stack>
#include <string>
#include <unordered_set>

#include "Token.h"
#include "SpaException.h"

namespace ExprProcessor {
  //==================//
  //  CondExprParser  //
  //==================//

  CondExprParser::CondExprParser(std::list<Token>& tokens) : tokens(tokens) {
    it = this->tokens.begin();
  };

  void CondExprParser::parse() {
    parseCondExpr();

    // Should have no more tokens left
    if (it != tokens.end()) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        it->value
      );
    }
  }

  std::unordered_set<std::string> CondExprParser::getVariables() const {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : tokens) {
      if (token.type == NAME.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  std::unordered_set<std::string> CondExprParser::getConstants() const {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : tokens) {
      if (token.type == CONST.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  void CondExprParser::parseCondExpr() {
    if (it == tokens.end()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS);
    }

    Token& currentToken = *it;
    if (currentToken == COND_EXPR_NOT) {
      validateAndConsume(COND_EXPR_NOT);
      validateAndConsume(LEFT_PARENTHESIS);
      parseCondExpr();
      validateAndConsume(RIGHT_PARENTHESIS);
    } else if (currentToken == LEFT_PARENTHESIS) {
      parseCondSubExpr();
    } else {
      parseRelExpr();
    }
  }


  void CondExprParser::parseCondSubExpr() {
    // Current token == '(' can mean a cond_expr or factor
    // Thus, try parsing a cond_expr first. If parsing fails, parse factor only
    std::list<Token>::iterator tempPosition = it;
    try {
      validateAndConsume(LEFT_PARENTHESIS);
      parseCondExpr();
      validateAndConsume(RIGHT_PARENTHESIS);

      if (it == tokens.end()) {
        throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS);
      }

      Token& condExprBinaryOperatorToken = *it;
      ++it;
      if (isCondExprBinaryOperator(condExprBinaryOperatorToken)) {
        std::list<Token>::iterator secondTempPosition = it;
        try {
          validateAndConsume(LEFT_PARENTHESIS);
          parseCondExpr();
          validateAndConsume(RIGHT_PARENTHESIS);
        } catch (const SyntaxError&) {
          it = secondTempPosition;
          parseRelExpr();
        }
      } else {
        throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_INVALID_COND_SUB_EXPR);
      }
    } catch (const SyntaxError&) {
      it = tempPosition;
      parseRelExpr();
    }
  }

  void CondExprParser::parseRelExpr() {
    parseRelFactor();

    if (it == tokens.end()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS);
    }
    Token& currentToken = *it;
    ++it;

    if (!isRelExprOperator(currentToken)) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_INVALID_REL_EXPR);
    }
    parseRelFactor();
  }

  // This method is redundant but we are following the redundant SIMPLE grammar rule
  void CondExprParser::parseRelFactor() {
    parseExpr();
  }

  void CondExprParser::parseExpr() {
    parseTerm();
    while (it != tokens.end() && isExprOperator(*it)) {
      ++it;
      parseTerm();
    }
  }

  void CondExprParser::parseTerm() {
    parseFactor();
    while (it != tokens.end() && isTermOperator(*it)) {
      ++it;
      parseFactor();
    }
  }

  void CondExprParser::parseFactor() {
    if (it == tokens.end()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS);
    }

    const Token& currentToken = *it;
    ++it;

    const bool isVarOrConst = currentToken.type == NAME.type || currentToken.type == CONST.type;
    if (currentToken == LEFT_PARENTHESIS) {
      parseExpr();
      validateAndConsume(RIGHT_PARENTHESIS);
    } else if (!isVarOrConst) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_INVALID_FACTOR);
    }
    // Else isVarOrConst is true, do nothing
  }

  void CondExprParser::validateAndConsume(const Token& validationToken) {
    if (it == tokens.end()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_COND_EXPR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = *it;

    const bool isValidateTokenType = validationToken.value.empty();
    if (isValidateTokenType) {
      // Check type only for empty token value
      if (frontToken.type != validationToken.type) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_COND_EXPR_WRONG_TOKEN_TYPE +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    } else {
      // Check exact match otherwise
      if (frontToken != validationToken) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_COND_EXPR_WRONG_TOKEN_VALUE +
          ErrorMessage::APPEND_TOKEN_EXPECTED +
          validationToken.value +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    }

    // Validation success
    ++it;
  }

  bool CondExprParser::isCondExprBinaryOperator(const Token& token) const {
    return token == COND_EXPR_AND || token == COND_EXPR_OR;
  }

  bool CondExprParser::isRelExprOperator(const Token& token) const {
    return token == REL_EXPR_OP_GRT ||
      token == REL_EXPR_OP_GEQ ||
      token == REL_EXPR_OP_LET ||
      token == REL_EXPR_OP_LEQ ||
      token == REL_EXPR_OP_EQV ||
      token == REL_EXPR_OP_NEQ;;
  }


  bool CondExprParser::isExprOperator(const Token& token) const {
    return token == EXPR_OP_PLUS || token == EXPR_OP_MINUS;
  }

  bool CondExprParser::isTermOperator(const Token& token) const {
    return token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE || token == EXPR_OP_MOD;
  }

  //==================//
  // AssignExprParser //
  //==================//

  AssignExprParser::AssignExprParser(std::list<Token>& infixExprTokens)
    : tokens(infixExprTokens) {
  }


  void AssignExprParser::parse() {
    generatePostfixExpr();
    validatePostfixExpr();
  }

  // Add spaces at the start and end so that 
  // " fizz " will not be a substring of " fizzBuzz "
  // Otherwise, "fizz" is a substring of "fizzBuzz"
  std::string AssignExprParser::getPostfixExprString() const {
    std::string resultString = " ";
    for (const Token& token : postfixExprTokens) {
      resultString.append(token.value);
      resultString.append(" ");
    }
    return resultString;
  }

  std::unordered_set<std::string> AssignExprParser::getVariables() const {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : postfixExprTokens) {
      if (token.type == NAME.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  std::unordered_set<std::string> AssignExprParser::getConstants() const {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : postfixExprTokens) {
      if (token.type == CONST.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  int AssignExprParser::getOperatorPrecedence(const Token& token) const {
    if (token == EXPR_OP_MOD || token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE) {
      return 2;
    }
    if (token == EXPR_OP_PLUS || token == EXPR_OP_MINUS) {
      return 1;
    }
    assert(false);
    return 0;
  }

  void AssignExprParser::generatePostfixExpr() {
    std::stack<Token> stack;
    bool expectOperand = true;

    while (!tokens.empty()) {
      const Token next = tokens.front();
      tokens.pop_front();
      const bool isOperator = next == EXPR_OP_PLUS || next == EXPR_OP_MINUS || next == EXPR_OP_TIMES || next == EXPR_OP_DIVIDE || next == EXPR_OP_MOD;
      const bool isOperand = next.type == NAME.type || next.type == CONST.type;

      if (isOperand) {
        if (!expectOperand) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERATOR_TOKEN +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            next.value
          );
        }
        expectOperand = false;

        postfixExprTokens.emplace_back(next);
      } else if (isOperator) {
        if (expectOperand) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERAND_TOKEN +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            next.value
          );
        }
        expectOperand = true;

        // Pop from stack and add to postfixExpr until current operator precedece is greater or '(' is encountered.
        while (!stack.empty() && stack.top() != LEFT_PARENTHESIS && (getOperatorPrecedence(next) <= getOperatorPrecedence(stack.top()))) {
          postfixExprTokens.emplace_back(stack.top());
          stack.pop();
        }
        stack.push(next);
      } else if (next == LEFT_PARENTHESIS) {
        if (!expectOperand) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERATOR_TOKEN +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            next.value
          );
        }

        stack.push(next);
      } else if (next == RIGHT_PARENTHESIS) {
        if (expectOperand) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERAND_TOKEN +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            next.value
          );
        }

        if (stack.empty()) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_MISMATCHED_PARENTHESIS +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            next.value
          );
        }

        while (stack.top() != LEFT_PARENTHESIS) {
          postfixExprTokens.emplace_back(stack.top());
          stack.pop();

          if (stack.empty()) { // Missing '(' to pop
            throw SyntaxError(
              ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_MISMATCHED_PARENTHESIS +
              ErrorMessage::APPEND_TOKEN_RECEIVED +
              next.value
            );
          }
        }

        stack.pop(); //Pop '(' in the stack
      } else { // Invalid tokens. e.g. '{', '<', '_'
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_INVALID_TOKEN +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          next.value
        );
      }
    }

    while (!stack.empty()) {
      if (stack.top() == LEFT_PARENTHESIS) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_MISMATCHED_PARENTHESIS +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          stack.top().value
        );
      }
      postfixExprTokens.emplace_back(stack.top());
      stack.pop();
    }
  }

  // Pre-condition: postfixExprTokens only contains operands (NAME, CONST) or operators (EXPR_OP: '+', '-', '*', '/', '%')
  // Validates postfix expr
  void AssignExprParser::validatePostfixExpr() {
    if (postfixExprTokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_EMPTY_EXPRESSION);
    }

    std::stack<Token> stack;
    std::list<Token> postfixExprTokens(this->postfixExprTokens); // duplicate postfixExprTokens for validation

    while (!postfixExprTokens.empty()) {
      const Token front = postfixExprTokens.front();
      postfixExprTokens.pop_front();

      const bool isOperand = front.type == CONST.type || front.type == NAME.type;
      const bool isOperator = front == EXPR_OP_PLUS || front == EXPR_OP_MINUS || front == EXPR_OP_TIMES || front == EXPR_OP_DIVIDE || front == EXPR_OP_MOD;;
      if (isOperand) {
        stack.push(front);
      } else if (isOperator) { // operator
        if (stack.empty()) {
          throw SyntaxError(ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_INVALID_EXPRESSION);
        }
        stack.pop();
        if (stack.empty()) {
          throw SyntaxError(ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_INVALID_EXPRESSION);
        }
        stack.pop();
        // Creates a dummy token as the result of the subexpression for validation
        stack.push(DUMMY_RESULT);
      } else {
        assert(false);
      }
    }

    // At this point, stack should have exactly one element which is the result
    if (stack.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_INVALID_EXPRESSION);
    }
    stack.pop(); // pop result
    if (!stack.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_ASSIGN_EXPR_INVALID_EXPRESSION);
    }
  }
}