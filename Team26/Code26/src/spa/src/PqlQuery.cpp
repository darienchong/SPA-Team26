#include "PqlQuery.h"

#include <string>

Pql::Entity::Entity()
  : type(Pql::EntityType::UNDEFINED) {}

Pql::Entity::Entity(const Pql::EntityType& type, const std::string& value)
  : type(type), value(value) {}

Pql::Clause::Clause()
  : type(ClauseType::UNDEFINED) {}

Pql::Clause::Clause(const Pql::ClauseType& type, const std::vector<Pql::Entity>& params)
  : type(type), params(params) {}

void Pql::Clause::setType(const ClauseType type) {
  this->type = type;
}

void Pql::Clause::addParam(const Entity param) {
  this->params.emplace_back(param);
}

void Pql::Query::setTarget(const Pql::Entity target) {
  this->target = target;
}

void Pql::Query::addClause(const Clause clause) {
  this->clauses.emplace_back(clause);
}