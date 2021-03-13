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

namespace {

  bool isSameTypeAndArguement(const Pql::Entity& e1, const Pql::Entity& e2) {
    return e1.getType() == e2.getType() && e1.getValue() == e2.getValue();
  }

  bool isLhsSmallerThanRhs(const Pql::Entity& e1, const Pql::Entity& e2) {
    return e1.getValue() < e2.getValue();
  }
}

namespace Pql {
  // Constructor
  PqlEvaluator::PqlEvaluator(Pkb& pkb, Query& query, std::list<std::string>& results)
    : pkb(pkb), query(query), results(results) {
  }

  // Executes query and extract results
  void PqlEvaluator::evaluateQuery() {
    if (!canShortCircuit()) {
      extractResults(executeQuery());
    }
    // Do nothing if can short-circuit
  }

  bool PqlEvaluator::canShortCircuit() {
    const std::vector<Clause>& clauses = query.getClauses();
    for (const Clause& clause : clauses) {
      const std::vector<Entity>& params = clause.getParams();
      switch (clause.getType()) {
      case ClauseType::FOLLOWS:
      case ClauseType::FOLLOWS_T:
      {
        const Entity& lhsEntity = params[0];
        const Entity& rhsEntity = params[1];
        if (isSameTypeAndArguement(lhsEntity, rhsEntity)) {
          return true;
        }
        if (lhsEntity.isNumber() && rhsEntity.isNumber()) { // check if lhs stmt number is equal or larger than rhs stmt number
          return !isLhsSmallerThanRhs(lhsEntity, rhsEntity);
        }
      }
      break;
      case ClauseType::PARENT:
      case ClauseType::PARENT_T:
      {
        const Entity& lhsEntity = params[0];
        const Entity& rhsEntity = params[1];
        if (isSameTypeAndArguement(lhsEntity, rhsEntity)) {
          return true;
        }
        if (lhsEntity.isNumber() && rhsEntity.isNumber()) { // Check if lhs stmt number is equal or larger than rhs stmt number
          return !isLhsSmallerThanRhs(lhsEntity, rhsEntity);
        }
        if (lhsEntity.isSynonym()) { // Check if lhs is a container stmt
          return !(lhsEntity.isStmtSynonym() || lhsEntity.isWhileSynonym() || lhsEntity.isIfSynonym());
        }
      }
      break;
      case ClauseType::USES_S:
      {
        const Entity& lhsEntity = params[0];
        if (lhsEntity.isReadSynonym()) {
          return true;
        }
      }
      break;
      case ClauseType::MODIFIES_S:
      {
        const Entity& lhsEntity = params[0];
        if (lhsEntity.isPrintSynonym()) {
          return true;
        }
      }
      break;
      }
    }
    return false;
  }

  //  Executes query and returns the result table
  Table PqlEvaluator::executeQuery() const {
    const std::vector<Clause>& clauses = query.getClauses();
    std::vector<Table> clauseResultTables;
    clauseResultTables.reserve(clauses.size()); // Optimization to avoid resizing of vector
    for (const Clause& clause : clauses) {
      Table clauseResult = executeClause(clause);
      clauseResultTables.emplace_back(clauseResult);
    }

    // Set finalResultTable as a table of the query target design entity type. 
    Table finalResultTable;
    const std::vector<Entity>& queryTargets = query.getTargets();
    const Entity& queryTarget = queryTargets[0];
    const EntityType queryTargetType = queryTarget.getType();
    switch (queryTargetType) {
    case EntityType::STMT:
      finalResultTable = pkb.getStmtTable();
      break;
    case EntityType::READ:
      finalResultTable = pkb.getReadTable();
      break;
    case EntityType::PRINT:
      finalResultTable = pkb.getPrintTable();
      break;
    case EntityType::WHILE:
      finalResultTable = pkb.getWhileTable();
      break;
    case EntityType::IF:
      finalResultTable = pkb.getIfTable();
      break;
    case EntityType::ASSIGN:
      finalResultTable = pkb.getAssignTable();
      break;
    case EntityType::VARIABLE:
      finalResultTable = pkb.getVarTable();
      break;
    case EntityType::CONSTANT:
      finalResultTable = pkb.getConstTable();
      break;
    case EntityType::PROCEDURE:
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
        return Table();
      } else {
        // natural join on finalResultTable
        finalResultTable.naturalJoin(table);
      }
    }
    return finalResultTable;
  }

  // Execute a given clause
  // Returns the clause result table
  Table PqlEvaluator::executeClause(const Clause& clause) const {
    Table clauseResultTable;
    const ClauseType& clauseType = clause.getType();
    switch (clauseType) {
    case ClauseType::FOLLOWS:
      clauseResultTable = pkb.getFollowsTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::FOLLOWS_T:
      clauseResultTable = pkb.getFollowsTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PARENT:
      clauseResultTable = pkb.getParentTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PARENT_T:
      clauseResultTable = pkb.getParentTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::USES_S:
      clauseResultTable = pkb.getUsesSTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::MODIFIES_S:
      clauseResultTable = pkb.getModifiesSTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PATTERN_ASSIGN:
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
  void PqlEvaluator::constructTableFromClause(Table& clauseResultTable, const Clause& clause) const {
    const std::vector<Entity>& params = clause.getParams();
    const Entity& lhsEntity = params[0];
    const Entity& rhsEntity = params[1];
    std::string header1 = "";
    std::string header2 = "";

    // Do nothing if param is wildcard
    if (!lhsEntity.isWildcard()) {
      clauseResultTable.innerJoin(getTableFromEntity(lhsEntity), 0, 0); // inner join on first column
      if (lhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header1 = lhsEntity.getValue();
      }
    }

    // Do nothing if param is wildcard
    if (!rhsEntity.isWildcard()) {
      clauseResultTable.innerJoin(getTableFromEntity(rhsEntity), 1, 0); // inner join on second column
      if (rhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header2 = rhsEntity.getValue();
      }
    }

    clauseResultTable.setHeader({ header1, header2 });
  }

  // Constructs Pattern Assign table from given clause
  void PqlEvaluator::constructPatternAssignTableFromClause(Table& clauseResultTable, const Clause& clause) const {
    const std::vector<Entity>& params = clause.getParams();

    const Entity& synonymEntity = params[0];
    const Entity& lhsEntity = params[1];
    const Entity& rhsEntity = params[2];

    std::string header2 = ""; // only second header can have different possible values

    if (lhsEntity.isSynonym()) { // Guarenteed to be of type VARIABLE
      header2 = lhsEntity.getValue();
    } else if (lhsEntity.isName()) {
      Table lhsTable({ "" });
      lhsTable.insertRow({ lhsEntity.getValue() });
      clauseResultTable.innerJoin(lhsTable, 1, 0); // inner join on second column of table
    }
    // else wildcard. do not join with any tables. 

    std::string postfixExpr = rhsEntity.getValue(); // to evaluate infix to post fix expression
    if (rhsEntity.isExpression()) {
      Table rhsTable({ "" });
      rhsTable.insertRow({ postfixExpr });
      clauseResultTable.innerJoin(rhsTable, 2, 0); // inner join on third column of table
    } else if (rhsEntity.isSubExpression()) {
      for (std::vector<std::string> row : clauseResultTable.getData()) {
        int hey = row[2].find(postfixExpr);
        if (row[2].find(postfixExpr) == std::string::npos) {
          clauseResultTable.deleteRow(row);
        }
      }
    }
    // else wildcard. do not join with any tables. 

    clauseResultTable.setHeader({ synonymEntity.getValue(), header2, "" });
  }

  // Get table from given entity 
  // If entity is a line number or variable name, return a table with a row with the entity's value.
  Table PqlEvaluator::getTableFromEntity(const Entity& entity) const {
    switch (entity.getType()) {
    case EntityType::STMT:
      return pkb.getStmtTable();
    case EntityType::READ:
      return pkb.getReadTable();
    case EntityType::PRINT:
      return pkb.getPrintTable();
    case EntityType::WHILE:
      return pkb.getWhileTable();
    case EntityType::IF:
      return pkb.getIfTable();
    case EntityType::ASSIGN:
      return pkb.getAssignTable();
    case EntityType::VARIABLE:
      return pkb.getVarTable();
    case EntityType::CONSTANT:
      return pkb.getConstTable();
    case EntityType::PROCEDURE:
      return pkb.getProcTable();
    case EntityType::NUMBER:
    case EntityType::NAME:
    {
      Table table({ "" });
      table.insertRow({ entity.getValue() });
      return table;
    }
    default:
      assert(false);
      return Table();
    }
  }

  // Extract query result from result table
  void PqlEvaluator::extractResults(const Table& resultTable) const {
    if (resultTable.empty()) {
      return;
    }
    const std::string& headerName = query.getTargets()[0].getValue();
    int col = resultTable.getColumnIndex(headerName);
    std::unordered_set<std::string> set; // for checking of repeated elements
    set.reserve(resultTable.size()); // optimization to avoid rehashing
    for (const Row& row : resultTable.getData()) {
      std::string element = row[col];
      if (set.count(element) == 0) {
        set.emplace(element);
        results.emplace_back(element);
      }
    }
  }
}
