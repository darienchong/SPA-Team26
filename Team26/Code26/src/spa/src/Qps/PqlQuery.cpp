#include "PqlQuery.h"

#include <string>

namespace Pql {
  // Entity methods
  Entity::Entity()
    : type(EntityType::UNDEFINED), attributeRefType(AttributeRefType::NONE) {
  }

  Entity::Entity(const EntityType& type, const std::string& value)
    : type(type), value(value), attributeRefType(AttributeRefType::NONE) {
  }

  Entity::Entity(const EntityType& type, const std::string& value, const AttributeRefType& attributeRefType)
    : type(type), value(value), attributeRefType(attributeRefType) {
  }

  EntityType Entity::getType() const {
    return type;
  }

  std::string Entity::getValue() const {
    return value;
  }

  AttributeRefType Entity::getAttributeRefType() const {
    return attributeRefType;
  }

  bool Entity::isAttributeRef() const {
    return attributeRefType != AttributeRefType::NONE;
  }

  bool Entity::isNumber() const {
    return type == EntityType::NUMBER;
  }

  bool Entity::isName() const {
    return type == EntityType::NAME;
  }

  bool Entity::isWildcard() const {
    return type == EntityType::WILDCARD;
  }

  bool Entity::isSubExpression() const {
    return type == EntityType::SUB_EXPRESSION;
  }

  bool Entity::isExpression() const {
    return type == EntityType::EXPRESSION;
  }

  bool Entity::isSynonym() const {
    return type == EntityType::STMT ||
      type == EntityType::READ ||
      type == EntityType::PRINT ||
      type == EntityType::CALL ||
      type == EntityType::WHILE ||
      type == EntityType::IF ||
      type == EntityType::ASSIGN ||
      type == EntityType::VARIABLE ||
      type == EntityType::CONSTANT ||
      type == EntityType::PROG_LINE ||
      type == EntityType::PROCEDURE;
  }

  bool Entity::isStmtSynonym() const {
    return type == EntityType::STMT;
  }

  bool Entity::isReadSynonym() const {
    return type == EntityType::READ;
  }

  bool Entity::isPrintSynonym() const {
    return type == EntityType::PRINT;
  }

  bool Entity::isWhileSynonym() const {
    return type == EntityType::WHILE;
  }

  bool Entity::isIfSynonym() const {
    return type == EntityType::IF;
  }

  // Clause methods
  Clause::Clause()
    : type(ClauseType::UNDEFINED) {
  }

  Clause::Clause(const ClauseType& type, const std::vector<Entity>& params)
    : type(type), params(params) {
      this->params.reserve(3); // Max number of params is 3
  }

  ClauseType Clause::getType() const {
    return type;
  }

  std::vector<Entity> Clause::getParams() const {
    return params;
  }

  void Clause::setType(const ClauseType type) {
    this->type = type;
  }

  void Clause::addParam(const Entity param) {
    params.emplace_back(param);
  }

  // Query methods
  bool Query::isBoolean() const {
    return targets.empty();
  }

  std::vector<Entity> Query::getTargets() const {
    return targets;
  }

  std::vector<Clause> Query::getClauses() const {
    return clauses;
  }

  void Query::addTarget(const Entity target) {
    targets.emplace_back(target);
  }

  void Query::addClause(const Clause clause) {
    clauses.emplace_back(clause);
  }
}