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
   * Iteration 1 design-entity : 'stmt' | 'read' | 'print' | 'while' | 'if' | 'assign' | 'variable' | 'constant' | 'procedure'
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
    : tokens(tokens) {
  }

  Query PqlParser::parseQuery() {
    Query pqlQuery; // Mutating pqlOuery object directly to avoid unnecessary copying.

    consumeFrontWhitespaceTokens(); // Handles possible whitespace before declarations

    parseDeclarations();

    parseBody(pqlQuery);

    // Check for unexpected tokens at the end of the query
    if (!tokens.empty()) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_ADDITIONAL_TOKENS +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        tokens.front().value
      );
    }

    return pqlQuery;
  }

  // Iteration 1
  // declaration: design-entity synonym (',' synonym)* ';'
  // design-entity: 'stmt' | 'read' | 'print' | 'while' | 'if' | 'assign' | 
  // 'variable' | 'constant' | 'procedure'
  // Parses the declarations of the PQL query
  void PqlParser::parseDeclarations() {
    while (!tokens.empty() && tokens.front() != SELECT) {
      parseDeclaration();
    }
  }

  // declaration: design-entity synonym (',' synonym)* ';'
  void PqlParser::parseDeclaration() {
    // Parses design-entity
    const Token& designEntityToken = validateAndGetAndConsumeWhitespaces(IDENTIFIER);

    if (!isValidDesignEntity(designEntityToken)) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_INVALID_DESIGN_ENTITY +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        designEntityToken.value
      );
    }

    const EntityType& designEntityType = tokenToDesignEntityTypeMapper.at(designEntityToken);

    // Parses first synonym
    parseDeclarationSynonym(designEntityType);

    // Parses additional synonyms
    while (!tokens.empty() && tokens.front() != SEMICOLON) {
      validateAndGetAndConsumeWhitespaces(COMMA);

      parseDeclarationSynonym(designEntityType);
    }

    validateAndGetAndConsumeWhitespaces(SEMICOLON);
  }

  // Parses the synonyms in the declarations of the PQL query and store the synonym as the specified design entity type
  void PqlParser::parseDeclarationSynonym(const EntityType& designEntityType) {
    const Token& synonymToken = validateAndGetAndConsumeWhitespaces(IDENTIFIER);

    // Check if synonym has been declared
    if (isSynonymDeclared(synonymToken.value)) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_DUPLICATE_SYNONYM_DECLARATION +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    // Store synonym
    declaredSynonyms[synonymToken.value] = designEntityType;
  }

  // query-body: 'Select' synonym [ suchthat-cl ] [ pattern-cl ]
  // Parses the PQL query body
  void PqlParser::parseBody(Query& queryUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(SELECT);

    parseSelectTarget(queryUnderConstruction);

    parseClauses(queryUnderConstruction);

  }

  // Parses and set the PQL query target
  void PqlParser::parseSelectTarget(Query& queryUnderConstruction) {
    const Token& synonymToken = validateAndGetAndConsumeWhitespaces(IDENTIFIER);

    // Check if synonym has been declared
    if (!isSynonymDeclared(synonymToken.value)) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    queryUnderConstruction.setTarget(Entity(declaredSynonyms[synonymToken.value], synonymToken.value));
  }

  // Parses the PQL query body clauses
  void PqlParser::parseClauses(Query& queryUnderConstruction) {
    if (!tokens.empty() && tokens.front() == SUCH) {
      parseSuchThatClause(queryUnderConstruction);
    }

    if (!tokens.empty() && tokens.front() == PATTERN) {
      parsePatternClause(queryUnderConstruction);
    }
  }

  // suchthat-cl: 'such that' relRef
  // Parses a such that clause
  void PqlParser::parseSuchThatClause(Query& queryUnderConstruction) {
    validateAndGet(SUCH);
    validateAndGet(SPACE);
    validateAndGetAndConsumeWhitespaces(THAT);

    const Token& relationToken = validateAndGet(IDENTIFIER);
    // Do not consume space yet
    // Handle Follow* and Parent* - Ensure no space between relation and *
    bool isTransitiveRelation = false;
    bool isNextTokenStar = !tokens.empty() && tokens.front() == STAR;
    if (isNextTokenStar) {
      if (!canBeTransitive(relationToken)) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
          ErrorMessage::APPEND_TOKEN_EXPECTED +
          LEFT_PARENTHESIS.value +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          STAR.value
        );
      }
      validateAndGet(STAR);
      isTransitiveRelation = true;
    }
    consumeFrontWhitespaceTokens();

    // Construct the clause
    Clause clauseUnderConstruction;
    if (relationToken == FOLLOWS) {
      clauseUnderConstruction.setType(isTransitiveRelation ? ClauseType::FOLLOWS_T : ClauseType::FOLLOWS);
      parseStmtAndStmtArgs(clauseUnderConstruction);
    } else if (relationToken == PARENT) {
      clauseUnderConstruction.setType(isTransitiveRelation ? ClauseType::PARENT_T : ClauseType::PARENT);
      parseStmtAndStmtArgs(clauseUnderConstruction);
    } else if (relationToken == USES) {
      clauseUnderConstruction.setType(ClauseType::USES_S);
      parseStmtAndEntArgs(clauseUnderConstruction);
    } else if (relationToken == MODIFIES) {
      clauseUnderConstruction.setType(ClauseType::MODIFIES_S);
      parseStmtAndEntArgs(clauseUnderConstruction);
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_INVALID_RELATION +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        relationToken.value
      );
    }

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // pattern-cl: 'pattern' syn-assign '('entRef ',' expression-spec ')'
  // Parses a pattern clause
  void PqlParser::parsePatternClause(Query& queryUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(PATTERN);

    Token& assignSynonymToken = validateAndGetAndConsumeWhitespaces(IDENTIFIER);

    if (!isSynonymDeclared(assignSynonymToken.value)) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        assignSynonymToken.value
      );
    }

    bool isAssignSynonym = declaredSynonyms[assignSynonymToken.value] != EntityType::ASSIGN;
    if (isAssignSynonym) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_NON_ASSIGN_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        assignSynonymToken.value
      );
    }


    Clause clauseUnderConstruction;
    clauseUnderConstruction.setType(ClauseType::PATTERN_ASSIGN);
    clauseUnderConstruction.addParam(Entity(EntityType::ASSIGN, assignSynonymToken.value));

    validateAndGetAndConsumeWhitespaces(LEFT_PARENTHESIS);

    parseEntRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(COMMA);

    parseExprSpec(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(RIGHT_PARENTHESIS);

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // '(' stmtRef ',' stmtRef ')'
  void PqlParser::parseStmtAndStmtArgs(Clause& clauseUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(LEFT_PARENTHESIS);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(COMMA);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(RIGHT_PARENTHESIS);
  }

  // '(' stmtRef ',' entRef ')'
  void PqlParser::parseStmtAndEntArgs(Clause& clauseUnderConstruction) {
    validateAndGetAndConsumeWhitespaces(LEFT_PARENTHESIS);

    parseStmtRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(COMMA);

    parseEntRef(clauseUnderConstruction);

    validateAndGetAndConsumeWhitespaces(RIGHT_PARENTHESIS);
  }

  // stmtRef: synonym | '_' | INTEGER
  void PqlParser::parseStmtRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR '_' OR INTEGER
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGetAndConsumeWhitespaces(IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const EntityType entityType = declaredSynonyms[frontToken.value];
      if (!isStmtRef(entityType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_STMT_REF +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Entity(entityType, frontToken.value));
    } else if (frontToken == UNDERSCORE) {
      validateAndGetAndConsumeWhitespaces(UNDERSCORE);

      clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, frontToken.value));
    } else if (frontToken.type == TokenType::NUMBER) {
      validateAndGetAndConsumeWhitespaces(NUMBER);

      std::string removedLeadingZerosNumber = std::to_string(std::stoi(frontToken.value));
      bool isStmtNumberZero = removedLeadingZerosNumber == "0";
      if (isStmtNumberZero) {
        throw SemanticError(ErrorMessage::SEMANTIC_ERROR_ZERO_STMT_NUMBER);
      }

      clauseUnderConstruction.addParam(Entity(EntityType::STMT_NUMBER, removedLeadingZerosNumber));
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // entRef: synonym | '_' | '"' IDENT '"'
  void PqlParser::parseEntRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR '_' OR '"' IDENT '"'
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGetAndConsumeWhitespaces(IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const EntityType entityType = declaredSynonyms[frontToken.value];
      if (!isEntRef(entityType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_ENT_REF +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Entity(entityType, frontToken.value));
    } else if (frontToken == UNDERSCORE) {
      validateAndGetAndConsumeWhitespaces(UNDERSCORE);

      clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, frontToken.value));
    } else if (frontToken == QUOTE) {
      validateAndGetAndConsumeWhitespaces(QUOTE);

      const Token& variableNameToken = validateAndGetAndConsumeWhitespaces(IDENTIFIER);

      validateAndGetAndConsumeWhitespaces(QUOTE);

      clauseUnderConstruction.addParam(Entity(EntityType::VARIABLE_NAME, variableNameToken.value));
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // expression-spec: '_' '"' factor '"' '_' | '_'
  void PqlParser::parseExprSpec(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }
    const Token frontToken = tokens.front();

    // Check for '_' OR '"'
    if (frontToken == QUOTE) {
      parseExpression(clauseUnderConstruction, true);
    } else {
      validateAndGetAndConsumeWhitespaces(UNDERSCORE);

      if (!tokens.empty() && tokens.front() == QUOTE) {
        parseExpression(clauseUnderConstruction, false);
        validateAndGet(UNDERSCORE);
      } else {
        clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, "_"));
      }
      consumeFrontWhitespaceTokens();
    }
  }

  // '"' factor '"'
  void PqlParser::parseExpression(Clause& clauseUnderConstruction, bool isExactMatch) {
    validateAndGetAndConsumeWhitespaces(QUOTE);

    // Create a list of infix expression tokens
    std::list<Token> infixExpressionTokens;
    while (!tokens.empty() && tokens.front() != QUOTE) {
      const Token currentToken = tokens.front();
      if (currentToken.type == NUMBER.type) { // Remove leading zero for const by casting to int and back to string
        const Token noLeadingZeroConstToken = { currentToken.type, std::to_string(std::stoi(currentToken.value)) };
        infixExpressionTokens.emplace_back(noLeadingZeroConstToken);
      } else {
        infixExpressionTokens.emplace_back(tokens.front());
      }
      tokens.pop_front();
      consumeFrontWhitespaceTokens();
    }
    validateAndGetAndConsumeWhitespaces(QUOTE);

    try {
      clauseUnderConstruction.addParam(
        Entity(
          isExactMatch ? EntityType::EXPRESSION : EntityType::SUB_EXPRESSION,
          infixToPostfixExpression(infixExpressionTokens)
        )
      );
    } catch (const ExprProcessor::SyntaxError& e) {
      throw SyntaxError(e.what());
    }
  }

  Token PqlParser::validateAndGetAndConsumeWhitespaces(const Token& validationToken) {
    const Token& token = validateAndGet(validationToken);
    consumeFrontWhitespaceTokens();
    return token;
  }

  Token PqlParser::validateAndGet(const Token& validationToken) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    bool isCheckTokenType = validationToken.value.empty();

    if (isCheckTokenType) {
      // Check token type
      if (frontToken.type != validationToken.type) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    } else {
      // Check exact match otherwise
      if (frontToken != validationToken) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_VALUE +
          ErrorMessage::APPEND_TOKEN_EXPECTED +
          validationToken.value +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
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
      bool isFrontTokenWhitespace = tokens.front().type == WHITESPACE.type;

      if (isFrontTokenWhitespace) {
        tokens.pop_front();
      } else {
        break;
      }
    }
  }

  bool PqlParser::isSynonymDeclared(const std::string& synonymName) const {
    return bool(declaredSynonyms.count(synonymName));
  }
}