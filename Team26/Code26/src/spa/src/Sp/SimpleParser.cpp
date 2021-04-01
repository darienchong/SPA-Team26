#include "SimpleParser.h"

#include <assert.h>

#include <list>
#include <string>
#include <vector>

#include "Token.h"
#include "SpaException.h"
#include "ExprParser.h"
#include "Pkb.h"

namespace SourceProcessor {
  // Delimiters
  const static Token SEMICOLON{ TokenType::DELIMITER, ";" };
  const static Token LEFT_BRACE{ TokenType::DELIMITER, "{" };
  const static Token RIGHT_BRACE{ TokenType::DELIMITER, "}" };
  const static Token LEFT_PARENTHESIS{ TokenType::DELIMITER, "(" };
  const static Token RIGHT_PARENTHESIS{ TokenType::DELIMITER, ")" };

  // Entities
  const static Token PROCEDURE{ TokenType::IDENTIFIER, "procedure" };
  const static Token READ{ TokenType::IDENTIFIER, "read" };
  const static Token PRINT{ TokenType::IDENTIFIER, "print" };
  const static Token ASSIGN{ TokenType::IDENTIFIER, "assign" };
  const static Token CALL{ TokenType::IDENTIFIER, "call" };
  const static Token WHILE{ TokenType::IDENTIFIER, "while" };
  const static Token IF{ TokenType::IDENTIFIER, "if" };
  const static Token THEN{ TokenType::IDENTIFIER, "then" };
  const static Token ELSE{ TokenType::IDENTIFIER, "else" };

  // Assignment Operator
  const static Token ASSIGN_OP{ TokenType::OPERATOR, "=" };

  // Identifier
  const static Token NAME{ TokenType::IDENTIFIER, "" };
  const static Token CONST{ TokenType::NUMBER, "" };

  void SimpleParser::incStmtNum() {
    ++stmtNum;
  }

  int SimpleParser::getStmtNum() {
    return stmtNum;
  }

  // Should only be called after getFrontToken()
  // Pre-condition: tokens will never be empty
  void SimpleParser::removeFrontToken() {
    if (tokens.empty()) {
      assert(false);
    }
    tokens.pop_front();
  }

  Token SimpleParser::getFrontToken() {
    if (tokens.empty()) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS +
        ErrorMessage::APPEND_STMT_NUMBER +
        std::to_string(getStmtNum())
      );
    }
    return tokens.front();
  }

  void SimpleParser::setCurrentProc(std::string procName) {
    currentProc = procName;
  }

  std::string SimpleParser::getCurrentProc() {
    return currentProc;
  }

  std::string SimpleParser::validate(const Token& validationToken) {
    Token front = getFrontToken();
    removeFrontToken();

    bool isValidateTokenType = validationToken.value.empty();
    if (isValidateTokenType) {
      if (validationToken.type != front.type) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          front.value +
          ErrorMessage::APPEND_STMT_NUMBER +
          std::to_string(getStmtNum())
        );
      }
    } else {
      if (validationToken != front) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_VALUE +
          ErrorMessage::APPEND_TOKEN_EXPECTED +
          validationToken.value +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          front.value +
          ErrorMessage::APPEND_STMT_NUMBER +
          std::to_string(getStmtNum())
        );
      }
    }

    return front.value;
  }

  // Valid number tokens should be guarenteed by Tokeniser
  void SimpleParser::validateNumToken() {
    Token front = getFrontToken();

    if (front.value.size() > 1 && front.value.front() == '0') {
      assert(false);
    }
  }

  std::string SimpleParser::parseAssignExpr() {
    std::list<Token> infixExprTokens;
    while (!tokens.empty()) {
      Token next = getFrontToken();
      if (next == SEMICOLON) { // check for end of assign stmt
        break;
      } else if (next.type == CONST.type) {
        validateNumToken();
      }
      infixExprTokens.emplace_back(next);
      removeFrontToken();
    }

    ExprProcessor::AssignExprParser exprParser(infixExprTokens);
    try {
      exprParser.parse();
    } catch (const ExprProcessor::SyntaxError& e) {
      throw SyntaxError(
        std::string(e.what()) +
        ErrorMessage::APPEND_STMT_NUMBER +
        std::to_string(getStmtNum())
      );
    }

    std::unordered_set<std::string> variablesUsed = exprParser.getVariables();
    std::unordered_set<std::string> constantsUsed = exprParser.getConstants();

    // adding information to pkb
    for (const std::string& variable : variablesUsed) {
      pkb.addUsesS(getStmtNum(), variable); // add Uses relation for stmt-var
      pkb.addUsesP(getCurrentProc(), variable); // add Uses relation for proc-var
      pkb.addVar(variable); // add vars
    }
    for (const std::string& constants : constantsUsed) {
      pkb.addConst(constants); // add consts
    }

    return exprParser.getPostfixExprString();
  }

  std::unordered_set<std::string> SimpleParser::parseCondExpr() {
    std::list<Token> infixExprTokens;
    auto it = tokens.begin();
    while (it != tokens.end() && std::next(it) != tokens.end()) {
      Token current = getFrontToken();
      Token next = *std::next(it);

      bool isEndOfCondExpr = current == RIGHT_PARENTHESIS && (next == THEN || next == LEFT_BRACE);
      if (isEndOfCondExpr) {
        break;
      } else if (current.type == CONST.type) {
        validateNumToken();
      }
      infixExprTokens.emplace_back(current);
      it++;
      removeFrontToken();
    }

    ExprProcessor::CondExprParser exprParser(infixExprTokens);
    try {
      exprParser.parse();
    } catch (const ExprProcessor::SyntaxError& e) {
      throw SyntaxError(
        std::string(e.what()) +
        ErrorMessage::APPEND_STMT_NUMBER +
        std::to_string(getStmtNum())
      );
    }
    std::unordered_set<std::string> variablesUsed = exprParser.getVariables();
    std::unordered_set<std::string> constantsUsed = exprParser.getConstants();

    // adding information to pkb
    for (const std::string& variable : variablesUsed) {
      pkb.addUsesS(getStmtNum(), variable); // add Uses relation for stmt-var
      pkb.addUsesP(getCurrentProc(), variable); // add Uses relation for proc-var
      pkb.addVar(variable); // add vars
    }
    for (const std::string& constants : constantsUsed) {
      pkb.addConst(constants); // add consts
    }

    return variablesUsed;
  }

  int SimpleParser::parseIf() {
    // grammar: 'if' '(' cond_expr ')'
    validate(IF);
    validate(LEFT_PARENTHESIS);
    std::unordered_set<std::string> variablesUsed = parseCondExpr();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addIf(stmtNum); // add if stmts
    for (const std::string& variable : variablesUsed) {
      pkb.addPatternIf(stmtNum, variable); // add if stmt control variable
    }

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    // grammar: 'then' '{' stmtLst '}'
    validate(THEN);
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

    std::vector<int> lastStmtsIfBranch;
    for (int i : prevStmts) {
      lastStmtsIfBranch.emplace_back(i);
    }

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    // grammar: 'else' '{' stmtLst '}'
    validate(ELSE);
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

    // update prevStmts
    for (int i : lastStmtsIfBranch) {
      prevStmts.emplace_back(i);
    }

    return stmtNum;
  }

  int SimpleParser::parseWhile() {
    // grammar: 'while' '(' cond_expr ')'
    validate(WHILE);
    validate(LEFT_PARENTHESIS);
    std::unordered_set<std::string> variablesUsed = parseCondExpr();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addWhile(stmtNum); // add while stmts
    for (const std::string& variable : variablesUsed) {
      pkb.addPatternWhile(stmtNum, variable); // add while stmt control variable
    }

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    // grammar: '{' stmtLst '}'
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    return stmtNum;
  }

  int SimpleParser::parseCall() {
    validate(CALL);
    std::string procName = validate(NAME);
    validate(SEMICOLON);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addCall(stmtNum); // add call stmts
    pkb.addCalls(getCurrentProc(), procName); // add proc-proc calls relation
    pkb.addCallProc(stmtNum, procName); // add stmt-proc

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    return stmtNum;
  }

  int SimpleParser::parseRead() {
    // grammar: 'read' var_name ';'
    validate(READ);
    std::string varName = validate(NAME);
    validate(SEMICOLON);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addVar(varName); // add variable
    pkb.addRead(stmtNum); // add read stmts
    pkb.addReadVar(stmtNum, varName); // add stmt-var
    pkb.addModifiesS(stmtNum, varName); // add Modifies relation for stmt-var
    pkb.addModifiesP(getCurrentProc(), varName); // add Modifies relation for proc-var

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    return stmtNum;
  }

  int SimpleParser::parsePrint() {
    // grammar: 'print' var_name ';'
    validate(PRINT);
    std::string varName = validate(NAME);
    validate(SEMICOLON);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addVar(varName); // add variable
    pkb.addPrint(stmtNum); // add print stmts
    pkb.addPrintVar(stmtNum, varName); // add stmt-var
    pkb.addUsesS(stmtNum, varName); // add Uses relation for stmt-var
    pkb.addUsesP(getCurrentProc(), varName); // add Uses relation for proc-var

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    return stmtNum;
  }

  int SimpleParser::parseAssign() {
    // grammar: var_name '=' expr ';'
    std::string varName = validate(NAME);
    validate(ASSIGN_OP);
    std::string exprString = parseAssignExpr();
    validate(SEMICOLON);

    // Getting relevant info to add to pkb
    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    for (int i : prevStmts) {
      pkb.addCfgEdge(i, stmtNum); // adding Next relation for stmt-stmt and Cfg nodes
    }
    pkb.addVar(varName); // add variable
    pkb.addAssign(stmtNum); /// add assign stmts
    pkb.addModifiesS(stmtNum, varName); // add Modifies relation for stmt-var
    pkb.addModifiesP(getCurrentProc(), varName); // add Modifies relation for proc-var
    pkb.addPatternAssign(stmtNum, varName, exprString); // add assign expr pattern

    // update prevStmts
    prevStmts.clear();
    prevStmts.emplace_back(stmtNum);

    return stmtNum;
  }

  int SimpleParser::parseStmt(int first) {
    Token keyword = getFrontToken();
    int stmt;

    if (keyword.type != NAME.type) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_UNKNOWN_STMT_TYPE +
        ErrorMessage::APPEND_STMT_NUMBER +
        std::to_string(getStmtNum())
      );
    } else {
      auto it = std::next(tokens.begin());

      if (it == tokens.end()) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS +
          ErrorMessage::APPEND_STMT_NUMBER +
          std::to_string(getStmtNum())
        );
      }

      if (*it == ASSIGN_OP) {
        stmt = parseAssign();
      } else if (keyword == IF) {
        stmt = parseIf();
      } else if (keyword == WHILE) {
        stmt = parseWhile();
      } else if (keyword == READ) {
        stmt = parseRead();
      } else if (keyword == PRINT) {
        stmt = parsePrint();
      } else if (keyword == CALL) {
        stmt = parseCall();
      } else {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_UNKNOWN_STMT_TYPE +
          ErrorMessage::APPEND_STMT_NUMBER +
          std::to_string(getStmtNum())
        );
      }
    }

    // Check if statement is itself
    if (first != stmt) {
      pkb.addFollows(first, stmt); // add Follows relation for stmt-stmt
    }
    pkb.addStmt(stmt); // add stmts

    return stmt;
  }

  void SimpleParser::parseStmtLst(int parent, int first) {
    bool isStmtLstEmpty = true;
    int stmt = first;
    while (!tokens.empty() && tokens.front() != RIGHT_BRACE) {
      stmt = parseStmt(stmt);

      bool isAtTopLevel = parent == 0;
      if (!isAtTopLevel) {
        pkb.addParent(parent, stmt); // add Parents relation for stmt-stmt
      }

      isStmtLstEmpty = false;
    }

    if (isStmtLstEmpty) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_EMPTY_STMT_LIST +
        ErrorMessage::APPEND_STMT_NUMBER +
        std::to_string(first)
      );
    }
  }

  void SimpleParser::parseProcedure() {
    prevStmts.clear();

    int start = getStmtNum();

    // grammar: 'procedure' proc_name '{' '}'
    validate(PROCEDURE);
    std::string procName = validate(NAME);
    setCurrentProc(procName);
    if (parsedProcs.count(procName) == 1) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_DUPLICATED_PROCEDURE_NAME +
        ErrorMessage::APPEND_STMT_NUMBER +
        procName
      );
    }
    parsedProcs.insert(procName);
    validate(LEFT_BRACE);
    parseStmtLst(0, getStmtNum());
    validate(RIGHT_BRACE);

    std::vector<int> end;
    for (int stmt : prevStmts) {
      end.push_back(stmt);
    }

    // adding to pkb
    pkb.addProc(procName); // add procedures
    pkb.addProcStartEnd(procName, start, end);
    pkb.addProcRange(procName, start, getStmtNum() - 1);
  }

  void SimpleParser::parseProgram() {
    // Multiple procedure allowed for iteration 2 onwards
    while (!tokens.empty()) {
      parseProcedure();
    }
  }

  SimpleParser::SimpleParser(Pkb& pkb, std::list<Token> tokens)
    : pkb(pkb), tokens(tokens) {
  };

  void SimpleParser::parse() {
    parseProgram();
  }
}