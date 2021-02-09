#include "SimpleParser.h"

#include <list>
#include <string>
#include <vector>

#include "ExprParser.h"
#include "PKB.h"
#include "PkbStub.h" // remove this once pkb is implemented

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

  // Identifier
  const static Token NAME{ TokenType::IDENTIFIER, "" };

  /**
   * Increments the statement number.
   *
   * @returns int The current statement number.
   */
  void SimpleParser::incStmtNum() {
    ++stmtNum;
  }

  /**
   * Gets the current statement number.
   *
   * @returns int The current statement number.
   */
  int SimpleParser::getStmtNum() {
    return stmtNum;
  }

  // TODO: Might need to add error checking to handle empty list of tokens!!!
  /**
   * Removes the first token in std::list<Token> token list.
   */
  void SimpleParser::removeNextToken() {
    tokens.pop_front();
  }

  // TODO: Might need to add error checking to handle empty list of tokens!!!
  /**
   * Gets the first token in std::list<Token> token list WITHOUT popping it.
   *
   * @returns Token The first token in the list of tokens.
   */
  Token SimpleParser::getNextToken() {
    return tokens.front();
  }

  // TODO: modify error messages to look "nicer"
  /**
   * Checks if the first token in std::list<Token> token list matches the input token and returns its string representation if they match.
   * This check validates token type for procedure/variable names, both token type and token values otherwise.
   *
   * @param token Pointer of the input token.
   * @returns std::string The string representation of the first tokenin the list of tokens.
   */
  std::string SimpleParser::validate(const Token &token) {
    if (tokens.empty()) {
      throw "Syntax Error! Not enough tokens!";
    }

    Token front = getNextToken();
    removeNextToken();
    if (front == token || (token.type == front.type && token.value.empty())) {
      return front.value;
    }

    throw "Syntax Error! Expected XXX but got YYY!";
  }

  /**
   * Parses tokens into an assign statement expression.
   * Calls ExprParser::addToken() and parseExpr().
   *
   */
  void SimpleParser::parseAssignExpr() {
    Token next = getNextToken();
    while (!tokens.empty() && next != SEMICOLON) {
      exprParser.addToken(next);
      removeNextToken();
    }

    parseExpr();
  }

  // TODO: complete function - needs auto it to check for next next token, whether it is == THEN/{
  void SimpleParser::parseCondExpr() {
    Token next = getNextToken();
    while (!tokens.empty()) {
      exprParser.addToken(next);
      removeNextToken();
    }

    parseExpr();
  }

  // TODO: verify helper functions postFix and evalPostFix
  /**
   * Parses tokens into a list of string of valid sub-expressions.
   * Calls ExprParser::parseExpr()
   *
   */
  void SimpleParser::parseExpr() {
    exprParser.parseExpr();
  }

  // TODO: finish implementation - returns a std::list <std::string> of variables used in the cond (currently returns list of exprs), ensures correct syntax of cond_expr used
  /**
   * Parses tokens into a valid conditional expression.
   * Returns a list of variables used in the conditional expression for Uses PQL query.
   * Recursively calls itself when encountering multiple conditional expressions.
   *
   * @returns std::list<stdstd::string> The list of variables used in the conditional expression.
   */
  std::list<std::string> SimpleParser::parseCond() {
    std::list<std::string> varLst;
    Token first = getNextToken();
    removeNextToken();

    // '(' token - multiple conditional expressions
    if (first == LEFT_PARENTHESIS) {
      varLst.splice(varLst.end(), parseCond());
      validate(RIGHT_PARENTHESIS);
      Token condOp = getNextToken();
      removeNextToken();
      if (!(condOp == COND_EXPR_AND || condOp == COND_EXPR_OR)) {
        throw "Missing AND/OR between conditional expressions!";
      }
      validate(LEFT_PARENTHESIS);
      varLst.splice(varLst.end(), parseCond());
      validate(RIGHT_PARENTHESIS);
    }
    // '!' token - multiple conditional expressions
    else if (first == COND_EXPR_NOT) {
      validate(LEFT_PARENTHESIS);
      varLst.splice(varLst.end(), parseCond());
      validate(RIGHT_PARENTHESIS);
    }
    else { // is just a rel_expr
      parseCondExpr();
      varLst.splice(varLst.end(), exprParser.getVariables());

      exprParser.clear();

      Token next = getNextToken();
      removeNextToken();
      if (!(next == REL_EXPR_OP_EQV || next == REL_EXPR_OP_GRT || next == REL_EXPR_OP_GEQ || next == REL_EXPR_OP_LET || next == REL_EXPR_OP_LEQ || next == REL_EXPR_OP_NEQ)) {
        throw "Missing valid operators between relational expressions!";
      }

      parseCondExpr();
      varLst.splice(varLst.end(), exprParser.getVariables());

      exprParser.clear();
    }

    return varLst;
  }

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into an IF statement and returns its statement number. 
   *
   * @returns int Statement number of the IF statement.
   */
  int SimpleParser::parseIf() {
    // grammar: 'if' '(' cond_expr ')'
    validate(IF);
    validate(LEFT_PARENTHESIS);
    std::list<std::string> varLst = parseCond();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addIf(stmtNum); // stub to add if stmts
    for (std::string var : varLst) {
      pkb.addUses(stmtNum, var); // stub for Uses relation
    }

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

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into a WHILE statement and returns its statement number.
   *
   * @returns int Statement number of the WHILE statement.
   */
  int SimpleParser::parseWhile() {
    // grammar: 'while' '(' cond_expr ')'
    validate(WHILE);
    validate(LEFT_PARENTHESIS);
    std::list<std::string> varLst = parseCond();
    validate(RIGHT_PARENTHESIS);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addWhile(stmtNum); // stub to add while stmts
    for (std::string var : varLst) {
      pkb.addUses(stmtNum, var); // stub for Uses relation
    }

    // grammar: '{' stmtLst '}'
    validate(LEFT_BRACE);
    parseStmtLst(stmtNum, getStmtNum());
    validate(RIGHT_BRACE);

    return stmtNum;
  }

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into a READ statement and returns its statement number.
   *
   * @returns int Statement number of the READ statement.
   */
  int SimpleParser::parseRead() {
    // grammar: 'read' var_name ';'
    validate(READ);
    std::string varName = validate(NAME);
    validate(SEMICOLON);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addRead(stmtNum); // stub to add read stmts
    pkb.addModifies(stmtNum, varName); // stub for Modifies relation

    return stmtNum;
  }

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into a PRINT statement and returns its statement number.
   *
   * @returns int Statement number of the PRINT statement.
   */
  int SimpleParser::parsePrint() {
    // grammar: 'print' var_name ';'
    validate(PRINT);
    std::string varName = validate(NAME);
    validate(SEMICOLON);

    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addPrint(stmtNum); // stub to add print stmts
    pkb.addUses(stmtNum, varName); // stub for Uses relation

    return stmtNum;
  }

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into an ASSIGN statement and returns its statement number.
   * Calls parseExpr() to parse the right hand side of the assign statement.
   * Calls getVariables() to retrieve variables used in the expression.
   *
   * @returns int Statement number of the ASSIGN statement.
   */
  int SimpleParser::parseAssign() {
    // grammar: var_name '=' expr ';'
    std::string varName = validate(NAME);
    validate(ASSIGN_OP);
    parseAssignExpr();
    validate(SEMICOLON);

    // Getting relevant info to add to pkb
    std::list<std::string> varLst = exprParser.getVariables();
    int stmtNum = getStmtNum();
    incStmtNum();

    // adding information to pkb
    pkb.addAssign(stmtNum); // stub to add assign stmts
    pkb.addUses(stmtNum, varName);
    for (std::string var : varLst) {
      pkb.addModifies(stmtNum, var); // stub for Modifies relation
    }

    // clean up
    exprParser.clear();

    return stmtNum;
  }

  // TODO: Iteration 2
  int SimpleParser::parseCall() {
    return 0;
  }

  // TODO: modify error message + change return type if needed
  /**
   * Parses tokens into a statement if valid and returns its statement number.
   * Statements are as follow - if/ while/ read/ print/ call/ assign.
   * Calls the corresponding parseIf(), parseWhile(), parseRead(), parsePrint(), parseCall() and parseAssign() functions.
   * Sets the FollowsT relation for the first statement and the current statement in the same statement list.
   *
   * @param first Statement number of the first statement in the current statement list. Used to set FollowsT() relation.
   * @returns int Statement number of the parsed statement.
   */
  int SimpleParser::parseStmt(int first) {
    Token keyword = getNextToken();
    int stmt;

    if (keyword == IF) {
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
    else if (keyword.type == NAME.type) {
      stmt = parseAssign();
    }
    else {
      throw "Syntax Error! Unable to parse statement at #stmtNo";
    }

    // adding information to pkb if first != statement - basically a statement cannot follow itself
    if (first != stmt) {
      pkb.addFollowsT(first, stmt); // stub for followsT relation
    }

    return stmt;
  }

  // TODO: replace stubs for pkb population & modify error message if needed
  /**
   * Parses tokens into a statement list and checks if the parsed statement list is empty.
   * Calls parseStmt() to parse statements in the statement list.
   * Sets the Parent relation if the parent statement exists.
   *
   * @param parent Statement number of the parent statement. 0 for no parent statement.
   * @param first Statement number of the first statement in the statement list.
   */
  void SimpleParser::parseStmtLst(int parent, int first) {
    bool isEmpty = true;
    while (!tokens.empty() && tokens.front() != RIGHT_BRACE) {
      int stmt = parseStmt(first);

      // adding information to pkb if a parent statement exists
      if (parent != 0) {
        pkb.addParent(parent, stmt); // stub for parents relation
      }

      isEmpty = false;
    }

    if (isEmpty) { // can replace with counter if theres no need to add the stmtlst anymore
      throw "Syntax Error! Statement list cannot be empty!";
    }
  }

  // TODO: replace stubs for pkb population
  /**
   * Parses tokens into a procedure if valid.
   * Calls parseStmtLst() to parse the statements in the procedure.
   *
   */
  void SimpleParser::parseProcedure() {
    // grammar: 'procedure' proc_name '{' '}'
    validate(PROCEDURE);
    std::string procName = validate(NAME);
    pkb.addProc(procName); // stub to add procedures
    validate(LEFT_BRACE);
    parseStmtLst(0, getStmtNum());
    validate(RIGHT_BRACE);
  }

  /**
   * Parses tokens in the program.
   * Calls parseProcedures() to parse procedures in the input program.
   * Might be more useful in Iteration 2 onwards.
   *
   */
  void SimpleParser::parseProgram() {
    // can remove this empty program check if already handled by tokeniser - ie guaranteed that the list is not empty when parsing
    if (tokens.empty()) {
      throw "Program cannot be empty!";
    }
    while (!tokens.empty()) {
      parseProcedure();
    }
  }

  SimpleParser::SimpleParser(std::list<Token> tokens) : tokens(tokens) {};
  SimpleParser::~SimpleParser() {};

  void SimpleParser::parse() {
    parseProgram();
  }
}