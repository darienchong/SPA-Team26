#include "ExprParser.h"

#include <assert.h>
#include <list>
#include <stack>
#include <string>
#include <unordered_set>

#include "Token.h"

namespace ExprProcessor {
  //==================//
  // CondExprParser //
  //==================//

  CondExprParser::CondExprParser(std::list<Token>& tokens) : tokens(tokens) {
    if (tokens.empty()) {
      throw "Conditional expression cannot be empty!";
    }
    it = this->tokens.begin();
  };

  void CondExprParser::parse() {
    parseCondExpr();
  }

  std::unordered_set<std::string> CondExprParser::getVariables() {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : tokens) {
      if (token.type == NAME.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  std::unordered_set<std::string> CondExprParser::getConstants() {
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
      throw "Syntax Error! Expected more tokens for conditional expression!";
    }

    Token currentToken = *it;
    if (currentToken == COND_EXPR_NOT) {
      validateAndConsume(COND_EXPR_NOT);
      validateAndConsume(LEFT_PARENTHESIS);
      parseCondExpr();
      validateAndConsume(RIGHT_PARENTHESIS);
    }
    else if (currentToken == LEFT_PARENTHESIS) {
      parseCondSubExpr();
    }
    else {
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
      Token condExprBinaryOperatorToken = *it;
      ++it;
      if (isCondExprBinaryOperator(condExprBinaryOperatorToken)) {
        it++;
        std::list<Token>::iterator secondTempPosition = it;
        try {
          validateAndConsume(LEFT_PARENTHESIS);
          parseCondExpr();
          validateAndConsume(RIGHT_PARENTHESIS);
        }
        catch (...) {
          it = secondTempPosition;
          parseRelExpr();
        }
      }
      else {
        throw "Syntax Error! Invalid conditional subexpression!";
      }
    }
    catch (...) {
      it = tempPosition;
      parseRelExpr();
    }
  }

  void CondExprParser::parseRelExpr() {
    parseRelFactor();

    if (it == tokens.end()) {
      throw "Syntax Error! Expected more tokens for conditional expression!";
    }
    Token currentToken = *it;
    ++it;

    if (!isRelExprOperator(currentToken)) {
      throw "Syntax Error! Invalid relational expression in conditional expression!";
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
      throw "Syntax Error! Expected more tokens for conditional expression!";
    }

    Token currentToken = *it;
    ++it;

    bool isVarOrConst = currentToken.type == NAME.type || currentToken.type == CONST.type;
    if (currentToken == LEFT_PARENTHESIS) {
      parseExpr();
      validateAndConsume(RIGHT_PARENTHESIS);
    }
    else if (!isVarOrConst) {
      throw "Syntax Error! Invalid factor for conditional expression!";
    }
    // Else if isVarOrConst is true, do nothing
  }

  void CondExprParser::validateAndConsume(const Token& token) {
    if (it == tokens.end()) {
      throw "Expected a token but received none";
    }

    const Token frontToken = *it;

    if (token.value.empty()) {
      // Check type only for empty token value
      if (frontToken.type != token.type) {
        throw "Encounted token of a wrong type";
      }
    }
    else {
      // Check exact match otherwise
      if (frontToken != token) {
        throw "Expected " + token.value + " but encountered " + frontToken.value;
      }
    }

    // Validation success
    ++it;
  }

  bool CondExprParser::isCondExprBinaryOperator(const Token& token) {
    return token == COND_EXPR_AND || token == COND_EXPR_OR;
  }

  bool CondExprParser::isRelExprOperator(const Token& token)
  {
    return token == REL_EXPR_OP_GRT ||
      token == REL_EXPR_OP_GEQ ||
      token == REL_EXPR_OP_LET ||
      token == REL_EXPR_OP_LEQ ||
      token == REL_EXPR_OP_EQV ||
      token == REL_EXPR_OP_NEQ;;
  }


  bool CondExprParser::isExprOperator(const Token& token) {
    return token == EXPR_OP_PLUS || token == EXPR_OP_MINUS;
  }

  bool CondExprParser::isTermOperator(const Token& token) {
    return token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE || token == EXPR_OP_MOD;
  }

  //==================//
  // AssignExprParser //
  //==================//

  AssignExprParser::AssignExprParser(std::list<Token>& infixExprTokens)
    : tokens(infixExprTokens) {}


  void AssignExprParser::parse() {
    generatePostfixExpr();
    validatePostfixExpr();
  }


  std::list<Token> AssignExprParser::getPostfixExprTokens() {
    return postfixExprTokens;
  }

  std::string AssignExprParser::getPostfixExprString() {
    std::string resultString;
    for (const Token& token : postfixExprTokens) {
      resultString.append(token.value);
      resultString.append(" ");
    }
    resultString.pop_back(); // remove last space
    return resultString;
  }

  std::unordered_set<std::string> AssignExprParser::getVariables() {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : postfixExprTokens) {
      if (token.type == NAME.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  std::unordered_set<std::string> AssignExprParser::getConstants() {
    std::unordered_set<std::string> resultSet;
    for (const Token& token : postfixExprTokens) {
      if (token.type == CONST.type) {
        resultSet.emplace(token.value);
      }
    }
    return resultSet;
  }

  // Used in postFix() - rmb to change to overloading
  int AssignExprParser::getOperatorPrecedence(Token& token) {
    if (token == EXPR_OP_MOD || token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE) {
      return 2;
    }
    if (token == EXPR_OP_PLUS || token == EXPR_OP_MINUS) {
      return 1;
    }
    return 0;
  }

  void AssignExprParser::generatePostfixExpr() {
    std::stack<Token> stack;
    bool expectOperand = true;

    while (!tokens.empty()) {
      Token next = tokens.front();
      tokens.pop_front();
      bool isOperator = next == EXPR_OP_PLUS || next == EXPR_OP_MINUS || next == EXPR_OP_TIMES || next == EXPR_OP_DIVIDE || next == EXPR_OP_MOD;
      bool isOperand = next.type == NAME.type || next.type == CONST.type;

      if (isOperand) {
        if (!expectOperand) {
          throw "Syntax Error! Expected operator between 2 operands!";
        }
        expectOperand = false;

        postfixExprTokens.emplace_back(next);
      }
      else if (isOperator) {
        if (expectOperand) {
          throw "Syntax Error! Expected operand but got an operator!";
        }
        expectOperand = true;

        // Pop from stack and add to postfixExpr until current operator precedece is greater or '(' is encountered.
        while (!stack.empty() && stack.top() != LEFT_PARENTHESIS && (getOperatorPrecedence(next) <= getOperatorPrecedence(stack.top()))) {
          postfixExprTokens.emplace_back(stack.top());
          stack.pop();
        }
        stack.push(next);
      }
      else if (next == LEFT_PARENTHESIS) {
        if (!expectOperand) {
          throw "Syntax Error! Expected operator before \"(\"!";
        }

        stack.push(next);
      }
      else if (next == RIGHT_PARENTHESIS) {
        if (expectOperand) {
          throw "Syntax Error! Expected operand before \")\"!";
        }

        if (stack.empty()) {
          throw "Syntax Error! Mismatched parenthesis in expression!";
        }

        while (stack.top() != LEFT_PARENTHESIS) {
          postfixExprTokens.emplace_back(stack.top());
          stack.pop();

          if (stack.empty()) { // Missing '(' to pop
            throw "Syntax Error! Mismatched parenthesis in expression!";
          }
        }

        stack.pop(); //Pop '(' in the stack
      }
      else { // Invalid tokens. e.g. '{', '<', '_'
        throw "Syntax Error! Invalid token for expression!";
      }
    }

    while (!stack.empty()) {
      if (stack.top() == LEFT_PARENTHESIS) {
        throw "Syntax Error! Mismatched parenthesis in expression!";
      }
      postfixExprTokens.emplace_back(stack.top());
      stack.pop();
    }
  }

  // Pre-condition: postfixExprTokens only contains operands (NAME, CONST) or operators (EXPR_OP: '+', '-', '*', '/', '%')
  // Validates postfix expr
  void AssignExprParser::validatePostfixExpr() {
    if (postfixExprTokens.empty()) {
      throw "Syntax Error! Expression cannot be empty!";
    }

    std::stack<Token> stack;
    std::list<Token> postfixExprTokens(this->postfixExprTokens); // duplicate postfixExprTokens for validation

    while (!postfixExprTokens.empty()) {
      Token front = postfixExprTokens.front();
      postfixExprTokens.pop_front();

      bool isOperand = front.type == CONST.type || front.type == NAME.type;
      bool isOperator = front == EXPR_OP_PLUS || front == EXPR_OP_MINUS || front == EXPR_OP_TIMES || front == EXPR_OP_DIVIDE || front == EXPR_OP_MOD;;
      if (isOperand) {
        stack.push(front);
      }
      else if (isOperator) { // operator
        if (stack.empty()) {
          throw "Syntax Error! Operator XXX requires 2 operands!";
        }
        stack.pop();
        if (stack.empty()) {
          throw "Syntax Error! Operator XXX requires 2 operands!";
        }
        stack.pop();
        // Creates a dummy token as the result of the subexpression for validation
        Token dummyResult = { TokenType::IDENTIFIER, "dummy" };
        stack.push(dummyResult);
      }
      else {
        assert(false);
      }
    }

    // At this point, stack should have exactly one element which is the result
    if (stack.empty()) {
      throw "Syntax Error! Invalid expression!";
    }
    stack.pop(); // pop result
    if (!stack.empty()) {
      throw "Syntax Error! Invalid expression!";
    }
  }
}