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
  // =================================
  // General - Used for type checking
  // =================================
  const static Token IDENTIFIER{ TokenType::IDENTIFIER, "" };
  const static Token NUMBER{ TokenType::NUMBER, "" };
  const static Token WHITESPACE{ TokenType::WHITESPACE, "" };

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

  // Symbols
  const static Token COMMA{ TokenType::DELIMITER, "," };
  const static Token SEMICOLON{ TokenType::DELIMITER, ";" };
  const static Token UNDERSCORE{ TokenType::DELIMITER, "_" };
  const static Token LEFT_PARENTHESIS{ TokenType::DELIMITER, "(" };
  const static Token RIGHT_PARENTHESIS{ TokenType::DELIMITER, ")" };
  const static Token QUOTE{ TokenType::DELIMITER, "\"" };
  const static Token STAR{ TokenType::OPERATOR, "*" };


  // design-entity : ‘stmt’ | ‘read’ | ‘print’ | ‘while’ | ‘if’ | ‘assign’ | ‘variable’ | ‘constant’ | ‘procedure’
  const static Token STMT{ TokenType::IDENTIFIER, "stmt" };
  const static Token READ{ TokenType::IDENTIFIER, "read" };
  const static Token PRINT{ TokenType::IDENTIFIER, "print" };
  const static Token WHILE{ TokenType::IDENTIFIER, "while" };
  const static Token IF{ TokenType::IDENTIFIER, "if" };
  const static Token ASSIGN{ TokenType::IDENTIFIER, "assign" };
  const static Token VARIABLE{ TokenType::IDENTIFIER, "variable" };
  const static Token CONSTANT{ TokenType::IDENTIFIER, "constant" };
  const static Token PROCEDURE{ TokenType::IDENTIFIER, "procedure" };

  // relRef: ModifiesS | UsesS | Parent | ParentT | Follows | FollowsT
  const static Token MODIFIES{ TokenType::IDENTIFIER, "Modifies" };
  const static Token USES{ TokenType::IDENTIFIER, "Uses" };
  const static Token PARENT{ TokenType::IDENTIFIER, "Parent" };
  const static Token FOLLOWS{ TokenType::IDENTIFIER, "Follows" };

  /**
   * Map that maps a given token to the corresponding entity type.
   */
  const static std::map<Token, Pql::EntityType> tokenToDesignEntityTypeMapper({
    {Pql::STMT, Pql::EntityType::STMT},
    {Pql::READ, Pql::EntityType::READ},
    {Pql::PRINT, Pql::EntityType::PRINT},
    {Pql::WHILE, Pql::EntityType::WHILE},
    {Pql::IF, Pql::EntityType::IF},
    {Pql::ASSIGN, Pql::EntityType::ASSIGN},
    {Pql::VARIABLE, Pql::EntityType::VARIABLE},
    {Pql::CONSTANT, Pql::EntityType::CONSTANT},
    {Pql::PROCEDURE, Pql::EntityType::PROCEDURE}
    });

  /**
   * Set that contains the entity types that refers to a statement.
   */
  const static std::unordered_set<Pql::EntityType> synonymStmtEntityTypes({
    Pql::EntityType::STMT,
    Pql::EntityType::READ,
    Pql::EntityType::PRINT,
    Pql::EntityType::WHILE,
    Pql::EntityType::IF,
    Pql::EntityType::ASSIGN
    });

  /**
   * Parser for the PQL query.
   */
  class PqlParser {
  private:
    std::list<Token> tokens;
    std::unordered_map<std::string, Pql::EntityType> declaredSynonyms;

  public:
    /**
     * Constructs a PQL Parser with a list of tokens from the tokenised PQL query.
     *
     * @param tokens List of tokens to be parsed.
     */
    PqlParser(const std::list<Token>& tokens);

    /**
    * Parses the PQL query and returns a query representation object.
    *
    * @return Query representation object.
    */
    Pql::Query parseQuery();

  private:
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
    void parseDeclarationSynonym(const Pql::EntityType& designEntityType);

    /**
     * Parses the body of a PQL query and stores the query representation in the given query that is
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseBody(Pql::Query& queryUnderConstruction);

    /**
     * Parses the select target of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseSelectTarget(Pql::Query& queryUnderConstruction);

    /**
     * Parses all the clauses of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseClauses(Pql::Query& queryUnderConstruction);

    /**
     * Parses a such that clause of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parseSuchThatClause(Pql::Query& queryUnderConstruction);

    /**
     * Parses a pattern clause of the PQL query and stores the result in the query representation
     * under construction.
     *
     * @param queryUnderConstruction Query representation object that is under construction.
     */
    void parsePatternClause(Pql::Query& queryUnderConstruction);

    /**
     * Parses a pair of stmtRef, stmtRef arguments from the clause of the PQL query and stores the
     * result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseStmtAndStmtArgs(Pql::Clause& clauseUnderConstruction);

    /**
     * Parses a pair of stmtRef, entRef arguments from the clause of the PQL query and stores the
     * result in the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseStmtAndEntArgs(Pql::Clause& clauseUnderConstruction);

    /**
     * Parses a stmtRef argument from the clause of the PQL query and stores the result in the
     * clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseStmtRef(Pql::Clause& clauseUnderConstruction);

    /**
     * Parses an entRef argument from the clause of the PQL query and stores the result in the
     * clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseEntRef(Pql::Clause& clauseUnderConstruction);

    /**
     * Parses an expression-spec argument from the clause of the PQL query and stores the result in
     * the clause representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     */
    void parseExprSpec(Pql::Clause& clauseUnderConstruction);

    /**
     * Parses an expression from the clause of the PQL query and stores the result in the clause
     * representation under construction.
     *
     * @param clauseUnderConstruction Clause representation object that is under construction.
     * @param isExactMatch True to parse the expression as type EXPRESSION, else type will be SUB_EXPRESSION.
     */
    void parseExpression(Pql::Clause& clauseUnderConstruction, bool isExactMatch);

    /**
     * Validate and get the next token against the given token that should be expected. Also consume any 
     * following whitespaces. If the given token has a value, check for an exact match. Otherwise, check 
     * that the token type matches.
     * 
     * NOTE: Validation and consumption of whitespaces are bundled into a function for convenience because
     * only in rare ocassion, consumption of whitespaces is not needed. In most cases, validation and 
     * consumption of whitespaces are performed in sequence. 
     *
     * @param token Token to be validated against.
     * @return Next token if valid. Otherwise, throw a SyntaxError.
     */
    Token validateAndGetAndConsumeWhitespaces(const Token& validationToken);

    /**
     * Validate and get the next token against the given token that should be expected.
     * If the given token has a value, check for an exact match. Otherwise, check that the token
     * type matches.
     *
     * @param token Token to be validated against.
     * @return Next token if valid. Otherwise, throw a SyntaxError.
     */
    Token validateAndGet(const Token& validationToken);

    /**
     * Consume all the whitespace tokens found at the front of tokens if any,
     * until a non-whitespace token is found.
     */
    void consumeFrontWhitespaceTokens();

    /**
     * Checks if the given synonym has already been declared.
     *
     * @param synonymName Name of the synonym to be checked.
     * @return True if the given synonym name has already been declared. Otherwise, false.
     */
    bool isSynonymDeclared(const std::string& synonymName) const;
  };
}
