#include "PqlEvaluator.h"

#include <assert.h>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>

#include "PqlQuery.h"
#include "Pkb.h"
#include "Table.h"

namespace Pql {
  // Constructor
  PqlEvaluator::PqlEvaluator(Pkb& pkb, Pql::Query& query, std::list<std::string>& results)
    : pkb(pkb), query(query), results(results) {
  }

  // Executes query and extract results
  void PqlEvaluator::evaluateQuery() {
    // Can perform preprocessing here
    Table resultTable = executeQuery();
    extractResults(resultTable);
  }

  //  Executes query and returns the result table
  Table PqlEvaluator::executeQuery() {
    std::vector<Pql::Clause> clauses = query.getClauses();
    std::vector<Table> clauseResultTables;
    for (const Pql::Clause& clause : clauses) {
      Table clauseResult = executeClause(clause);
      clauseResultTables.emplace_back(clauseResult);
    }

    // Set finalResultTable as a table of the query target design entity type. 
    Table finalResultTable;
    Pql::Entity queryTarget = query.getTarget();
    Pql::EntityType queryTargetType = queryTarget.getType();
    switch (queryTargetType) {
    case Pql::EntityType::STMT:
      finalResultTable = pkb.getStmtTable();
      break;
    case Pql::EntityType::READ:
      finalResultTable = pkb.getReadTable();
      break;
    case Pql::EntityType::PRINT:
      finalResultTable = pkb.getPrintTable();
      break;
    case Pql::EntityType::WHILE:
      finalResultTable = pkb.getWhileTable();
      break;
    case Pql::EntityType::IF:
      finalResultTable = pkb.getIfTable();
      break;
    case Pql::EntityType::ASSIGN:
      finalResultTable = pkb.getAssignTable();
      break;
    case Pql::EntityType::VARIABLE:
      finalResultTable = pkb.getVarTable();
      break;
    case Pql::EntityType::CONSTANT:
      finalResultTable = pkb.getConstTable();
      break;
    case Pql::EntityType::PROCEDURE:
      finalResultTable = pkb.getProcTable();
      break;
    default:
      assert(false);
      break;
    }
    finalResultTable.setHeader({ queryTarget.getValue() });

    // Join each clause result table to finalResultTable
    for (const Table& table : clauseResultTables) {
      if (table.empty()) {
        // short circuit
        return Table(0);
      } else {
        // natural join on finalResultTable
        finalResultTable.naturalJoin(table);
      }
    }
    return finalResultTable;
  }

  // Execute a given clause
  // Returns the clause result table
  Table PqlEvaluator::executeClause(Pql::Clause clause) {
    Table clauseResultTable;
    Pql::ClauseType clauseType = clause.getType();
    switch (clauseType) {
    case Pql::ClauseType::FOLLOWS:
      clauseResultTable = pkb.getFollowsTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::FOLLOWS_T:
      clauseResultTable = pkb.getFollowsTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::PARENT:
      clauseResultTable = pkb.getParentTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::PARENT_T:
      clauseResultTable = pkb.getParentTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::USES_S:
      clauseResultTable = pkb.getUsesSTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::MODIFIES_S:
      clauseResultTable = pkb.getModifiesSTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case Pql::ClauseType::PATTERN_ASSIGN:
      clauseResultTable = pkb.getPatternAssignTable();
      constructPatternAssignTableFromClause(clauseResultTable, clause);
      break;
    default:
      assert(false);
      break;
    }

    return clauseResultTable;
  }

  // Constructs Design Abstraction table from given clause
  void PqlEvaluator::constructTableFromClause(Table& table, Pql::Clause clause) {
    std::vector< Pql::Entity> params = clause.getParams();
    Pql::Entity lhsEntity = params[0];
    Pql::Entity rhsEntity = params[1];
    std::string header1 = "";
    std::string header2 = "";

    // Do nothing if param is wildcard
    if (!lhsEntity.isWildcard()) {
      table.innerJoin(getTableFromEntity(lhsEntity), 0, 0); // inner join on first column
      if (lhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header1 = lhsEntity.getValue();
      }
    }

    // Do nothing if param is wildcard
    if (!rhsEntity.isWildcard()) {
      table.innerJoin(getTableFromEntity(rhsEntity), 1, 0); // inner join on second column
      if (rhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header2 = rhsEntity.getValue();
      }
    }

    table.setHeader({ header1, header2 });
  }

  // Constructs Pattern Assign table from given clause
  void PqlEvaluator::constructPatternAssignTableFromClause(Table& table, Pql::Clause& clause) {
    std::vector< Pql::Entity> params = clause.getParams();

    Pql::Entity synonymEntity = params[0];
    Pql::Entity lhsEntity = params[1];
    Pql::Entity rhsEntity = params[2];

    std::string header1 = synonymEntity.getValue();
    std::string header2 = "";
    std::string header3 = "";

    if (lhsEntity.isSynonym()) { // Guarenteed to be of type VARIABLE
      header2 = lhsEntity.getValue();
    } else if (lhsEntity.isVariableName()) {
      Table lhsTable({ "" });
      lhsTable.insertRow({ lhsEntity.getValue() });
      table.innerJoin(lhsTable, 1, 0); // inner join on second column of table
    }
    // else wildcard. do not join with any tables. 

    std::string postfixExpr = rhsEntity.getValue(); // to evaluate infix to post fix expression
    if (rhsEntity.isExpression()) {
      Table rhsTable({ "" });
      rhsTable.insertRow({ postfixExpr });
      table.innerJoin(rhsTable, 2, 0); // inner join on third column of table
    } else if (rhsEntity.isSubExpression()) {
      for (std::vector<std::string> row : table.getData()) {
        int hey = row[2].find(postfixExpr);
        if (row[2].find(postfixExpr) == std::string::npos) {
          table.deleteRow(row);
        }
      }
    }
    // else wildcard. do not join with any tables. 
    table.setHeader({ header1, header2, header3 });
  }

  // Get table from given entity 
  // If entity is a line number or variable name, return a table with a row with the entity's value.
  Table PqlEvaluator::getTableFromEntity(const Pql::Entity& entity) {
    Pql::EntityType entityType = entity.getType();
    switch (entityType) {
    case Pql::EntityType::STMT:
      return pkb.getStmtTable();
    case Pql::EntityType::READ:
      return pkb.getReadTable();
    case Pql::EntityType::PRINT:
      return pkb.getPrintTable();
    case Pql::EntityType::WHILE:
      return pkb.getWhileTable();
    case Pql::EntityType::IF:
      return pkb.getIfTable();
    case Pql::EntityType::ASSIGN:
      return pkb.getAssignTable();
    case Pql::EntityType::VARIABLE:
      return pkb.getVarTable();
    case Pql::EntityType::CONSTANT:
      return pkb.getConstTable();
    case Pql::EntityType::PROCEDURE:
      return pkb.getProcTable();
    case Pql::EntityType::LINE_NUMBER:
    case Pql::EntityType::VARIABLE_NAME:
    {
      Table table({ "" });
      table.insertRow({ entity.getValue() });
      return table;
    }
    default:
      assert(false);
      return Table(0);
    }
  }

  // Extract query result from result table
  void PqlEvaluator::extractResults(Table& resultTable) {
    if (resultTable.empty()) {
      return;
    }
    std::string headerName = query.getTarget().getValue();
    int col = resultTable.getColumnIndex(headerName);
    std::unordered_set<std::string> set; // for checking of repeated elements
    for (const auto& row : resultTable.getData()) {
      std::string element = row[col];
      if (set.count(element) == 0) {
        set.emplace(element);
        results.emplace_back(element);
      }
    }
  }
}
