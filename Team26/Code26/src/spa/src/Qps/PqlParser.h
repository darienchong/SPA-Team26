#pragma once

#include <list>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <exception>
#include <string>

#include "Token.h"
#include "PqlQuery.h"

/**
 * Standard tokens in a PQL query.
 */
namespace Pql {
  /**
   * Enum used for parsing of multiple clauses.
   */
  enum class ParsingClauseType {
    UNDEFINED,
    SUCH_THAT,
    PATTERN,
    WITH
  };

  // =================================
  // General - Used for type checking
  // =================================
  const static Token IDENTIFIER{ TokenType::IDENTIFIER, "" };
  const static Token NUMBER{ TokenType::NUMBER, "" };

  // =============================================
  // Specific - Used to check for specific values
  // =============================================

  // Space character
  const static Token SPACE{ TokenType::WHITESPACE, " " };

  // Keywords
  const static Token SELECT{ TokenType::IDENTIFIER, "Select" };
  const static Token SUCH{ TokenType::IDENTIFIER, "such" };
  const static Token THAT{ TokenType::IDENTIFIER, "that" };
  const static Token PATTERN{ TokenType::IDENTIFIER, "pattern" };
  const static Token WITH{ TokenType::IDENTIFIER, "with" };
  const static Token AND{ TokenType::IDENTIFIER, "and" };
  const static Token BOOLEAN{ TokenType::IDENTIFIER, "BOOLEAN" };

  // Symbols
  const static Token COMMA{ TokenType::DELIMITER, "," };
  const static Token SEMICOLON{ TokenType::DELIMITER, ";" };
  const static Token UNDERSCORE{ TokenType::DELIMITER, "_" };
  const static Token LEFT_PARENTHESIS{ TokenType::DELIMITER, "(" };
  const static Token RIGHT_PARENTHESIS{ TokenType::DELIMITER, ")" };
  const static Token QUOTE{ TokenType::DELIMITER, "\"" };
  const static Token DOT{ TokenType::DELIMITER, "." };
  const static Token NUMBER_SIGN{ TokenType::DELIMITER, "#" };
  const static Token STAR{ TokenType::OPERATOR, "*" };
  const static Token LEFT_ANGLE_BRACKET{ TokenType::OPERATOR, "<" };
  const static Token RIGHT_ANGLE_BRACKET{ TokenType::OPERATOR, ">" };
  const static Token EQUAL{ TokenType::OPERATOR, "=" };

  // design-entity : 'stmt' | 'read' | 'print' | 'call' | 'while' | 'if' | 'assign' | 'variable' |
  //                 'constant' | 'procedure' | 'proc_line'
  const static Token STMT{ TokenType::IDENTIFIER, "stmt" };
  const static Token READ{ TokenType::IDENTIFIER, "read" };
  const static Token PRINT{ TokenType::IDENTIFIER, "print" };
  const static Token CALL{ TokenType::IDENTIFIER, "call" };
  const static Token WHILE{ TokenType::IDENTIFIER, "while" };
  const static Token IF{ TokenType::IDENTIFIER, "if" };
  const static Token ASSIGN{ TokenType::IDENTIFIER, "assign" };
  const static Token VARIABLE{ TokenType::IDENTIFIER, "variable" };
  const static Token CONSTANT{ TokenType::IDENTIFIER, "constant" };
  const static Token PROG{ TokenType::IDENTIFIER, "prog" };
  const static Token LINE{ TokenType::IDENTIFIER, "line" };
  const static Token PROCEDURE{ TokenType::IDENTIFIER, "procedure" };

  // attrName : 'procName'| 'varName' | 'value' | 'stmt#'
  const static Token PROC_NAME{ TokenType::IDENTIFIER, "procName" };
  const static Token VAR_NAME{ TokenType::IDENTIFIER, "varName" };
  const static Token VALUE{ TokenType::IDENTIFIER, "value" };

  // STMT already defined

  // relRef : ModifiesS | UsesS | Parent | ParentT | Follows | FollowsT | Calls | CallsT | Next | 
  //          NextT | Affects | AffectsT
  const static Token MODIFIES{ TokenType::IDENTIFIER, "Modifies" };
  const static Token USES{ TokenType::IDENTIFIER, "Uses" };
  const static Token PARENT{ TokenType::IDENTIFIER, "Parent" };
  const static Token FOLLOWS{ TokenType::IDENTIFIER, "Follows" };
  const static Token CALLS{ TokenType::IDENTIFIER, "Calls" };
  const static Token NEXT{ TokenType::IDENTIFIER, "Next" };
  const static Token AFFECTS{ TokenType::IDENTIFIER, "Affects" };

  /**
   * Map that maps a given token to the corresponding entity type.
   */
  const static std::map<Token, EntityType> tokenToDesignEntityTypeMapper({
      {STMT, EntityType::STMT},
      {READ, EntityType::READ},
      {PRINT, EntityType::PRINT},
      {CALL, EntityType::CALL},
      {WHILE, EntityType::WHILE},
      {IF, EntityType::IF},
      {ASSIGN, EntityType::ASSIGN},
      {VARIABLE, EntityType::VARIABLE},
      {CONSTANT, EntityType::CONSTANT},
      {PROG, EntityType::PROG_LINE},
      {PROCEDURE, EntityType::PROCEDURE}
    });

  /**
   * Map that maps a given token to the corresponding attribute reference type.
   */
  const static std::map<Token, AttributeRefType> tokenToAttributeRefTypeMapper({
      {PROC_NAME, AttributeRefType::PROC_NAME},
      {VAR_NAME, AttributeRefType::VAR_NAME},
      {VALUE, AttributeRefType::VALUE},
      {STMT, AttributeRefType::STMT_NUMBER}
    });

  /**
   * Set that contains the entity types that refers to a statement.
   */
  const static std::unordered_set<EntityType> synonymStmtEntityTypes({
      EntityType::STMT,
      EntityType::READ,
      EntityType::PRINT,
      EntityType::CALL,
      EntityType::WHILE,
      EntityType::IF,
      EntityType::ASSIGN,
      EntityType::PROG_LINE
    });

  /**
   * Parser for the PQL query.
   */
  class PqlParser {
  private:
    std::string semanticErrorMessage;
    std::list<Token>& tokens;
    std::unordered_map<std::string, EntityType> declaredSynonyms;

  public:
    /**
     * Constructs a PQL Parser with a list of tokens from the tokenised PQL query.
     *
     * @param tokens List of tokens to be parsed.
     */
    PqlParser(std::list<Token>& tokens);

    /**
    * Parses the PQL query and returns a query representation object.
    *
    * @return Query representation object.
    */
    Query parseQuery();

  private:
    /**
     * Adds a semantic error message to the record in semanticErrorMessages.
     * These messages will be added to the query object at the end.
     */
    void addSemanticErrorMessage(const std::string& message);

    /**
     * Parses all the declaration statements of a PQL query.
     */
    void parseDeclarations();

    /**
     * Parses a single declaration statement of a PQL query.
     */
    void parseDeclaration();

    /**
     * Parses a single declared synonym as the given design entity type.
     *
     * @param designEntityType Design entity type of the synonym to be parsed.
     */
    void parseDeclarationSynonym(const EntityType& designEntityType);

    /**
     * Parses the body of a PQL query and stores the query representation in the given query that is
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseBody(Query& queryUnderConstruction);

    /**
     * Parses the select targets of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseSelectTargets(Query& queryUnderConstruction);

    /**
     * Parses a single select target of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseSelectTarget(Query& queryUnderConstruction);

    /**
     * Parses all the clauses of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseClauses(Query& queryUnderConstruction);

    /**
     * Parses a such that clause of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseSuchThatClause(Query& queryUnderConstruction);

    /**
     * Parses a pattern clause of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parsePatternClause(Query& queryUnderConstruction);

    /**
     * Parses a with clause of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseWithClause(Query& queryUnderConstruction);

    /**
     * Parses the Follows clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseFollowsClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Follows* clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseFollowsTClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Parent clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseParentClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Parent* clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseParentTClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Calls clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseCallsClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Calls* clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseCallsTClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Next clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseNextClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Next* clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseNextTClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Affects clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseAffectsClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Affects* clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseAffectsTClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Uses clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseUsesClause(Clause& clauseUnderConstruction);

    /**
     * Parses the Modifies clause and its parameters and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseModifiesClause(Clause& clauseUnderConstruction);

    /**
     * Helper method used by parseUsesClause() and parseModifiesClause() that parses both Uses and 
     * Modifies clauses according to the given clause types. Stores the result in the clause representation
     * under construction.
     * 
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param procedureType UsesP or ModifiesP clause type.
     * @param stmtType UsesS or ModifiesS clause type.
     */
    void parseUsesModifiesClause(Clause& clauseUnderConstruction, const ClauseType& procedureType, const ClauseType& stmtType);

    /**
     * Parses a pattern assign clause of the PQL query and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param synonymName Synonym name of the pattern if clause.
     */
    void parsePatternAssignClause(Clause& clauseUnderConstruction, const std::string& synonymName);

    /**
     * Parses a pattern if clause of the PQL query and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param synonymName Synonym name of the pattern if clause.
     */
    void parsePatternIfClause(Clause& clauseUnderConstruction, const std::string& synonymName);

    /**
     * Parses a pattern while clause of the PQL query and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param synonymName Synonym name of the pattern while clause.
     */
    void parsePatternWhileClause(Clause& clauseUnderConstruction, const std::string& synonymName);

    /**
     * Parses a semantically invalid pattern clause of the PQL query and stores the result in the clause representation
     * under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param synonymName Synonym name.
     * @param synonymType Synonym type.
     */
    void parsePatternInvalidClause(Clause& clauseUnderConstruction, const std::string& synonymName, const EntityType& synonymType);

    /**
     * Parses a pair of stmtRef, stmtRef arguments from the clause of the PQL query and stores the
     * result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseStmtAndStmtArgs(Clause& clauseUnderConstruction);

    /**
     * Parses a pair of procRef, procRef arguments from the clause of the PQL query and stores the
     * result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseProcAndProcArgs(Clause& clauseUnderConstruction);

    /**
     * Parses a ref argument from the with clause of the PQL query and stores the result in the
     * clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseRef(Clause& clauseUnderConstruction);

    /**
     * Parses a stmtRef argument from the clause of the PQL query and stores the result in the
     * clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseStmtRef(Clause& clauseUnderConstruction);

    /**
     * Parses a varRef (variable type entRef) argument from the clause of the PQL query and stores
     * the result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseVarRef(Clause& clauseUnderConstruction);

    /**
     * Parses a procRef (variable type entRef) argument from the clause of the PQL query and stores
     * the result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseProcRef(Clause& clauseUnderConstruction);

    /**
     * Helper method that parses an entRef argument from the clause of the PQL query based on the ref 
     * type check function specified and stores the result in the clause representation under construction.
     * 
     * @param clauseUnderConstruction Clause representation object that is under construction. 
     * @param refTypeCheck Function that checks the synonym type of the entRef
     */
    void parseEntRef(Clause& clauseUnderConstruction, bool refTypeCheck(const Pql::EntityType&));

    /**
     * Parses an expression-spec argument from the clause of the PQL query and stores the result in
     * the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseExprSpec(Clause& clauseUnderConstruction);

    /**
     * Parses an expression from the clause of the PQL query and stores the result in the clause
     * representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param isExactMatch True to parse the expression as type EXPRESSION, else type will be SUB_EXPRESSION.
     */
    void parseExpression(Clause& clauseUnderConstruction, const bool isExactMatch);

    /**
     * Validate and get the next token against the given token that should be expected.
     * If the given token has a value, check for an exact match. Otherwise, check that the token
     * type matches. Consumes any trailing whitespaces.
     *
     * @param token Token to be validated against.
     * @return Next token if valid. Otherwise, throw a SyntaxError.
     */
    Token validateAndGet(const Token& validationToken);

    /**
     * Validate and get the next token against the given token that should be expected.
     * If the given token has a value, check for an exact match. Otherwise, check that the token
     * type matches. shouldConsumeWhitespaces can be set it indicate whether trailing whitespaces
     * should be consumed.
     *
     * @param token Token to be validated against.
     * @param shouldConsumeWhitespaces
     * @return Next token if valid. Otherwise, throw a SyntaxError.
     */
    Token validateAndGet(const Token& validationToken, const bool shouldConsumeWhitespaces);

    /**
     * Consume all the whitespace tokens found at the front of tokens if any,
     * until a non-whitespace token is found.
     */
    void consumeFrontWhitespaceTokens();

    /**
     * Checks if there are anymore tokens and returns the front token.
     * Does not consume the front token.
     * Throws a Syntax Error if there are no more tokens left.
     * 
     * @return Front token.
     */
    Token getFrontToken();

    /**
     * Gets the synonym type from the specified synonym name.
     * This method will check if the synonym has been declared and adds to the semantic error
     * messages if the synonym is not declared. 
     *
     * @param synonymName Name of the synonym.
     * @return Entity type that corresponds to the given synonym name.
     */
    EntityType getSynonymType(const std::string& synonymName);

    /**
     * Checks if the given synonym has already been declared.
     *
     * @param synonymName Name of the synonym to be checked.
     * @return True if the given synonym name has already been declared. Otherwise, false.
     */
    bool isSynonymDeclared(const std::string& synonymName) const;
  };
}
