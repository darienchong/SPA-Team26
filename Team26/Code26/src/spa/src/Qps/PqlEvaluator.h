#pragma once

#include <string>
#include <list>

#include "PqlQuery.h"
#include "Pkb.h"
#include "Table.h"

namespace Pql {
  class PqlEvaluator {
  private:
    Pql::Query& query;
    Pkb& pkb;
    std::list<std::string>& results;

    //  Executes query and returns a table
    Table executeQuery();

    // Execute a given clause
    Table executeClause(Pql::Clause clause);

    // Constructs Design Abstraction table from given clause
    void constructTableFromClause(Table& table, Pql::Clause clause);

    // Constructs Pattern Assign table from given clause
    void constructPatternAssignTableFromClause(Table& clauseResultTable, Pql::Clause& clause);

    // Get table from given entity
    Table getTableFromEntity(const Pql::Entity& e);

    // Extract query result from result table
    void extractResults(Table& resultTable);

  public:
    // Constructor
    PqlEvaluator(Pkb& pkb, Pql::Query& query, std::list<std::string>& results);

    // Executes query and formats results
    void evaluateQuery();
  };
}
