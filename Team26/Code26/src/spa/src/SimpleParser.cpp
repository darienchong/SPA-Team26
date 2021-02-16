#include "SimpleParser.h"

#include <list>
#include <string>
#include <vector>

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

  // Might need to modify error message!!!
  void SimpleParser::removeFrontToken() {
    if (tokens.empty()) {
      throw "No more tokens to remove!";
    }
    tokens.pop_front();
  }

  // Might need to modify error message!!!
  Token SimpleParser::getFrontToken() {
    if (tokens.empty()) {
      throw "Syntax Error! Not enough tokens!";
    }
    return tokens.front();
  }

  void SimpleParser::setCurrentProc(std::string procName) {
    currentProc = procName;
  }

  std::string SimpleParser::getCurrentProc() {
    return currentProc;
  }

  // TODO: modify error messages to look "nicer"
  std::string SimpleParser::validate(const Token& token) {
    Token front = getFrontToken();
    removeFrontToken();

    if (front == token || (token.type == front.type && token.value.empty())) {
      return front.value;
    }

    throw "Syntax Error! Expected XXX but got YYY!";
  }

  void SimpleParser::validateNumToken() {
    Token front = getFrontToken();

    if (front.value.size() > 1 && front.value.front() == '0') {
      throw "Syntax Error! Numbers cannot start with 0!";
    }
  }

  // TODO: uncomment the method pkb.addConst() after pkb side finish its implementation
  std::string SimpleParser::parseAssignExpr() {
    std::list<Token> infixExprTokens;
    while (!tokens.empty()) {
      Token next = getFrontToken();
      if (next == SEMICOLON) { // encountered ';'
        break;
      }
      if (next.type == CONST.type) {
        validateNumToken();
      }
      infixExprTokens.emplace_back(next);
      removeFrontToken();
    }

    ExprProcessor::AssignExprParser exprParser(infixExprTokens);
    exprParser.parse();
    std::unordered_set<std::string> variablesUsed = exprParser.getVariables();
    std::unordered_set<std::string> constantsUsed = exprParser.getConstants();

    // add variables and Uses relation for stmt-var to pkb
    for (const std::string& variable : variablesUsed) {
      pkb.addUses(getStmtNum(), variable);
      pkb.addUses(getCurrentProc(), variable);
      pkb.addVar(variable);
    }
    // add constants to pkb
    for (const std::string& constants : constantsUsed) {
      //pkb.addConst(constants); // method not implemented yet
    }

    return exprParser.getPostfixExprString();
  }

  // TODO: uncomment call to pkb.addConst() after pkb side finish implementing!!!!
  void SimpleParser::parseCondExpr() {
    std::list<Token> infixExprTokens;
    // might need to change
    auto it = tokens.begin();
    while (it != tokens.end() && std::next(it) != tokens.end()) {
      Token current = getFrontToken();
      Token next = *std::next(it);

      bool isEndOfCondExpr = current == RIGHT_PARENTHESIS && (next == THEN || next == LEFT_BRACE);
      if (isEndOfCondExpr) {
        break;
      }
      if (current.type == CONST.type) {
        validateNumToken();
      }
      infixExprTokens.emplace_back(current);
      it++;
      removeFrontToken();
    }

    ExprProcessor::CondExprParser exprParser(infixExprTokens);
    exprParser.parse();
    std::unordered_set<std::string> variablesUsed = exprParser.getVariables();
    std::unordered_set<std::string> constantsUsed = exprParser.getConstants();

    // adding information to pkb
    for (const std::string& variable : variablesUsed) {
      pkb.addUses(getStmtNum(), variable); // add Uses relation for stmt-var
      pkb.addUses(getCurrentProc(), variable); // add Uses relation for proc-var
      pkb.addVar(variable); // add vars
    }
    for (const std::string& constants : constantsUsed) {
      //pkb.addConst(constants); // add consts
    }
  }

  int SimpleParser::parseIf() {
    // grammar: 'if' '(' cond_expr ')'
    validate(IF);
    validate(LEFT_PARENTHESIS);
    parseCondExpr();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addIf(stmtNum); // add if stmts

    // grammar: 'then' '{' stmtLst '}' 'else' '{' stmtLst '}'
    validate(THEN);
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);
    validate(ELSE);
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

    return stmtNum;
  }

  int SimpleParser::parseWhile() {
    // grammar: 'while' '(' cond_expr ')'
    validate(WHILE);
    validate(LEFT_PARENTHESIS);
    parseCondExpr();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addWhile(stmtNum); // add while stmts

    // grammar: '{' stmtLst '}'
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

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
    pkb.addVar(varName); // add variable
    pkb.addRead(stmtNum); // add read stmts
    pkb.addModifies(stmtNum, varName); // add Modifies relation for stmt-var
    pkb.addModifies(getCurrentProc(), varName); // add Modifies relation for proc-var

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
    pkb.addVar(varName); // add variable
    pkb.addPrint(stmtNum); // add print stmts
    pkb.addUses(stmtNum, varName); // add Uses relation for stmt-var
    pkb.addUses(getCurrentProc(), varName); // add Uses relation for proc-var

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
    pkb.addVar(varName); // add variable
    pkb.addAssign(stmtNum); /// add assign stmts
    pkb.addModifies(stmtNum, varName);
    pkb.addModifies(getCurrentProc(), varName);
    pkb.addPatternAssign(stmtNum, varName, exprString);

    return stmtNum;
  }

  // TODO: Iteration 2
  int SimpleParser::parseCall() {
    return 0;
  }

  // TODO: modify error message + change return type if needed
  int SimpleParser::parseStmt(int first) {
    Token keyword = getFrontToken();
    int stmt;

    if (keyword.type != NAME.type) {
      throw "Syntax Error! Unable to parse statement at #stmtNo";
    }
    else {
      auto it = std::next(tokens.begin());

      if (it == tokens.end()) {
        throw "Syntax Error! Not enough tokens!";
      }

      if (*it == ASSIGN_OP) {
        stmt = parseAssign();
      }
      else if (keyword == IF) {
        stmt = parseIf();
      }
      else if (keyword == WHILE) {
        stmt = parseWhile();
      }
      else if (keyword == READ) {
        stmt = parseRead();
      }
      else if (keyword == PRINT) {
        stmt = parsePrint();
      }
      else if (keyword == CALL) {
        stmt = parseCall();
      }
      else {
        throw "Syntax Error! Unknown statement type!";
      }
    }

    // adding information to pkb if first != statement - basically a statement cannot follow itself
    if (first != stmt) {
      pkb.addFollows(first, stmt); // add Follows relation for stmt-stmt
    }
    pkb.addStmt(stmt); // add stmts

    return stmt;
  }

  // TODO: modify error message if needed
  void SimpleParser::parseStmtLst(int parent, int first) {
    bool isEmpty = true;
    int stmt = first;
    while (!tokens.empty() && tokens.front() != RIGHT_BRACE) {
      stmt = parseStmt(stmt);

      // adding information to pkb if a parent statement exists
      if (parent != 0) {
        pkb.addParent(parent, stmt); // add Parents relation for stmt-stmt
      }

      isEmpty = false;
    }

    if (isEmpty) {
      throw "Syntax Error! Statement list cannot be empty!";
    }
  }

  void SimpleParser::parseProcedure() {
    // grammar: 'procedure' proc_name '{' '}'
    validate(PROCEDURE);
    std::string procName = validate(NAME);
    setCurrentProc(procName);
    pkb.addProc(procName); // add procedures
    validate(LEFT_BRACE);
    parseStmtLst(0, getStmtNum());
    validate(RIGHT_BRACE);
  }

  void SimpleParser::parseProgram() {
    // can remove this empty program check if already handled by tokeniser - ie guaranteed that the list is not empty when parsing
    if (tokens.empty()) {
      throw "Program cannot be empty!";
    }
    // code for iteration 2 onwards after multiple procs allowed
    //while (!tokens.empty()) {
    //  parseProcedure();
    //}

    // code for iteration 1
    parseProcedure();
    if (!tokens.empty()) {
      throw "Error! Extra tokens at the end of the procedure! Only 1 procedure allowed for Iteration 1!";
    }
  }

  SimpleParser::SimpleParser(Pkb& pkb, std::list<Token> tokens) : pkb(pkb), tokens(tokens) {};
  SimpleParser::~SimpleParser() {};

  void SimpleParser::parse() {
    parseProgram();
  }

  Pkb SimpleParser::getPkb() {
    return pkb;
  }
}