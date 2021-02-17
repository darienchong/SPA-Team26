#include "PqlParser.h"

#include <list>
#include <string>

#include "PqlQuery.h"
#include "Token.h"
#include "ExprParser.h"

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
    return bool(PqlParserUtils::tokenToDesignEntityTypeMapper.count(token));
  }

  /**
   * Checks whether the given token is a relation that can have a transitive version. i.e. Follows and Parent.
   * 
   * @param relationToken Token to verify.
   * @return True if the token is a relation that can have a transitive version. Otherwise, false.
   */
  bool canBeTransitive(const Token& relationToken) {
    return relationToken == PqlTokens::FOLLOWS || relationToken == PqlTokens::PARENT;
  }

  /**
   * Checks whether the given entity type is a synonym that refers to a statement.
   * 
   * @param entityType Entity type to verify.
   * @return True if the entity type refers to a statement. Otherwise, false.
   */
  bool isStmtRef(const Pql::EntityType& entityType) {
    return bool(PqlParserUtils::synonymStmtEntityTypes.count(entityType));
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

  // TODO - To change after ExprParser changes
  std::string infixToPostfixExpression(std::list<Token>& infixExpressionTokens) {
    ExprProcessor::AssignExprParser exprParser(infixExpressionTokens);
    exprParser.parse();
    std::list<Token> postfixTokens = exprParser.getPostfixExprTokens();
    std::string postfixString;
    for (const Token& token : postfixTokens) {
      postfixString.append(token.value);
    }
    return postfixString;
  }
}

// Constructor
PqlParser::PqlParser(const std::list<Token>& tokens)
  : tokens(tokens) {}


Pql::Query PqlParser::parseQuery() {
  parseDeclarations();
  Pql::Query pqlQuery; // Mutating pqlOuery object directly to avoid unnecessary copying.
  parseBody(pqlQuery);
  return pqlQuery;
}

// Iteration 1
// declaration: design-entity synonym (‘,’ synonym)* ‘;’
// design-entity: ‘stmt’ | ‘read’ | ‘print’ | ‘while’ | ‘if’ | ‘assign’ | 
// ‘variable’ | ‘constant’ | ‘procedure’
// Parses the declarations of the PQL query
void PqlParser::parseDeclarations() {
  while (!tokens.empty() && tokens.front() != PqlTokens::SELECT) {
    parseDeclaration();
  }
}


// Parses a single declaration statment of the PQL query
void PqlParser::parseDeclaration() {
  const Token& designEntityToken = validate(PqlTokens::IDENTIFIER);

  // Check if the design entity is valid
  if (!isValidDesignEntity(designEntityToken)) {
    throw PqlParserUtils::SyntaxError("Encounted an invalid design entity for declaration " + designEntityToken.value);
  }

  const Pql::EntityType& designEntityType = PqlParserUtils::tokenToDesignEntityTypeMapper.at(designEntityToken);

  parseDeclarationSynonym(designEntityType);
  while (!tokens.empty() && tokens.front() != PqlTokens::SEMICOLON) {
    validate(PqlTokens::COMMA);
    parseDeclarationSynonym(designEntityType);
  }
  validate(PqlTokens::SEMICOLON);
}

// Parses the synonyms in the declarations of the PQL query and store the synonym as the specified design entity type
void PqlParser::parseDeclarationSynonym(const Pql::EntityType& designEntityType) {
  const Token& synonymToken = validate(PqlTokens::IDENTIFIER);

  // Check if synonym has been declared
  if (isSynonymDeclared(synonymToken.value)) {
    throw PqlParserUtils::SemanticError("Encounted an already declared synonym " + synonymToken.value);
  }

  // Store synonym
  declaredSynonyms[synonymToken.value] = designEntityType;
}

// query-body: ‘Select’ synonym [ suchthat-cl ] [ pattern-cl ]
// Parses the PQL query body
void PqlParser::parseBody(Pql::Query& queryUnderConstruction) {
  validate(PqlTokens::SELECT);
  parseSelectTarget(queryUnderConstruction);
  parseClauses(queryUnderConstruction);

  // Check for unexpected tokens after the clauses
  if (!tokens.empty()) {
    throw PqlParserUtils::SyntaxError("Encounter an unexpected token after the query body " + tokens.front().value);
  }
}

// Parses and set the PQL query target
void PqlParser::parseSelectTarget(Pql::Query& queryUnderConstruction) {
  const Token& synonymToken = validate(PqlTokens::IDENTIFIER);

  // Check if synonym has been declared
  if (!isSynonymDeclared(synonymToken.value)) {
    throw PqlParserUtils::SemanticError("Encounted an undeclared synonym " + synonymToken.value);
  }

  queryUnderConstruction.setTarget(Pql::Entity(declaredSynonyms[synonymToken.value], synonymToken.value));
}

// Parses the PQL query body clauses
void PqlParser::parseClauses(Pql::Query& queryUnderConstruction) {
  if (!tokens.empty() && tokens.front() == PqlTokens::SUCH) {
    parseSuchThatClause(queryUnderConstruction);
  }

  if (!tokens.empty() && tokens.front() == PqlTokens::PATTERN) {
    parsePatternClause(queryUnderConstruction);
  }
}

// suchthat-cl: ‘such that’ relRef
// Parses a such that clause
void PqlParser::parseSuchThatClause(Pql::Query& queryUnderConstruction) {
  validate(PqlTokens::SUCH);
  validate(PqlTokens::THAT);

  const Token& relationToken = validate(PqlTokens::IDENTIFIER);
  bool isTransitiveRelation = false;
  if (!tokens.empty() && tokens.front() == PqlTokens::STAR) {
    if (!canBeTransitive(relationToken)) {
      throw PqlParserUtils::SyntaxError("Encounted an unexpected token *");
    }
    validate(PqlTokens::STAR);
    isTransitiveRelation = true;
  }

  Pql::Clause clauseUnderConstruction;
  if (relationToken == PqlTokens::FOLLOWS) {
    clauseUnderConstruction.setType(isTransitiveRelation ? Pql::ClauseType::FOLLOWS_T : Pql::ClauseType::FOLLOWS);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }
  else if (relationToken == PqlTokens::PARENT) {
    clauseUnderConstruction.setType(isTransitiveRelation ? Pql::ClauseType::PARENT_T : Pql::ClauseType::PARENT);
    parseStmtAndStmtArgs(clauseUnderConstruction);
  }
  else if (relationToken == PqlTokens::USES) {
    clauseUnderConstruction.setType(Pql::ClauseType::USES_S);
    parseStmtAndEntArgs(clauseUnderConstruction);
  }
  else if (relationToken == PqlTokens::MODIFIES) {
    clauseUnderConstruction.setType(Pql::ClauseType::MODIFIES_S);
    parseStmtAndEntArgs(clauseUnderConstruction);
  }
  else {
    throw PqlParserUtils::SyntaxError("Encounted an unknow relation " + relationToken.value);
  }
  queryUnderConstruction.addClause(clauseUnderConstruction);
}

// pattern-cl: ‘pattern’ syn-assign ‘(‘entRef ’,’ expression-spec ’)’
// Parses a pattern clause
void PqlParser::parsePatternClause(Pql::Query& queryUnderConstruction) {
  validate(PqlTokens::PATTERN);

  Token& assignSynonymToken = validate(PqlTokens::IDENTIFIER);

  if (!isSynonymDeclared(assignSynonymToken.value)) {
    throw PqlParserUtils::SemanticError("Encounted an undeclared synonym " + assignSynonymToken.value);
  }

  if (declaredSynonyms[assignSynonymToken.value] != Pql::EntityType::ASSIGN) {
    throw PqlParserUtils::SemanticError("Expected a synonym of type assign but encounted another type");
  }

  Pql::Clause clauseUnderConstruction;
  clauseUnderConstruction.setType(Pql::ClauseType::PATTERN_ASSIGN);
  clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::ASSIGN, assignSynonymToken.value));
  validate(PqlTokens::LEFT_PARENTHESIS);
  parseEntRef(clauseUnderConstruction);
  validate(PqlTokens::COMMA);
  parseExprSpec(clauseUnderConstruction);
  validate(PqlTokens::RIGHT_PARENTHESIS);
  queryUnderConstruction.addClause(clauseUnderConstruction);
}

// ‘(’ stmtRef ‘,’ stmtRef ‘)’
void PqlParser::parseStmtAndStmtArgs(Pql::Clause& clauseUnderConstruction) {
  validate(PqlTokens::LEFT_PARENTHESIS);
  parseStmtRef(clauseUnderConstruction);
  validate(PqlTokens::COMMA);
  parseStmtRef(clauseUnderConstruction);
  validate(PqlTokens::RIGHT_PARENTHESIS);
}

// ‘(’ stmtRef ‘,’ entRef ‘)’
void PqlParser::parseStmtAndEntArgs(Pql::Clause& clauseUnderConstruction) {
  validate(PqlTokens::LEFT_PARENTHESIS);
  parseStmtRef(clauseUnderConstruction);
  validate(PqlTokens::COMMA);
  parseEntRef(clauseUnderConstruction);
  validate(PqlTokens::RIGHT_PARENTHESIS);
}

// stmtRef: synonym | ‘_’ | INTEGER
void PqlParser::parseStmtRef(Pql::Clause& clauseUnderConstruction) {
  if (tokens.empty()) {
    throw PqlParserUtils::SyntaxError("Expected a stmtRef but ran out of tokens");
  }
  const Token frontToken = tokens.front();
  if (frontToken.type == TokenType::IDENTIFIER) {
    validate(PqlTokens::IDENTIFIER);
    if (!isSynonymDeclared(frontToken.value)) {
      throw PqlParserUtils::SemanticError("Encounted an undeclared synonym " + frontToken.value);
    }
    if (!isStmtRef(declaredSynonyms[frontToken.value])) {
      throw PqlParserUtils::SemanticError("Expected a stmtRef but encounted other design entity " + frontToken.value);
    }
    clauseUnderConstruction.addParam(Pql::Entity(declaredSynonyms[frontToken.value], frontToken.value));
  }
  else if (frontToken == PqlTokens::UNDERSCORE) {
    validate(PqlTokens::UNDERSCORE);
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, frontToken.value));
  }
  else if (frontToken.type == TokenType::NUMBER) {
    validate(PqlTokens::NUMBER);
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::LINE_NUMBER, frontToken.value));
  }
  else {
    throw PqlParserUtils::SyntaxError("Encountered unexpected token " + frontToken.value);
  }
}

// entRef: synonym | ‘_’ | ‘"’ IDENT ‘"’
void PqlParser::parseEntRef(Pql::Clause& clauseUnderConstruction) {
  if (tokens.empty()) {
    throw PqlParserUtils::SyntaxError("Expected an entRef but ran out of tokens");
  }
  const Token frontToken = tokens.front();
  if (frontToken.type == TokenType::IDENTIFIER) {
    validate(PqlTokens::IDENTIFIER);
    if (!isSynonymDeclared(frontToken.value)) {
      throw PqlParserUtils::SemanticError("Encounted an undeclared synonym " + frontToken.value);
    }
    if (!isEntRef(declaredSynonyms[frontToken.value])) {
      throw PqlParserUtils::SemanticError("Expected an entRef but encounted other design entity " + frontToken.value);
    }
    clauseUnderConstruction.addParam(Pql::Entity(declaredSynonyms[frontToken.value], frontToken.value));
  }
  else if (frontToken == PqlTokens::UNDERSCORE) {
    validate(PqlTokens::UNDERSCORE);
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, frontToken.value));
  }
  else if (frontToken == PqlTokens::QUOTE) {
    validate(PqlTokens::QUOTE);
    const Token& variableNameToken = validate(PqlTokens::IDENTIFIER);
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::VARIABLE_NAME, variableNameToken.value));
    validate(PqlTokens::QUOTE);
  }
  else {
    throw PqlParserUtils::SyntaxError("Encountered unexpected token " + frontToken.value);
  }
}

// expression-spec: ‘_’ ‘"’ factor ‘"’ ‘_’ | ‘_’
void PqlParser::parseExprSpec(Pql::Clause& clauseUnderConstruction) {
  validate(PqlTokens::UNDERSCORE);
  if (!tokens.empty() && tokens.front() == PqlTokens::QUOTE) {
    parseExpression(clauseUnderConstruction);
    validate(PqlTokens::UNDERSCORE);
  }
  else {
    clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::WILDCARD, "_"));
  }
}

// ‘"’ factor ‘"’
void PqlParser::parseExpression(Pql::Clause& clauseUnderConstruction) {
  validate(PqlTokens::QUOTE);
  std::list<Token> infixExpressionTokens;
  while (!tokens.empty() && tokens.front() != PqlTokens::QUOTE) {
    infixExpressionTokens.emplace_back(tokens.front());
    tokens.pop_front();
  }
  validate(PqlTokens::QUOTE);
  clauseUnderConstruction.addParam(Pql::Entity(Pql::EntityType::SUB_EXPRESSION,
    infixToPostfixExpression(infixExpressionTokens)));
}

Token PqlParser::validate(const Token& token) {
  if (tokens.empty()) {
    throw PqlParserUtils::SyntaxError("Incomplete PQL query");
  }

  const Token frontToken = tokens.front();

  if (token.value.empty()) {
    // Check type only for empty token value
    if (frontToken.type != token.type) {
      throw PqlParserUtils::SyntaxError("Encounted token of a wrong type");
    }
  }
  else {
    // Check exact match otherwise
    if (frontToken != token) {
      throw PqlParserUtils::SyntaxError("Expected " + token.value + " but encountered " + frontToken.value);
    }
  }

  // Validation success
  tokens.pop_front();
  return frontToken;
}

bool PqlParser::isSynonymDeclared(const std::string& synonymName) const {
  return bool(declaredSynonyms.count(synonymName));
}
