#include "PqlParser.h"

#include <assert.h>

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
   * design-entity : 'stmt' | 'read' | 'print' | 'call' | 'while' | 'if' | 'assign' | 'variable' |
   *                 'constant' | 'procedure' | 'proc_line'
   *
   * @param token Token to verify.
   * @return True if the token is valid. Otherwise, false.
   */
  bool isValidDesignEntity(const Token& token) {
    return Pql::tokenToDesignEntityTypeMapper.count(token) == 1;
  }

  /**
   * Checks whether the given token is a relation that can have a transitive version. e.g. Follows and Parent.
   *
   * @param relationToken Token to verify.
   * @return True if the token is a relation that can have a transitive version. Otherwise, false.
   */
  bool canBeTransitive(const Token& relationToken) {
    return relationToken == Pql::FOLLOWS ||
      relationToken == Pql::PARENT ||
      relationToken == Pql::CALLS ||
      relationToken == Pql::NEXT ||
      relationToken == Pql::AFFECTS;
  }

  /**
   * Checks whether the given token is an attribute name token. e.g. procName and varName.
   *
   * @param token Token to verify.
   * @return True if the token is a relation that can have a transitive version. Otherwise, false.
   */
  bool isAttributeName(const Token& token) {
    return Pql::tokenToAttributeRefTypeMapper.count(token) == 1;
  }

  /**
   * Checks whether the given synonym type and attribute reference type pair is valid.
   * attributeRefType cannot be of type NONE.
   *
   * @param synonymType Synonym type to verify. Cannot be of type NONE.
   * @param attributeRefType Attribute reference type to verify.
   * @return True if the pair is valid. Otherwise, false.
   */
  bool isSemanticallyValidAttributeName(Pql::EntityType synonymType, Pql::AttributeRefType attributeRefType) {
    switch (attributeRefType) {
    case Pql::AttributeRefType::PROC_NAME:
      return
        synonymType == Pql::EntityType::PROCEDURE ||
        synonymType == Pql::EntityType::CALL;
    case Pql::AttributeRefType::VAR_NAME:
      return
        synonymType == Pql::EntityType::VARIABLE ||
        synonymType == Pql::EntityType::READ ||
        synonymType == Pql::EntityType::PRINT;
    case Pql::AttributeRefType::VALUE:
      return
        synonymType == Pql::EntityType::CONSTANT;
    case Pql::AttributeRefType::STMT_NUMBER:
      return
        synonymType == Pql::EntityType::STMT ||
        synonymType == Pql::EntityType::READ ||
        synonymType == Pql::EntityType::PRINT ||
        synonymType == Pql::EntityType::CALL ||
        synonymType == Pql::EntityType::WHILE ||
        synonymType == Pql::EntityType::IF ||
        synonymType == Pql::EntityType::ASSIGN;
    default:
      assert(false);
      return true;
    }
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
  bool isVarRef(const Pql::EntityType& entityType) {
    return entityType == Pql::EntityType::VARIABLE;
  }

  /**
   * Checks whether the given entity type is a synonym that refers to a procedure.
   *
   * @param entityType Entity type to verify.
   * @return True if the entity type refers to a procedure. Otherwise, false.
   */
  bool isProcRef(const Pql::EntityType& entityType) {
    return entityType == Pql::EntityType::PROCEDURE;
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
  PqlParser::PqlParser(std::list<Token>& tokens)
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

  // declaration: design-entity synonym (',' synonym)* ';'
  // design-entity: 'stmt' | 'read' | 'print' | 'while' | 'if' | 'assign' | 
  // 'variable' | 'constant' | 'procedure' | 'call' | 'prog_line'
  // Parses the declarations of the PQL query
  void PqlParser::parseDeclarations() {
    while (!tokens.empty() && tokens.front() != SELECT) {
      parseDeclaration();
    }
  }

  // declaration: design-entity synonym (',' synonym)* ';'
  void PqlParser::parseDeclaration() {
    // Parses design-entity - Do not consume whitespaces yet (For prog_line)
    const Token& designEntityToken = validateAndGet(IDENTIFIER, false);

    if (!isValidDesignEntity(designEntityToken)) {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_INVALID_DESIGN_ENTITY +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        designEntityToken.value
      );
    }

    // Check for prog_line
    if (designEntityToken == PROG) {
      validateAndGet(UNDERSCORE, false);
      validateAndGet(LINE, false);
    }

    consumeFrontWhitespaceTokens(); // Consume whitespaces at the end

    const EntityType& designEntityType = tokenToDesignEntityTypeMapper.at(designEntityToken);

    // Parses first synonym
    parseDeclarationSynonym(designEntityType);

    // Parses additional synonyms
    while (!tokens.empty() && tokens.front() != SEMICOLON) {
      validateAndGet(COMMA);

      parseDeclarationSynonym(designEntityType);
    }

    validateAndGet(SEMICOLON);
  }

  // Parses the synonyms in the declarations of the PQL query and store the synonym as the specified design entity type
  void PqlParser::parseDeclarationSynonym(const EntityType& designEntityType) {
    const Token& synonymToken = validateAndGet(IDENTIFIER);

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
    validateAndGet(SELECT);

    parseSelectTargets(queryUnderConstruction);

    parseClauses(queryUnderConstruction);

  }

  // Parses and set the PQL query targets
  void PqlParser::parseSelectTargets(Query& queryUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    if (frontToken == BOOLEAN) { // BOOLEAN query
      validateAndGet(BOOLEAN);
    } else if (frontToken == LEFT_ANGLE_BRACKET) { // Tuple select
      validateAndGet(LEFT_ANGLE_BRACKET);

      // Parse first select target in tuple
      parseSelectTarget(queryUnderConstruction);

      // Check for additional select targets in tuple
      while (!tokens.empty() && tokens.front() == COMMA) {
        validateAndGet(COMMA);
        parseSelectTarget(queryUnderConstruction);
      }

      validateAndGet(RIGHT_ANGLE_BRACKET);
    } else { // Single select
      parseSelectTarget(queryUnderConstruction);
    }
  }

  // Parses and add a single PQL query target
  void PqlParser::parseSelectTarget(Query& queryUnderConstruction) {
    const Token& synonymToken = validateAndGet(IDENTIFIER);

    // Check if synonym has been declared
    if (!isSynonymDeclared(synonymToken.value)) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    const EntityType synonymType = declaredSynonyms[synonymToken.value];
    AttributeRefType attributeRefType = AttributeRefType::NONE;

    // Check if select target is an attrRef
    if (!tokens.empty() && tokens.front() == DOT) {
      validateAndGet(DOT);
      const Token& attributeNameToken = validateAndGet(IDENTIFIER, false); // Don't consume whitespaces yet due to 'stmt#'
      if (!isAttributeName(attributeNameToken)) {
        throw SyntaxError(
          ErrorMessage::SYNTAX_ERROR_INVALID_ATTRIBUTE_NAME +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          attributeNameToken.value
        );
      }

      if (attributeNameToken == STMT) {
        validateAndGet(NUMBER_SIGN, false);
      }
      consumeFrontWhitespaceTokens(); // Consume whitespaces at the end

      attributeRefType = tokenToAttributeRefTypeMapper.at(attributeNameToken); // Update attributeRefType

      // Checks if synonym has the attribute name
      if (!isSemanticallyValidAttributeName(synonymType, attributeRefType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_INVALID_ATTRIBUTE_NAME +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          attributeNameToken.value
        );
      }

    }

    queryUnderConstruction.addTarget(Entity(synonymType, synonymToken.value, attributeRefType));
  }

  // Parses the PQL query body clauses
  void PqlParser::parseClauses(Query& queryUnderConstruction) {
    ParsingClauseType currentClauseType = ParsingClauseType::UNDEFINED;
    while (!tokens.empty()) {
      const Token frontToken = tokens.front();
      if (frontToken == SUCH) { // such that clause
        // Consume 'such that'
        validateAndGet(SUCH, false);
        validateAndGet(SPACE, false);
        validateAndGet(THAT);

        parseSuchThatClause(queryUnderConstruction);
        currentClauseType = ParsingClauseType::SUCH_THAT;
      } else if (frontToken == PATTERN) { // pattern clause
        // Consume 'pattern'
        validateAndGet(PATTERN);

        parsePatternClause(queryUnderConstruction);
        currentClauseType = ParsingClauseType::PATTERN;
      } else if (frontToken == WITH) { // with clause
        // Consume 'with'
        validateAndGet(WITH);

        parseWithClause(queryUnderConstruction);
        currentClauseType = ParsingClauseType::WITH;
      } else if (frontToken == AND) {
        validateAndGet(AND);

        switch (currentClauseType) {
        case ParsingClauseType::SUCH_THAT:
          parseSuchThatClause(queryUnderConstruction);
          break;
        case ParsingClauseType::PATTERN:
          parsePatternClause(queryUnderConstruction);
          break;
        case ParsingClauseType::WITH:
          parseWithClause(queryUnderConstruction);
          break;
        default:
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_VALUE +
            ErrorMessage::APPEND_TOKEN_EXPECTED + SUCH.value + "/" + PATTERN.value + "/" + WITH.value +
            ErrorMessage::APPEND_TOKEN_RECEIVED + frontToken.value
          );
          break;
        }
      } else {
        break; // Additional tokens will be caught by parseQuery()
      }
    }
  }

  // Parses a such that clause
  void PqlParser::parseSuchThatClause(Query& queryUnderConstruction) {
    // 'such that' OR 'and' already consumed

    const Token& relationToken = validateAndGet(IDENTIFIER, false);
    // Do not consume space yet
    // Ensure no space between relation and *
    bool isTransitiveRelation = false;
    const bool isNextTokenStar = !tokens.empty() && tokens.front() == STAR;
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
      validateAndGet(STAR, false);
      isTransitiveRelation = true;
    }
    consumeFrontWhitespaceTokens();

    // Construct the clause
    Clause clauseUnderConstruction;
    if (relationToken == FOLLOWS) {
      isTransitiveRelation
        ? parseFollowsTClause(clauseUnderConstruction)
        : parseFollowsClause(clauseUnderConstruction);
    } else if (relationToken == PARENT) {
      isTransitiveRelation
        ? parseParentTClause(clauseUnderConstruction)
        : parseParentClause(clauseUnderConstruction);
    } else if (relationToken == CALLS) {
      isTransitiveRelation
        ? parseCallsTClause(clauseUnderConstruction)
        : parseCallsClause(clauseUnderConstruction);
    } else if (relationToken == NEXT) {
      isTransitiveRelation
        ? parseNextTClause(clauseUnderConstruction)
        : parseNextClause(clauseUnderConstruction);
    } else if (relationToken == AFFECTS) {
      isTransitiveRelation
        ? parseAffectsTClause(clauseUnderConstruction)
        : parseAffectsClause(clauseUnderConstruction);
    } else if (relationToken == USES) {
      parseUsesClause(clauseUnderConstruction);
    } else if (relationToken == MODIFIES) {
      parseModifiesClause(clauseUnderConstruction);
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_INVALID_RELATION +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        relationToken.value
      );
    }

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // Parses a pattern clause
  void PqlParser::parsePatternClause(Query& queryUnderConstruction) {
    // 'pattern' OR 'and' already consumed

    const Token& synonymToken = validateAndGet(IDENTIFIER);

    if (!isSynonymDeclared(synonymToken.value)) {
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
    }

    Clause clauseUnderConstruction;
    EntityType synonymType = declaredSynonyms[synonymToken.value];
    switch (synonymType) {
    case Pql::EntityType::WHILE:
      parsePatternWhileClause(clauseUnderConstruction, synonymToken.value);
      break;
    case Pql::EntityType::IF:
      parsePatternIfClause(clauseUnderConstruction, synonymToken.value);
      break;
    case Pql::EntityType::ASSIGN:
      parsePatternAssignClause(clauseUnderConstruction, synonymToken.value);
      break;
    default:
      throw SemanticError(
        ErrorMessage::SEMANTIC_ERROR_NON_ASSIGN_SYNONYM +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        synonymToken.value
      );
      break;
    }

    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  // Parses a with clause
  void PqlParser::parseWithClause(Query& queryUnderConstruction) {
    // 'with' OR 'and' already consumed
    Clause clauseUnderConstruction;
    clauseUnderConstruction.setType(ClauseType::WITH);
    parseRef(clauseUnderConstruction);
    validateAndGet(EQUAL);
    parseRef(clauseUnderConstruction);
    queryUnderConstruction.addClause(clauseUnderConstruction);
  }

  void PqlParser::parseFollowsClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::FOLLOWS);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseFollowsTClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::FOLLOWS_T);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseParentClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::PARENT);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseParentTClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::PARENT_T);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseCallsClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::CALLS);
    parseProcAndProcArgs(clauseUnderConstruction);
  }

  void PqlParser::parseCallsTClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::CALLS_T);
    parseProcAndProcArgs(clauseUnderConstruction);
  }

  void PqlParser::parseNextClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::NEXT);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseNextTClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::NEXT_T);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseAffectsClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::AFFECTS);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  void PqlParser::parseAffectsTClause(Clause& clauseUnderConstruction) {
    clauseUnderConstruction.setType(ClauseType::AFFECTS_T);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }

  // 'Uses' '(' stmtRef ',' entRef ')' OR 'Uses' '(' entRef ',' entRef ')'
  void PqlParser::parseUsesClause(Clause& clauseUnderConstruction) {
    // 'uses' already consumed
    validateAndGet(LEFT_PARENTHESIS);

    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    if (frontToken == UNDERSCORE) {
      throw SemanticError(ErrorMessage::SEMANTIC_ERROR_INVALID_WILDCARD);
    }

    if (frontToken.type == TokenType::NUMBER) {                // constant as stmtRef
      parseStmtRef(clauseUnderConstruction);
      clauseUnderConstruction.setType(ClauseType::USES_S);
    } else if (frontToken == QUOTE) {                          // name as procName
      parseProcRef(clauseUnderConstruction);
      clauseUnderConstruction.setType(ClauseType::USES_P);
    } else if (frontToken.type == TokenType::IDENTIFIER) {     // synonym
      // Check if synonym is declared
      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      EntityType synonymType = declaredSynonyms[frontToken.value];
      if (isStmtRef(synonymType)) {
        parseStmtRef(clauseUnderConstruction);
        clauseUnderConstruction.setType(ClauseType::USES_S);
      } else if (isProcRef(synonymType)) {
        parseProcRef(clauseUnderConstruction);
        clauseUnderConstruction.setType(ClauseType::USES_P);
      } else { // Other synonym types
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_ASSIGN_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }


    validateAndGet(COMMA);

    parseVarRef(clauseUnderConstruction);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // 'Modifies' '(' stmtRef ',' entRef ')' OR 'Modifies' '(' entRef ',' entRef ')'
  void PqlParser::parseModifiesClause(Clause& clauseUnderConstruction) {
    // 'Modifies' already consumed
    validateAndGet(LEFT_PARENTHESIS);

    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    if (frontToken == UNDERSCORE) {
      throw SemanticError(ErrorMessage::SEMANTIC_ERROR_INVALID_WILDCARD);
    }

    if (frontToken.type == TokenType::NUMBER) {                // constant as stmtRef
      parseStmtRef(clauseUnderConstruction);
      clauseUnderConstruction.setType(ClauseType::MODIFIES_S);
    } else if (frontToken == QUOTE) {                          // name as procName
      parseProcRef(clauseUnderConstruction);
      clauseUnderConstruction.setType(ClauseType::MODIFIES_P);
    } else if (frontToken.type == TokenType::IDENTIFIER) {     // synonym
      // Check if synonym is declared
      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      EntityType synonymType = declaredSynonyms[frontToken.value];
      if (isStmtRef(synonymType)) {
        parseStmtRef(clauseUnderConstruction);
        clauseUnderConstruction.setType(ClauseType::MODIFIES_S);
      } else if (isProcRef(synonymType)) {
        parseProcRef(clauseUnderConstruction);
        clauseUnderConstruction.setType(ClauseType::MODIFIES_P);
      } else { // Other synonym types
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_ASSIGN_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }


    validateAndGet(COMMA);

    parseVarRef(clauseUnderConstruction);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // assign : 'pattern' syn-assign '('entRef ',' expression-spec ')'
  void PqlParser::parsePatternAssignClause(Pql::Clause& clauseUnderConstruction, const std::string& synonymName) {
    clauseUnderConstruction.setType(ClauseType::PATTERN_ASSIGN);
    clauseUnderConstruction.addParam(Entity(EntityType::ASSIGN, synonymName));

    validateAndGet(LEFT_PARENTHESIS);

    parseVarRef(clauseUnderConstruction);

    validateAndGet(COMMA);

    parseExprSpec(clauseUnderConstruction);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // if : syn-if '(' entRef ',' '_' ',' '_' ')'
  void PqlParser::parsePatternIfClause(Pql::Clause& clauseUnderConstruction, const std::string& synonymName) {
    clauseUnderConstruction.setType(ClauseType::PATTERN_IF);
    clauseUnderConstruction.addParam(Entity(EntityType::IF, synonymName));

    validateAndGet(LEFT_PARENTHESIS);

    parseVarRef(clauseUnderConstruction);

    validateAndGet(COMMA);

    validateAndGet(UNDERSCORE);

    validateAndGet(COMMA);

    validateAndGet(UNDERSCORE);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // while : syn-while '(' entRef ',' '_' ')'
  void PqlParser::parsePatternWhileClause(Pql::Clause& clauseUnderConstruction, const std::string& synonymName) {
    clauseUnderConstruction.setType(ClauseType::PATTERN_WHILE);
    clauseUnderConstruction.addParam(Entity(EntityType::WHILE, synonymName));

    validateAndGet(LEFT_PARENTHESIS);

    parseVarRef(clauseUnderConstruction);

    validateAndGet(COMMA);

    validateAndGet(UNDERSCORE);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // '(' stmtRef ',' stmtRef ')'
  void PqlParser::parseStmtAndStmtArgs(Clause& clauseUnderConstruction) {
    validateAndGet(LEFT_PARENTHESIS);

    parseStmtRef(clauseUnderConstruction);

    validateAndGet(COMMA);

    parseStmtRef(clauseUnderConstruction);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  // '(' procRef ',' procRef ')'
  void PqlParser::parseProcAndProcArgs(Clause& clauseUnderConstruction) {
    validateAndGet(LEFT_PARENTHESIS);

    parseProcRef(clauseUnderConstruction);

    validateAndGet(COMMA);

    parseProcRef(clauseUnderConstruction);

    validateAndGet(RIGHT_PARENTHESIS);
  }

  void PqlParser::parseRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    if (frontToken == QUOTE) { // '"' IDENT '"'
      validateAndGet(QUOTE);

      const Token& nameToken = validateAndGet(IDENTIFIER);

      validateAndGet(QUOTE);

      clauseUnderConstruction.addParam(Entity(EntityType::NAME, nameToken.value));
    } else if (frontToken.type == TokenType::NUMBER) { // INTEGER
      const Token& numberToken = validateAndGet(NUMBER);
      std::string removedLeadingZerosNumber = std::to_string(std::stoi(numberToken.value));
      clauseUnderConstruction.addParam(Entity(EntityType::NUMBER, removedLeadingZerosNumber));
    } else if (frontToken.type == TokenType::IDENTIFIER) { // prog_line synoynm OR attrRef
      const Token& synonymToken = validateAndGet(IDENTIFIER);

      // Check if synonym has been declared
      if (!isSynonymDeclared(synonymToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          synonymToken.value
        );
      }

      const EntityType synonymType = declaredSynonyms[synonymToken.value];
      if (synonymType == EntityType::PROG_LINE) { // prog_line synonym
        clauseUnderConstruction.addParam(Entity(synonymType, synonymToken.value));
      } else { // attrRef
        validateAndGet(DOT);
        const Token& attributeNameToken = validateAndGet(IDENTIFIER, false); // Don't consume whitespaces yet due to 'stmt#'
        if (!isAttributeName(attributeNameToken)) {
          throw SyntaxError(
            ErrorMessage::SYNTAX_ERROR_INVALID_ATTRIBUTE_NAME +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            attributeNameToken.value
          );
        }

        if (attributeNameToken == STMT) {
          validateAndGet(NUMBER_SIGN, false);
        }
        consumeFrontWhitespaceTokens(); // Consume whitespaces at the end

        AttributeRefType attributeRefType = tokenToAttributeRefTypeMapper.at(attributeNameToken); // Update attributeRefType
        // Checks if synonym has the attribute name
        if (!isSemanticallyValidAttributeName(synonymType, attributeRefType)) {
          throw SemanticError(
            ErrorMessage::SEMANTIC_ERROR_INVALID_ATTRIBUTE_NAME +
            ErrorMessage::APPEND_TOKEN_RECEIVED +
            attributeNameToken.value
          );
        }

        clauseUnderConstruction.addParam(Entity(synonymType, synonymToken.value, attributeRefType));
      }
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // stmtRef: synonym | '_' | INTEGER
  void PqlParser::parseStmtRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR '_' OR INTEGER
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGet(IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const EntityType& entityType = declaredSynonyms[frontToken.value];
      if (!isStmtRef(entityType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_STMT_REF +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Entity(entityType, frontToken.value));
    } else if (frontToken == UNDERSCORE) {
      validateAndGet(UNDERSCORE);

      clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, frontToken.value));
    } else if (frontToken.type == TokenType::NUMBER) {
      validateAndGet(NUMBER);

      std::string removedLeadingZerosNumber = std::to_string(std::stoi(frontToken.value));
      const bool isStmtNumberZero = removedLeadingZerosNumber == "0";
      if (isStmtNumberZero) {
        throw SemanticError(ErrorMessage::SEMANTIC_ERROR_ZERO_STMT_NUMBER);
      }

      clauseUnderConstruction.addParam(Entity(EntityType::NUMBER, removedLeadingZerosNumber));
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // var-synonym | '_' | '"' IDENT '"'
  void PqlParser::parseVarRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR '_' OR '"' IDENT '"'
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGet(IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const EntityType& entityType = declaredSynonyms[frontToken.value];
      if (!isVarRef(entityType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_ENT_REF +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Entity(entityType, frontToken.value));
    } else if (frontToken == UNDERSCORE) {
      validateAndGet(UNDERSCORE);

      clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, frontToken.value));
    } else if (frontToken == QUOTE) {
      validateAndGet(QUOTE);

      const Token& variableNameToken = validateAndGet(IDENTIFIER);

      validateAndGet(QUOTE);

      clauseUnderConstruction.addParam(Entity(EntityType::NAME, variableNameToken.value));
    } else {
      throw SyntaxError(
        ErrorMessage::SYNTAX_ERROR_WRONG_TOKEN_TYPE +
        ErrorMessage::APPEND_TOKEN_RECEIVED +
        frontToken.value
      );
    }
  }

  // procedure-synonym | '_' | '"' IDENT '"'
  void PqlParser::parseProcRef(Clause& clauseUnderConstruction) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();

    // Check for synonym OR '_' OR '"' IDENT '"'
    if (frontToken.type == TokenType::IDENTIFIER) {
      validateAndGet(IDENTIFIER);

      if (!isSynonymDeclared(frontToken.value)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_UNDECLARED_SYNONYM +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      const EntityType entityType = declaredSynonyms[frontToken.value];
      if (!isProcRef(entityType)) {
        throw SemanticError(
          ErrorMessage::SEMANTIC_ERROR_NON_ENT_REF +
          ErrorMessage::APPEND_TOKEN_RECEIVED +
          frontToken.value
        );
      }

      clauseUnderConstruction.addParam(Entity(entityType, frontToken.value));
    } else if (frontToken == UNDERSCORE) {
      validateAndGet(UNDERSCORE);

      clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, frontToken.value));
    } else if (frontToken == QUOTE) {
      validateAndGet(QUOTE);

      const Token& procNameToken = validateAndGet(IDENTIFIER);

      validateAndGet(QUOTE);

      clauseUnderConstruction.addParam(Entity(EntityType::NAME, procNameToken.value));
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
      validateAndGet(UNDERSCORE);

      if (!tokens.empty() && tokens.front() == QUOTE) {
        parseExpression(clauseUnderConstruction, false);
        validateAndGet(UNDERSCORE, false);
      } else {
        clauseUnderConstruction.addParam(Entity(EntityType::WILDCARD, "_"));
      }
      consumeFrontWhitespaceTokens();
    }
  }

  void PqlParser::parseExpression(Clause& clauseUnderConstruction, const bool isExactMatch) {
    validateAndGet(QUOTE);

    // Create a list of infix expression tokens
    std::list<Token> infixExpressionTokens;
    while (!tokens.empty() && tokens.front() != QUOTE) {
      const Token currentToken = tokens.front();
      if (currentToken.type == TokenType::NUMBER) {
        // Remove leading zero for const by casting to int and back to string
        const Token& noLeadingZeroConstToken = { currentToken.type, std::to_string(std::stoi(currentToken.value)) };
        infixExpressionTokens.emplace_back(noLeadingZeroConstToken);
      } else {
        infixExpressionTokens.emplace_back(tokens.front());
      }
      tokens.pop_front();
      consumeFrontWhitespaceTokens();
    }
    validateAndGet(QUOTE);

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

  Token PqlParser::validateAndGet(const Token& validationToken) {
    return validateAndGet(validationToken, true);
  }

  Token PqlParser::validateAndGet(const Token& validationToken, const bool shouldConsumeWhitespaces) {
    if (tokens.empty()) {
      throw SyntaxError(ErrorMessage::SYNTAX_ERROR_NOT_ENOUGH_TOKENS);
    }

    const Token frontToken = tokens.front();
    const bool isCheckTokenType = validationToken.value.empty();

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

    if (shouldConsumeWhitespaces) {
      consumeFrontWhitespaceTokens();
    }

    return frontToken;
  }

  void PqlParser::consumeFrontWhitespaceTokens() {
    while (!tokens.empty()) {
      const bool isFrontTokenWhitespace = tokens.front().type == TokenType::WHITESPACE;

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