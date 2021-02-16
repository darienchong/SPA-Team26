#include "ExprParser.h"

#include <list>
#include <string>

#include "Token.h"

namespace ExprProcessor {
  // Operators
  const static Token EXPR_OP_PLUS{ TokenType::OPERATOR, "+" };
  const static Token EXPR_OP_MINUS{ TokenType::OPERATOR, "-" };
  const static Token EXPR_OP_TIMES{ TokenType::OPERATOR, "*" };
  const static Token EXPR_OP_DIVIDE{ TokenType::OPERATOR, "/" };
  const static Token EXPR_OP_MOD{ TokenType::OPERATOR, "%" };

  // Delimiters
  const static Token LEFT_PARENTHESIS{ TokenType::DELIMITER, "(" };
  const static Token RIGHT_PARENTHESIS{ TokenType::DELIMITER, ")" };

  // Variable and Constant
  const static Token NAME{ TokenType::IDENTIFIER, "" };
  const static Token CONST{ TokenType::NUMBER, "" };

  void ExprParser::addToPostFix(Token token) {
    bool isOperator = token == EXPR_OP_PLUS || token == EXPR_OP_MINUS || token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE || token == EXPR_OP_MOD;
    bool isParenthesis = token == LEFT_PARENTHESIS || token == RIGHT_PARENTHESIS;
    bool isElement = token.type == NAME.type || token.type == CONST.type;

    if (!(isOperator || isParenthesis || isElement)) {
      throw "Invalid token XXX for expression!";
    }

    post.push_back(token);
  }

  void ExprParser::addToVarLst(std::string varName) {
    varLst.push_back(varName);
  }

  Token ExprParser::getNextToken() {
    return tokens.front();
  }
  void ExprParser::removeNextToken() {
    tokens.pop_front();
  }

  // Used in postFix() - rmb to change to overloading
  int ExprParser::precedence(Token& token) {
    if (token == EXPR_OP_MOD || token == EXPR_OP_TIMES || token == EXPR_OP_DIVIDE) {
      return 2;
    }
    if (token == EXPR_OP_PLUS || token == EXPR_OP_MINUS) {
      return 1;
    }
    return 0;
  }

  // IMP: NEEDS TESTING
  void ExprParser::toPostFix() {
    std::list<Token> stack;

    while (!tokens.empty()) {
      Token next = getNextToken();
      removeNextToken();
      bool isOperator = next == EXPR_OP_PLUS || next == EXPR_OP_MINUS || next == EXPR_OP_TIMES || next == EXPR_OP_DIVIDE || next == EXPR_OP_MOD;

      if (next.type == NAME.type || next.type == CONST.type) {
        addToPostFix(next);
      }
      else if (isOperator) {
        while (!stack.empty() && stack.front() != LEFT_PARENTHESIS && (precedence(next) <= precedence(stack.front()))) {
          post.push_back(stack.front());
          stack.pop_front();
        }
        stack.push_front(next);
      }
      else if (next == LEFT_PARENTHESIS) {
        stack.push_front(next);
      }
      else { // next = RIGHT_PARENTHESIS
        if (stack.empty()) {
          throw "Syntax Error! Mismatched parenthesis in expression!";
        }

        while (stack.front() != LEFT_PARENTHESIS) {
          addToPostFix(stack.front());
          stack.pop_front();

          if (stack.empty()) { // Missing '(' to pop
            throw "Syntax Error! Mismatched parenthesis in expression!";
          }
        }

        stack.pop_front(); //Pop '(' in the stack
      }
    }

    while (!stack.empty()) {
      if (stack.front() == LEFT_PARENTHESIS) {
        throw "Syntax Error! Mismatched parenthesis in expression!";
      }
      post.push_back(stack.front());
      stack.pop_front();
    }
  }

  // Evaluates postfix expr, also checks for mismatched OPs - should only need to check for empty expr
  void ExprParser::evalPostFix(std::list<Token> post) {
    if (post.empty()) {
      throw "Syntax Error! Expression cannot be empty!";
    }

    //std::list<std::string> exprs;
    std::list<std::string> stack;

    while (!post.empty()) {
      Token front = post.front();
      post.pop_front();
      if (front.type == CONST.type) {
        stack.push_front(front.value);
      }
      else if (front.type == NAME.type) {
        stack.push_front(front.value);
        addToVarLst(front.value);
      }
      else { // operator
        if (stack.empty()) {
          throw "Syntax Error! Operator XXX requires 2 operands!";
        }
        std::string first = stack.front();
        stack.pop_front();
        if (stack.empty()) {
          throw "Syntax Error! Operator XXX requires 2 operands!";
        }
        std::string second = stack.front();
        stack.pop_front();
        std::string combined = first + front.value + second; // might need to swap around first and second
        //if (exprs.empty()) {
        //  exprs.push_back(first);
        //}
        //exprs.push_back(second);
        //exprs.push_back(combined);
        stack.push_front(combined);
      }
    }
  }

  ExprParser::ExprParser() {};
  ExprParser::~ExprParser() {};

  void ExprParser::addToken(Token token) {
    tokens.push_back(token);
  }

  void ExprParser::parseExpr() {
    toPostFix();
    evalPostFix(post);
  }

  void ExprParser::clear() {
    post.clear();
    tokens.clear();
    varLst.clear();
    exprLst.clear();
  }

  /**
   * Returns a list of tokens in postfix expression.
   *
   * @returns std::list<Token> The list of tokens in postfix expression.
   */
  std::list<Token> ExprParser::getPostFix() {
    return post;
  }

  /**
   * Returns a list of variables used in the expression.
   *
   * @returns std::list<std::string> The list of variables in string representation.
   */
  std::list<std::string> ExprParser::getVariables() {
    return varLst;
  }

  // might be useless
  std::list<std::string> ExprParser::getExprs() {
    return exprLst;
  }
}