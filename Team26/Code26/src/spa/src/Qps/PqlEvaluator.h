#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include <unordered_map>

#include "PqlQuery.h"
#include "Pkb.h"
#include "Table.h"

namespace Pql {
  class PqlEvaluator {
  private:
    Query& query;
    Pkb& pkb;
    std::list<std::string>& results;

    /**
     * @brief Execute the query in PqlEvaluator using the PKB and returns the result in a Table.
     *
     * @return Table containing the result.
     */
    Table executeQuery() const;

    /**
     * Executes a given clause and returns the clause result table.
     *
     * @param clause Clause to be executed.
     * @return Clause result table.
     */
    Table executeClause(const Clause& clause) const;

    /**
     * Constructs a clause result table given a such that clause.
     *
     * @param table Table to be constructed.
     * @param clause Such that clause object.
     */
    void constructSuchThatTableFromClause(Table& clauseResultTable, const Clause& clause) const;

    /**
     * Constructs a clause result table given a clause of type PATTERN_ASSIGN.
     *
     * @param clauseResultTable Table to be constructed.
     * @param clause Pattern assign clause object.
     */
    void constructPatternAssignTableFromClause(Table& clauseResultTable, const Clause& clause) const;

    /**
     * Constructs a clause result table given a clause of type PATTERN_IF or PATTERN_WHILE.
     *
     * @param clauseResultTable Table to be constructed.
     * @param clause Pattern_If clause object or PATTERN_WHILE clause object.
     */
    void constructPatternCondTableFromClause(Table& clauseResultTable, const Clause& clause) const;


    /**
     * Constructs a clause result table given a with clause
     *
     * @param clauseResultTable Table to be constructed.
     * @param clause with clause object.
     */
    void constructWithTableFromClause(Table& clauseResultTable, const Clause& clause) const;

    /**
     * Helper function to get the corresponding Table from the PKB when given a synonym entity.
     * E.g. Entity with EntityType of STMT will return the stmtTable from PKB.
     *
     * @param entity Given entity to retrieve the corresponding table from PKB.
     * @return Table corresponding to the given entity.
     */
    Table getTableFromEntity(const Entity& synonymEntity) const;

    /**
     * Helper function to get the corresponding values of a given entity from the PKB.
     *
     * @param entity Given entity to retrieve the corresponding values.
     * @return Set of possible values of the given entity.
     */
    std::unordered_set<int> getValuesFromEntity(const Entity& synonymEntity) const;

    /**
     * Helper function to get the corresponding attribute reference mapping Table from the PKB when given an entity.
     * E.g. Entity with EntityType of STMT will return the stmtTable from PKB.
     *
     * @param entity Given entity to retrieve the corresponding attribute reference mapping table from PKB.
     * @return Table of attribute reference mapping.
     */
    Table PqlEvaluator::getAttrRefMappingTableFromEntity(const Entity& entity) const;

    /**
     * Extracts the result from the given Table and populates the results list of the PqlEvaluator.
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
    PqlEvaluator(Pkb& pkb, Query& query, std::list<std::string>& results);

    /**
     * @brief Evaluates the query using the given PKB and stores the result in the results list of the PqlEvaluator.
     */
    void evaluateQuery();
  };
}
