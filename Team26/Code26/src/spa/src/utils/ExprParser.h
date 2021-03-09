#pragma once

#include <list>
#include <string>
#include <unordered_set>

#include "Token.h"

namespace ExprProcessor {
  // Variable and Constant
  const static Token NAME{ TokenType::IDENTIFIER, "" };
  const static Token CONST{ TokenType::NUMBER, "" };

  // Operators
  const static Token EXPR_OP_PLUS{ TokenType::OPERATOR, "+" };
  const static Token EXPR_OP_MINUS{ TokenType::OPERATOR, "-" };
  const static Token EXPR_OP_TIMES{ TokenType::OPERATOR, "*" };
  const static Token EXPR_OP_DIVIDE{ TokenType::OPERATOR, "/" };
  const static Token EXPR_OP_MOD{ TokenType::OPERATOR, "%" };

  // Delimiters
  const static Token LEFT_PARENTHESIS{ TokenType::DELIMITER, "(" };
  const static Token RIGHT_PARENTHESIS{ TokenType::DELIMITER, ")" };

  // Relation Expression Operators
  const static Token REL_EXPR_OP_GRT{ TokenType::OPERATOR, ">" };
  const static Token REL_EXPR_OP_GEQ{ TokenType::OPERATOR, ">=" };
  const static Token REL_EXPR_OP_LET{ TokenType::OPERATOR, "<" };
  const static Token REL_EXPR_OP_LEQ{ TokenType::OPERATOR, "<=" };
  const static Token REL_EXPR_OP_EQV{ TokenType::OPERATOR, "==" };
  const static Token REL_EXPR_OP_NEQ{ TokenType::OPERATOR, "!=" };

  // Conditional Statement Operator
  const static Token COND_EXPR_NOT{ TokenType::OPERATOR, "!" };
  const static Token COND_EXPR_AND{ TokenType::OPERATOR, "&&" };
  const static Token COND_EXPR_OR{ TokenType::OPERATOR, "||" };

  // Dummy result token
  const static Token DUMMY_RESULT = { TokenType::IDENTIFIER, "dummy" };

  class CondExprParser {
  private:
    std::list<Token>& tokens;
    std::list<Token>::iterator it;

  public:
    /**
     * Constructs a CondExprParser with the given list of tokens.
     *
     * @param tokens List of tokens.
     */
    CondExprParser(std::list<Token>& tokens);

    /**
     * Runs the parser to parse an assignment expression
     */
    void parse();

    /**
     * Returns an unordered set of variables used in the expression.
     *
     * @return Unordered set of variables used in the expression.
     */
    std::unordered_set<std::string> getVariables() const;

    /**
     * Returns an unordered set of constants used in the expression.
     *
     * @return Unordered set of constants used in the expression.
     */
    std::unordered_set<std::string> getConstants() const;

  private:
    void parseCondExpr();
    void parseCondSubExpr();
    void parseRelExpr();
    void parseRelFactor();
    void parseExpr();
    void parseTerm();
    void parseFactor();
    void validateAndConsume(const Token& validationToken);
    bool isCondExprBinaryOperator(const Token& token) const;
    bool isRelExprOperator(const Token& token) const;
    bool isExprOperator(const Token& token) const;
    bool isTermOperator(const Token& token) const;


  };

  class AssignExprParser {
  private:
    std::list<Token> postfixExprTokens;
    std::list<Token>& tokens;

  public:
    /**
     * Constructs an AssignExprParser with the given list of tokens in infix format order.
     *
     * @param infixExprTokens List of tokens in infix format order
     */
    AssignExprParser(std::list<Token>& infixExprTokens);

    /**
     * Runs the parser to parse an assignment expression
     */
    void parse();

    /**
     * Returns a string representation of the expression in postfix format.
     *
     * @return String representation of the expression in postfix format.
     */
    std::string getPostfixExprString() const;

    /**
     * Returns a unordered set of variables used in the expression.
     *
     * @return Unordered set of variables used in the expression.
     */
    std::unordered_set<std::string> getVariables() const;

    /**
     * Returns a unordered set of constants used in the expression.
     *
     * @return Unordered set of constants used in the expression.
     */
    std::unordered_set<std::string> getConstants() const;

  private:
    void generatePostfixExpr();
    void validatePostfixExpr();
    int getOperatorPrecedence(const Token& token) const;
  };
}