#pragma once

#include <vector>
#include "PqlQuery.h"

/**
 * Sorts clauses into groups based on the algorithm described in CS3203-OPTIMIZATION.pdf.
 */
class PqlPreprocessor {
private:
public:
  std::vector<std::vector<Pql::Clause>> sortClauses(std::vector<Pql::Entity> targets, std::vector<Pql::Clause> clauses);
};
