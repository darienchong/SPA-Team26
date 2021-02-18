#pragma once

#include <string>
#include <exception>

namespace ExprProcessor {
  namespace ErrorMessage {
    // =============
    // Base Message
    // =============

    // Syntax Errors
    // Cond Expr
    const static std::string SYNTAX_ERROR_COND_EXPR_NOT_ENOUGHT_TOKENS = "Expected more tokens for conditional expression but ran out.";
    const static std::string SYNTAX_ERROR_COND_EXPR_ADDITIONAL_TOKENS = "Encountered additional tokens the end of a conditional expression.";
    const static std::string SYNTAX_ERROR_COND_EXPR_WRONG_TOKEN_TYPE = "Expected token of a different type.";
    const static std::string SYNTAX_ERROR_COND_EXPR_WRONG_TOKEN_VALUE = "Expected token of a different value.";
    const static std::string SYNTAX_ERROR_COND_EXPR_INVALID_COND_SUB_EXPR = "Encountered invalid conditional subexpression.";
    const static std::string SYNTAX_ERROR_COND_EXPR_INVALID_REL_EXPR = "Encountered invalid relational expression in conditional expression.";
    const static std::string SYNTAX_ERROR_COND_EXPR_INVALID_FACTOR = "Encountered invalid factor in conditional expression.";

    // AssignExpr
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_EMPTY_EXPRESSION = "Encounted an empty expression.";
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_INVALID_EXPRESSION = "Encountered an invalid expression.";
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_INVALID_TOKEN = "Encounted an invalid token in an expression.";
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERATOR_TOKEN = "Expected an operator between 2 operands.";
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_NON_OPERAND_TOKEN = "Expected an operand.";
    const static std::string SYNTAX_ERROR_ASSIGN_EXPR_MISMATCHED_PARENTHESIS = "Encountered mismatched parenthesis in an expression.";

    // =================
    // Appended Message
    // =================
    const static std::string APPEND_TOKEN_EXPECTED = "\nToken expected: ";
    const static std::string APPEND_TOKEN_RECEIVED = "\nToken received: ";
  }

  /**
   * Exception to be thrown when there is a syntax error in the SIMPLE program.
   */
  class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string& msg);
  };
}

namespace SourceProcessor {
  namespace ErrorMessage {
    // =============
    // Base Message
    // =============

    // Syntax Errors
    const static std::string SYNTAX_ERROR_NOT_ENOUGHT_TOKENS = "Expected more tokens but ran out.";
    const static std::string SYNTAX_ERROR_ADDITIONAL_TOKENS = "Expected no more tokens but encountered additional tokens.";
    const static std::string SYNTAX_ERROR_WRONG_TOKEN_TYPE = "Expected token of a different type.";
    const static std::string SYNTAX_ERROR_WRONG_TOKEN_VALUE = "Expected token of a different value.";
    const static std::string SYNTAX_ERROR_EMPTY_STMT_LIST = "Encountered an empty stmtLst.";
    const static std::string SYNTAX_ERROR_UNKNOWN_STMT_TYPE = "Encountered an unknowned statement type.";

    // Semantic Errors - Non for Iteration 1

    // =================
    // Appended Message
    // =================
    const static std::string APPEND_TOKEN_EXPECTED = "\nToken expected: ";
    const static std::string APPEND_TOKEN_RECEIVED = "\nToken received: ";
    const static std::string APPEND_STMT_NUMBER = "\nStatement number: ";
  }

  /**
   * Exception to be thrown when there is a syntax error in the SIMPLE program.
   */
  class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string& msg);
  };

  /**
   * Exception to be thrown when there is a semantic error in the SIMPLE program.
   */
  class SemanticError : public std::exception {
  public:
    SemanticError(const std::string& msg);
  };
}