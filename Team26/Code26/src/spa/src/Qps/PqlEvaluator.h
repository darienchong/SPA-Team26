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

    /**
     * @brief Execute the query in PqlEvaluator using the PKB and returns the result in a Table.
     *
     * @return Table containing the result.
     */
    Table executeQuery() const;

    /**
     * Check if any clause in the query can be short-circuited. 
     * Short-circuit means that the result will be empty. 
     * 
     * @return True if the query can be short-circuited. Otherwise, false.
     */
    bool canShortCircuit();

    /**
     * Executes a given clause and returns the clause result table.
     *
     * @param clause Clause to be executed.
     * @return Clause result table.
     */
    Table executeClause(const Pql::Clause& clause) const;

    /**
     * Constructs a clause result table given a such that clause.
     *
     * @param table Table to be constructed.
     * @param clause Such that clause object.
     */
    void constructTableFromClause(Table& clauseResultTable, const Pql::Clause& clause) const;

    /**
     * Constructs a clause result table given a clause of type PATTERN_ASSIGN.
     *
     * @param clauseResultTable Table to be constructed.
     * @param clause Pattern assign clause object.
     */
    void constructPatternAssignTableFromClause(Table& clauseResultTable, const Pql::Clause& clause) const;

    /**
     * Helper function to get the corresponding Table from the PKB when given an entity.
     * E.g. Entity with EntityType of STMT will return the stmtTable from PKB.
     *
     * @param entity Given entity to retrieve the corresponding table from PKB.
     * @return Table corresponding to the given entity.
     */
    Table getTableFromEntity(const Pql::Entity& entity) const;

    /**
     * @brief Extracts the result from the given Table and populates the results list of the PqlEvaluator.
     *
     * @param resultTable Final result table from execution of all clauses.
     */
    void extractResults(const Table& resultTable) const;

  public:
    /**
     * Constructs a PQL Evaluator with the give PKB, query representation object and result list to be filled.
     *
     * @param pkb PKB.
     * @param query Query representation object.
     * @param results Result list to be filled.
     */
    PqlEvaluator(Pkb& pkb, Pql::Query& query, std::list<std::string>& results);

    /**
     * @brief Evaluates the query using the given PKB and stores the result in the results list of the PqlEvaluator.
     */
    void evaluateQuery();
  };
}
