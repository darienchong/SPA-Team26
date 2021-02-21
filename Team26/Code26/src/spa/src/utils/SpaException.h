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
    const static std::string SYNTAX_ERROR_COND_EXPR_NOT_ENOUGH_TOKENS = "Expected more tokens for conditional expression but ran out.";
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
    const static std::string SYNTAX_ERROR_NOT_ENOUGH_TOKENS = "Expected more tokens but ran out.";
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

namespace Pql {
  namespace ErrorMessage {
    // =============
    // Base Message
    // =============

    // Syntax Errors
    const static std::string SYNTAX_ERROR_NOT_ENOUGH_TOKENS = "Expected more tokens but ran out.";
    const static std::string SYNTAX_ERROR_ADDITIONAL_TOKENS = "Expected no more tokens but encountered additional tokens.";
    const static std::string SYNTAX_ERROR_WRONG_TOKEN_TYPE = "Expected token of a different type.";
    const static std::string SYNTAX_ERROR_WRONG_TOKEN_VALUE = "Expected token of a different value.";
    const static std::string SYNTAX_ERROR_INVALID_DESIGN_ENTITY = "Encountered an invalid design entity for declaration.";
    const static std::string SYNTAX_ERROR_INVALID_RELATION = "Encountered an invalid relation for such that clause.";


    // Semantic Errors
    const static std::string SEMANTIC_ERROR_DUPLICATE_SYNONYM_DECLARATION = "Encountered an already declared synonym.";
    const static std::string SEMANTIC_ERROR_UNDECLARED_SYNONYM = "Encountered undeclared synonym.";
    const static std::string SEMANTIC_ERROR_NON_ASSIGN_SYNONYM = "Expected a synonym of type assign but encounted another type.";
    const static std::string SEMANTIC_ERROR_NON_STMT_REF = "Expected a stmtRef but encounted other design entity.";
    const static std::string SEMANTIC_ERROR_NON_ENT_REF = "Expected a entRef but encounted other design entity.";
    const static std::string SEMANTIC_ERROR_ZERO_STMT_NUMBER = "Encountered 0 as a statement number.";

    // =================
    // Appended Message
    // =================
    const static std::string APPEND_TOKEN_EXPECTED = "\nToken expected: ";
    const static std::string APPEND_TOKEN_RECEIVED = "\nToken received: ";
  }

  /**
   * Exception to be thrown when there is a syntax error in the PQL query.
   */
  class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string& msg);
  };

  /**
   * Exception to be thrown when there is a semantic error in the PQL query.
   */
  class SemanticError : public std::exception {
  public:
    SemanticError(const std::string& msg);
  };
}
