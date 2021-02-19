#include "PqlParser.h"

#include <list>
#include <string>

#include "PqlQuery.h"
#include "Token.h"
#include "ExprParser.h"
#include "SpaException.h"

// Helper methods
namespace {
  /**
   * Checks whether the given token is a valid design entity.
   * Iteration 1 design-entity : ‘stmt’ | ‘read’ | ‘print’ | ‘while’ | ‘if’ | ‘assign’ | ‘variable’ | ‘constant’ | ‘procedure’
   *
   * @param token Token to verify.
   * @return True if the token is valid. Otherwise, false.
   */
  bool isValidDesignEntity(const Token& token) {
    return bool(Pql::tokenToDesignEntityTypeMapper.count(token));
  }

  /**
   * Checks whether the given token is a relation that can have a transitive version. i.e. Follows and Parent.
   *
   * @param relationToken Token to verify.
   * @return True if the token is a relation that can have a transitive version. Otherwise, false.
   */
  bool canBeTransitive(const Token& relationToken) {
    return relationToken == Pql::FOLLOWS || relationToken == Pql::PARENT;
  }

  /**
   * Checks whether the given entity type is a synonym that refers to a statement.
   *
   * @param entityType Entity type to verify.
   * @return True if the entity type refers to a statement. Otherwise, false.
   */
  bool isStmtRef(const Pql::EntityType& entityType) {
    return bool(Pql::synonymStmtEntityTypes.count(entityType));
  }

  /**
   * Checks whether the given entity type is a synonym that refers to a variable.
   *
   * @param entityType Entity type to verify.
   * @return True if the entity type refers to a variable. Otherwise, false.
   */
  bool isEntRef(const Pql::EntityType& entityType) {
    return entityType == Pql::EntityType::VARIABLE;
  }

  /**
   * Converts infix expression tokens into postfix expression as a whitespace delimited string.
   *
   * @param infixExpressionTokens List of infix expression tokens.
   * @return Postfix expression string.
   */
  std::string infixToPostfixExpression(std::list<Token>& infixExpressionTokens) {
    ExprProcessor::AssignExprParser exprParser(infixExpressionTokens);
    exprParser.parse();
    return exprParser.getPostfixExprString();
  }
}

namespace Pql {
  // Constructor
  PqlParser::PqlParser(const std::list<Token>& tokens)
    : tokens(tokens) {}

  Pql::Query PqlParser::parseQuery() {
    Pql::Query pqlQuery; // Mutating pqlOuery object directly to avoid unnecessary copying.

    consumeFrontWhitespaceTokens(); // Handles possible whitespace before declarations

    parseDeclarations();

    parseBody(pqlQuery);

    // Check for unexpected tokens at the end of the query
    if (!tokens.empty()) {
      throw Pql::SyntaxError(
        Pql::ErrorMessage::SYNTAX_ERROR_ADDITIONAL_TOKENS +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        tokens.front().value
      );
    }

    return pqlQuery;
  }

  // Iteration 1
  // declaration: design-entity synonym (‘,’ synonym)* ‘;’
  // design-entity: ‘stmt’ | ‘read’ | ‘print’ | ‘while’ | ‘if’ | ‘assign’ | 
  // ‘variable’ | ‘constant’ | ‘procedure’
  // Parses the declarations of the PQL query
  void PqlParser::parseDeclarations() {
    while (!tokens.empty() && tokens.front() != Pql::SELECT) {
      parseDeclaration();
    }
  }

  // declaration: design-entity synonym (‘,’ synonym)* ‘;’
  void PqlParser::parseDeclaration() {
    // Parses design-entity
    const Token& designEntityToken = validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

    if (!isValidDesignEntity(designEntityToken)) {
      throw Pql::SyntaxError(
        Pql::ErrorMessage::SYNTAX_ERROR_INVALID_DESIGN_ENTITY +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        designEntityToken.value
      );
    }

    const Pql::EntityType& designEntityType = Pql::tokenToDesignEntityTypeMapper.at(designEntityToken);

    // Parses first synonym
    parseDeclarationSynonym(designEntityType);

    // Parses additional synonyms
    while (!tokens.empty() && tokens.front() != Pql::SEMICOLON) {
      validateAndGetAndConsumeWhitespaces(Pql::COMMA);

      parseDeclarationSynonym(designEntityType);
    }

    validateAndGetAndConsumeWhitespaces(Pql::SEMICOLON);
  }

  // Parses the synonyms in the declarations of the PQL query and store the synonym as the specified design entity type
  void PqlParser::parseDeclarationSynonym(const Pql::EntityType& designEntityType) {
    const Token& synonymToken = validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

    // Check if synonym has been declared
    if (isSynonymDeclared(synonymToken.value)) {
      throw Pql::SemanticError(
        Pql::ErrorMessage::SEMANTIC_ERROR_DUPLICATE_SYNONYM_DECLARATION +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    // Store synonym
    declaredSynonyms[synonymToken.value] = designEntityType;
  }

  // query-body: ‘Select’ synonym [ suchthat-cl ] [ pattern-cl ]
  // Parses the PQL query body
  void PqlParser::parseBody(Pql::Query& queryUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(Pql::SELECT);

    parseSelectTarget(queryUnderConstruction);

    parseClauses(queryUnderConstruction);

  }

  // Parses and set the PQL query target
  void PqlParser::parseSelectTarget(Pql::Query& queryUnderConstruction) {
    const Token& synonymToken = validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

    // Check if synonym has been declared
    if (!isSynonymDeclared(synonymToken.value)) {
      throw Pql::SemanticError(
        Pql::ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    queryUnderConstruction.setTarget(Pql::Entity(declaredSynonyms[synonymToken.value], synonymToken.value));
  }

  // Parses the PQL query body clauses
  void PqlParser::parseClauses(Pql::Query& queryUnderConstruction) {
    if (!tokens.empty() && tokens.front() == Pql::SUCH) {
      parseSuchThatClause(queryUnderConstruction);
    }

    if (!tokens.empty() && tokens.front() == Pql::PATTERN) {
      parsePatternClause(queryUnderConstruction);
    }
  }

  // suchthat-cl: ‘such that’ relRef
  // Parses a such that clause
  void PqlParser::parseSuchThatClause(Pql::Query& queryUnderConstruction) {
    validateAndGet(Pql::SUCH);
    validateAndGet(Pql::SPACE);
    validateAndGetAndConsumeWhitespaces(Pql::THAT);

    const Token& relationToken = validateAndGet(Pql::IDENTIFIER);
    // Do not consume space yet
    // Handle Follow* and Parent* - Ensure no space between relation and *
    bool isTransitiveRelation = false;
    bool isNextTokenStar = !tokens.empty() && tokens.front() == Pql::STAR;
    if (isNextTokenStar) {
      if (!canBeTransitive(relationToken)) {
        throw Pql::SyntaxError(
          Pql::ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
          Pql::ErrorMessage::APPEND_TOKEN_EXPECTED +
          Pql::LEFT_PARENTHESIS.value +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          Pql::STAR.value
        );
      }
      validateAndGet(Pql::STAR);
      isTransitiveRelation = true;
    }
    consumeFrontWhitespaceTokens();

    // Construct the clause
    Pql::Clause clauseUnderConstruction;
    if (relationToken == Pql::FOLLOWS) {
      clauseUnderConstruction.setType(isTransitiveRelation ? Pql::ClauseType::FOLLOWS_T : Pql::ClauseType::FOLLOWS);
      parseStmtAndStmtArgs(clauseUnderConstruction);
    }
    else if (relationToken == Pql::PARENT) {
      clauseUnderConstruction.setType(isTransitiveRelation ? Pql::ClauseType::PARENT_T : Pql::ClauseType::PARENT);
      parseStmtAndStmtArgs(clauseUnderConstruction);
    }
    else if (relationToken == Pql::USES) {
      clauseUnderConstruction.setType(Pql::ClauseType::USES_S);
      parseStmtAndEntArgs(clauseUnderConstruction);
    }
    else if (relationToken == Pql::MODIFIES) {
      clauseUnderConstruction.setType(Pql::ClauseType::MODIFIES_S);
      parseStmtAndEntArgs(clauseUnderConstruction);
    }
    else {
      throw Pql::SyntaxError(
        Pql::ErrorMessage::SYNTAX_ERROR_INVALID_RELATION +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        relationToken.value
      );
    }

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // pattern-cl: ‘pattern’ syn-assign ‘(‘entRef ’,’ expression-spec ’)’
  // Parses a pattern clause
  void PqlParser::parsePatternClause(Pql::Query& queryUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(Pql::PATTERN);

    Token& assignSynonymToken = validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

    if (!isSynonymDeclared(assignSynonymToken.value)) {
      throw Pql::SemanticError(
        Pql::ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        assignSynonymToken.value
      );
    }

    bool isAssignSynonym = declaredSynonyms[assignSynonymToken.value] != Pql::EntityType::ASSIGN;
    if (isAssignSynonym) {
      throw Pql::SemanticError(
        Pql::ErrorMessage::SEMANTIC_ERROR_NON_ASSIGN_SYNONYM +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        assignSynonymToken.value
      );
    }


    Pql::Clause clauseUnderConstruction;
    clauseUnderConstruction.setType(Pql::ClauseType::PATTERN_ASSIGN);
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::ASSIGN, assignSynonymToken.value));

    validateAndGetAndConsumeWhitespaces(Pql::LEFT_PARENTHESIS);

    parseEntRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::COMMA);

    parseExprSpec(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::RIGHT_PARENTHESIS);

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // ‘(’ stmtRef ‘,’ stmtRef ‘)’
  void PqlParser::parseStmtAndStmtArgs(Pql::Clause& clauseUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(Pql::LEFT_PARENTHESIS);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::COMMA);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::RIGHT_PARENTHESIS);
  }

  // ‘(’ stmtRef ‘,’ entRef ‘)’
  void PqlParser::parseStmtAndEntArgs(Pql::Clause& clauseUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(Pql::LEFT_PARENTHESIS);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::COMMA);

    parseEntRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(Pql::RIGHT_PARENTHESIS);
  }

  // stmtRef: synonym | ‘_’ | INTEGER
  void PqlParser::parseStmtRef(Pql::Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw Pql::SyntaxError(Pql::ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR ‘_’ OR INTEGER
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw Pql::SemanticError(
          Pql::ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const Pql::EntityType entityType = declaredSynonyms[frontToken.value];
      if (!isStmtRef(entityType)) {
        throw Pql::SemanticError(
          Pql::ErrorMessage::SEMANTIC_ERROR_NON_STMT_REF +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Pql::Entity(entityType, frontToken.value));
    }
    else if (frontToken == Pql::UNDERSCORE) {
      validateAndGetAndConsumeWhitespaces(Pql::UNDERSCORE);

      clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, frontToken.value));
    }
    else if (frontToken.type == TokenType::NUMBER) {
      validateAndGetAndConsumeWhitespaces(Pql::NUMBER);

      clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::LINE_NUMBER, frontToken.value));
    }
    else {
      throw Pql::SyntaxError(
        Pql::ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // entRef: synonym | ‘_’ | ‘"’ IDENT ‘"’
  void PqlParser::parseEntRef(Pql::Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw Pql::SyntaxError(Pql::ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR ‘_’ OR ‘"’ IDENT ‘"’
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw Pql::SemanticError(
          Pql::ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const Pql::EntityType entityType = declaredSynonyms[frontToken.value];
      if (!isEntRef(entityType)) {
        throw Pql::SemanticError(
          Pql::ErrorMessage::SEMANTIC_ERROR_NON_ENT_REF +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Pql::Entity(entityType, frontToken.value));
    }
    else if (frontToken == Pql::UNDERSCORE) {
      validateAndGetAndConsumeWhitespaces(Pql::UNDERSCORE);

      clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, frontToken.value));
    }
    else if (frontToken == Pql::QUOTE) {
      validateAndGetAndConsumeWhitespaces(Pql::QUOTE);

      const Token& variableNameToken = validateAndGetAndConsumeWhitespaces(Pql::IDENTIFIER);

      validateAndGetAndConsumeWhitespaces(Pql::QUOTE);

      clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::VARIABLE_NAME, variableNameToken.value));
    }
    else {
      throw Pql::SyntaxError(
        Pql::ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // expression-spec: ‘_’ ‘"’ factor ‘"’ ‘_’ | ‘_’
  void PqlParser::parseExprSpec(Pql::Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw Pql::SyntaxError(Pql::ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }
    const Token frontToken = tokens.front();

    // Check for ‘_’ OR ‘"’
    if (frontToken == Pql::QUOTE) {
      parseExpression(clauseUnderConstruction, true);
    }
    else {
      validateAndGetAndConsumeWhitespaces(Pql::UNDERSCORE);

      if (!tokens.empty() && tokens.front() == Pql::QUOTE) {
        parseExpression(clauseUnderConstruction, false);
        validateAndGet(Pql::UNDERSCORE);
      }
      else {
        clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, "_"));
      }
      consumeFrontWhitespaceTokens();
    }
  }

  // ‘"’ factor ‘"’
  void PqlParser::parseExpression(Pql::Clause& clauseUnderConstruction, bool isExactMatch) {
    validateAndGetAndConsumeWhitespaces(Pql::QUOTE);

    // Create a list of infix expression tokens
    std::list<Token> infixExpressionTokens;
    while (!tokens.empty() && tokens.front() != Pql::QUOTE) {
      const Token currentToken = tokens.front();
      if (currentToken.type == Pql::NUMBER.type) { // Remove leading zero for const by casting to int and back to string
        const Token noLeadingZeroConstToken = { currentToken.type, std::to_string(std::stoi(currentToken.value)) };
        infixExpressionTokens.emplace_back(noLeadingZeroConstToken);
      }
      else {
        infixExpressionTokens.emplace_back(tokens.front());
      }
      tokens.pop_front();
      consumeFrontWhitespaceTokens();
    }
    validateAndGetAndConsumeWhitespaces(Pql::QUOTE);

    clauseUnderConstruction.addParam(
      Pql::Entity(
        isExactMatch ? Pql::EntityType::EXPRESSION : Pql::EntityType::SUB_EXPRESSION,
        infixToPostfixExpression(infixExpressionTokens)
      )
    );
  }

  Token PqlParser::validateAndGetAndConsumeWhitespaces(const Token& validationToken) {
    const Token& token = validateAndGet(validationToken);
    consumeFrontWhitespaceTokens();
    return token;
  }

  Token PqlParser::validateAndGet(const Token& validationToken) {
    if (tokens.empty()) {
      throw Pql::SyntaxError(Pql::ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    bool isCheckTokenType = validationToken.value.empty();

    if (isCheckTokenType) {
      // Check token type
      if (frontToken.type != validationToken.type) {
        throw Pql::SyntaxError(
          Pql::ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    }
    else {
      // Check exact match otherwise
      if (frontToken != validationToken) {
        throw Pql::SyntaxError(
          Pql::ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_VALUE +
          Pql::ErrorMessage::APPEND_TOKEN_EXPECTED +
          validationToken.value +
          Pql::ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    }

    // Validation success
    tokens.pop_front();
    return frontToken;
  }

  void PqlParser::consumeFrontWhitespaceTokens() {
    while (!tokens.empty()) {
      bool isFrontTokenWhitespace = tokens.front().type == Pql::WHITESPACE.type;

      if (isFrontTokenWhitespace) {
        tokens.pop_front();
      }
      else {
        break;
      }
    }
  }

  bool PqlParser::isSynonymDeclared(const std::string& synonymName) const {
    return bool(declaredSynonyms.count(synonymName));
  }
}