#include "SpaException.h"  

#include <exception>
#include <string>

ExprProcessor::SyntaxError::SyntaxError(const std::string& msg)
  : std::exception(msg.c_str()) {}

SourceProcessor::SyntaxError::SyntaxError(const std::string& msg)
  : std::exception(("[SIMPLE Syntax Error] " + msg).c_str()) {}

SourceProcessor::SemanticError::SemanticError(const std::string& msg)
  : std::exception(("[SIMPLE Semantic Error] " + msg).c_str()) {}

Pql::SyntaxError::SyntaxError(const std::string& msg)
  : std::exception(("[PQL Syntax Error] " + msg).c_str()) {}

Pql::SemanticError::SemanticError(const std::string& msg)
  : std::exception(("[PQL Semantic Error] " + msg).c_str()) {}