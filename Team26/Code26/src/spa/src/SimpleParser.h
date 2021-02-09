#pragma once

#include <list>
#include <string>

#include "ExprParser.h"
#include "PKB.h"
#include "PkbStub.h" // remove this once pkb is implemented
#include "Token.h"

namespace SourceProcessor {
  class SimpleParser {
  private:
    // Class variables
    std::list<Token> tokens;
    int stmtNum = 1;
    ExprProcessor::ExprParser exprParser;
    PkbStub pkb; // might not be needed???

    // Functions
    Token getNextToken();
    std::string validate(const Token& token);
    std::list<std::string> parseCond();
    int parseIf();
    int parseCall();
    int parseRead();
    int getStmtNum();
    int parsePrint();
    int parseWhile();
    int parseAssign();
    int parseStmt(int first);
    void parseExpr();
    void incStmtNum();
    void parseProgram();
    void parseCondExpr();
    void parseProcedure();
    void parseAssignExpr();
    void removeNextToken();
    void parseStmtLst(int parent, int first);

  public:
    SimpleParser(std::list<Token> tokens);
    ~SimpleParser();

    void parse();
  };
}