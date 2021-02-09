#pragma once

#include <list>
#include <string>

#include "Token.h"

namespace ExprProcessor {
  class ExprParser {
  private:
    // Class variables
    std::list<Token> post;
    std::list<Token> tokens;
    std::list<std::string> varLst;
    std::list<std::string> exprLst;

    // Functions
    void toPostFix();
    void removeNextToken();
    void addToPostFix(Token token);
    void addToVarLst(std::string varName);
    void evalPostFix(std::list<Token> post);
    Token getNextToken();
    int precedence(Token& token);

  public:
    ExprParser();
    ~ExprParser();

    void clear();
    void parseExpr();
    void addToken(Token token);
    std::list<Token> getPostFix();
    std::list<std::string> getExprs();
    std::list<std::string> getVariables();
  };
}