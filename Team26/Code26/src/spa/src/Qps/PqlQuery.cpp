#include "PqlQuery.h"

#include <string>

// Entity methods
Pql::Entity::Entity()
  : type(Pql::EntityType::UNDEFINED) {
}

Pql::Entity::Entity(const Pql::EntityType& type, const std::string& value)
  : type(type), value(value) {
}

Pql::EntityType Pql::Entity::getType() const {
  return type;
}

std::string Pql::Entity::getValue() const {
  return value;
}

bool Pql::Entity::isStmtNumber() const {
  return type == Pql::EntityType::STMT_NUMBER;
}

bool Pql::Entity::isVariableName() const {
  return type == Pql::EntityType::VARIABLE_NAME;
}

bool Pql::Entity::isWildcard() const {
  return type == Pql::EntityType::WILDCARD;
}

bool Pql::Entity::isSubExpression() const {
  return type == Pql::EntityType::SUB_EXPRESSION;
}

bool Pql::Entity::isExpression() const {
  return type == Pql::EntityType::EXPRESSION;
}

bool Pql::Entity::isSynonym() const {
  return type == Pql::EntityType::STMT ||
    type == Pql::EntityType::READ ||
    type == Pql::EntityType::PRINT ||
    type == Pql::EntityType::WHILE ||
    type == Pql::EntityType::IF ||
    type == Pql::EntityType::ASSIGN ||
    type == Pql::EntityType::VARIABLE ||
    type == Pql::EntityType::CONSTANT ||
    type == Pql::EntityType::PROCEDURE;
}

bool Pql::Entity::isStmtSynonym() const {
  return type == Pql::EntityType::STMT;
}

bool Pql::Entity::isReadSynonym() const {
  return type == Pql::EntityType::READ;
}

bool Pql::Entity::isPrintSynonym() const {
  return type == Pql::EntityType::PRINT;
}

bool Pql::Entity::isWhileSynonym() const {
  return type == Pql::EntityType::WHILE;
}

bool Pql::Entity::isIfSynonym() const {
  return type == Pql::EntityType::IF;
}

// Clause methods
Pql::Clause::Clause()
  : type(ClauseType::UNDEFINED) {
}

Pql::Clause::Clause(const Pql::ClauseType& type, const std::vector<Pql::Entity>& params)
  : type(type), params(params) {
  this->params.reserve(3); // Max number of params is 3
}

Pql::ClauseType Pql::Clause::getType() const {
  return type;
}

std::vector<Pql::Entity> Pql::Clause::getParams() const {
  return params;
}

void Pql::Clause::setType(const ClauseType type) {
  this->type = type;
}

void Pql::Clause::addParam(const Entity param) {
  params.emplace_back(param);
}

// Query methods
Pql::Entity Pql::Query::getTarget() const {
  return target;
}

std::vector<Pql::Clause> Pql::Query::getClauses() const {
  return clauses;
}

void Pql::Query::setTarget(const Pql::Entity target) {
  this->target = target;
}

void Pql::Query::addClause(const Clause clause) {
  clauses.emplace_back(clause);
}