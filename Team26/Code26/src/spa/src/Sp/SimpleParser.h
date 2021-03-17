#pragma once

#include <list>
#include <string>
#include <vector>

#include "ExprParser.h"
#include "Pkb.h"
#include "Token.h"

namespace SourceProcessor {
  class SimpleParser {
  private:
    // Class variables
    std::string currentProc;
    std::list<Token> tokens;
    int stmtNum = 1;
    Pkb& pkb;
    std::vector<int> prevStmts;

    // Functions

    /**
     * Increments the statement number.
     *
     */
    void incStmtNum();

    /**
     * Gets the current statement number.
     *
     * @returns int The current statement number.
     */
    int getStmtNum();

    /**
     * Removes the first token in std::list<Token> token list.
     */
    void removeFrontToken();

    /**
     * Gets the first token in std::list<Token> token list WITHOUT removing it.
     *
     * @returns Token The first token in the list of tokens.
     */
    Token getFrontToken();

    /**
     * Sets the name of the current procedure for filling in of procedural relations in Pkb.
     *
     * @param procName The name of the procedure in string representation.
     */
    void setCurrentProc(std::string procName);

    /**
     * Returns the name of the current procedure.
     *
     * @returns std::string The string representation of the name of the current procedure.
     */
    std::string getCurrentProc();

    /**
     * Checks if the first token in std::list<Token> token list matches the input token and returns its string representation if they match.
     * This check validates token type for procedure/variable names, both token type and token values otherwise.
     *
     * @param validationToken Reference of the input token used for validation.
     * @returns std::string The string representation of the first token in the list of tokens.
     */
    std::string validate(const Token& validationToken);

    /**
     * Checks if the token is a valid constant not starting with '0',
     * if the first token in std::list<Token> token list is of TokenType::NUMBER.
     * Example: '0' and '42' are valid tokens, while '042' is not.
     *
     */
    void validateNumToken();

    /**
     * Parses tokens into an assign statement expression (right hand side of the '=') and
     * returns the postfix expression in std::string representation.
     * Calls AssignExprParser::parse() to parse the tokens, getVariables() and getConstants() to get
     * the variables and constants used in the expression respectively, and getPostfixExprString() to
     * get the postfix of the assign expression.
     * Adds the Uses relation for stmt-var and proc-var, and variables and constants present into the Pkb.
     *
     * @returns std::string Expression in postfix.
     */
    std::string parseAssignExpr();

    /**
     * Parses tokens into a conditional expression.
     * Calls CondExprParser::parse() to parse the tokens, getVariables() and getConstants() to get
     * the variables and constants used in the expression respectively.
     * Adds the Uses relation for stmt-var and proc-var, and variables and constants present into the Pkb.
     *
     * @returns std::unordered_set<std::string> The set of control variables in the conditional expression.
     */
    std::unordered_set<std::string> parseCondExpr();

    /**
     * Parses tokens into an IF statement and returns its statement number.
     * Calls parseCondExpr() to parse its conditional expression,
     * then parseStmtLst() TWICE to parse the statements in its TWO statement lists.
     * Adds the if statement into the pkb.
     *
     * @returns int Statement number of the IF statement.
     */
    int parseIf();

    /**
     * Parses tokens into a WHILE statement and returns its statement number.
     * Adds the while statement into the pkb.
     *
     * @returns int Statement number of the WHILE statement.
     */
    int parseWhile();

    /**
     * Parses tokens into a CALL statement and returns its statement number.
     * Adds the call statement and proc-proc relation into the Pkb.
     *
     * @returns int Statement number of the CALL statement.
     */
    int parseCall();

    /**
     * Parses tokens into a READ statement and returns its statement number.
     * Adds the read statement and Modifies relation for stmt-var and proc-var into the Pkb.
     *
     * @returns int Statement number of the READ statement.
     */
    int parseRead();

    /**
     * Parses tokens into a PRINT statement and returns its statement number.
     * Adds the print statement and Uses relation for stmt-var and proc-var into the Pkb.
     *
     * @returns int Statement number of the PRINT statement.
     */
    int parsePrint();

    /**
     * Parses tokens into an ASSIGN statement and returns its statement number.
     * Calls parseAssignExpr() to parse the right hand side of the assign statement.
     * Adds the assign statement, Modifies relation for stmt-var and proc-var, and Pattern information into the Pkb.
     *
     * @returns int Statement number of the ASSIGN statement.
     */
    int parseAssign();

    // TODO: For Iteration 2
    //int parseCall();

    /**
     * Parses tokens into a statement if valid and returns its statement number.
     * Statements are as follow - if/ while/ read/ print/ call/ assign.
     * Calls the corresponding parseIf(), parseWhile(), parseRead(), parsePrint(), parseCall() and parseAssign() functions.
     * Adds the Follows relation for the earlier statement and the current statement in the same statement list and
     * the current statement into the Pkb.
     *
     * @param first Statement number of the first statement in the current statement list. Used to set FollowsT() relation.
     * @returns int Statement number of the parsed statement.
     */
    int parseStmt(int first);

    /**
     * Parses tokens into a statement list and checks if the parsed statement list is empty.
     * Calls parseStmt() to parse statements in the statement list.
     * Sets the Parent relation if the parent statement exists.
     *
     * @param parent Statement number of the parent statement. 0 for no parent statement.
     * @param first Statement number of the first statement in the statement list.
     */
    void parseStmtLst(int parent, int first);

    /**
     * Parses tokens into a procedure if valid.
     * Calls parseStmtLst() to parse the statements in the procedure.
     *
     */
    void parseProcedure();

    /**
     * Parses tokens in the program.
     * Calls parseProcedure() to parse procedures in the input program.
     * Might be more useful in Iteration 2 onwards.
     *
     */
    void parseProgram();

  public:
    SimpleParser(Pkb& pkb, std::list<Token> tokens);

    void parse();
  };
}