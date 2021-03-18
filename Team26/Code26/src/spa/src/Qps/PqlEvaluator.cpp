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
  /**
   * Checks if a given such that clause requires an inner join on the left hand side
   * of the clause table given the clause type and the left hand side parameter entity.
   *
   * @param clauseType Clause type to check.
   * @param entity Left hand side parameter entity.
   * @return True if an inner join is not required. Otherwise, false.
   */
  bool canOmitJoinSuchThatLhs(const Pql::ClauseType& clauseType, const Pql::Entity& entity) {
    switch (clauseType) {
    case Pql::ClauseType::FOLLOWS:
    case Pql::ClauseType::FOLLOWS_T:
    case Pql::ClauseType::PARENT:
    case Pql::ClauseType::PARENT_T:
    case Pql::ClauseType::MODIFIES_S:
    case Pql::ClauseType::USES_S:
    case Pql::ClauseType::NEXT:
    case Pql::ClauseType::NEXT_T:
    case Pql::ClauseType::AFFECTS:
    case Pql::ClauseType::AFFECTS_T:
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE; // stmtRef

    case Pql::ClauseType::MODIFIES_P:
    case Pql::ClauseType::USES_P:
    case Pql::ClauseType::CALLS:
    case Pql::ClauseType::CALLS_T:
      return entity.getType() == Pql::EntityType::PROCEDURE; // procRef
    }
  }

  /**
   * Checks if a given such that clause requires an inner join on the right hand side
   * of the clause table given the clause type and the right hand side parameter entity.
   *
   * @param clauseType Clause type to check.
   * @param entity Right hand side parameter entity.
   * @return True if an inner join is not required. Otherwise, false.
   */
  bool canOmitJoinSuchThatRhs(const Pql::ClauseType& clauseType, const Pql::Entity& entity) {
    switch (clauseType) {
    case Pql::ClauseType::FOLLOWS:
    case Pql::ClauseType::FOLLOWS_T:
    case Pql::ClauseType::PARENT:
    case Pql::ClauseType::PARENT_T:
    case Pql::ClauseType::NEXT:
    case Pql::ClauseType::NEXT_T:
    case Pql::ClauseType::AFFECTS:
    case Pql::ClauseType::AFFECTS_T:
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE; // stmtRef

    case Pql::ClauseType::MODIFIES_S:
    case Pql::ClauseType::USES_S:
    case Pql::ClauseType::MODIFIES_P:
    case Pql::ClauseType::USES_P:
      return entity.getType() == Pql::EntityType::VARIABLE; // varRef

    case Pql::ClauseType::CALLS:
    case Pql::ClauseType::CALLS_T:
      return entity.getType() == Pql::EntityType::PROCEDURE; // procRef
    }
  }

  /**
   * Checks if a given entity requires an attribute reference mapping from stmt number
   * to the attribute reference. (True for call.procName, read.varName and print.varName)
   *
   * @param entity Entity to check.
   * @return True if an attribute reference mapping is required.
   */
  bool needsAttrRefMapping(const Pql::Entity& entity) {
    switch (entity.getType()) {
    case Pql::EntityType::CALL:
      return entity.getAttributeRefType() == Pql::AttributeRefType::PROC_NAME;
    case Pql::EntityType::READ:
    case Pql::EntityType::PRINT:
      return entity.getAttributeRefType() == Pql::AttributeRefType::VAR_NAME;
    default:
      return false;
    }
  }
}

namespace Pql {
  // Constructor
  PqlEvaluator::PqlEvaluator(Pkb& pkb, Query& query, std::list<std::string>& results)
    : pkb(pkb), query(query), results(results) {
  }

  // Executes query and extract results
  void PqlEvaluator::evaluateQuery() {
    if (canShortCircuit()) {
      extractResults(Table());
    } else {
      extractResults(executeQuery());
    }
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
        if (lhsEntity == rhsEntity) {
          return true;
        }

        const bool isRhsSmallerThanLhs =
          lhsEntity.isNumber() &&
          rhsEntity.isNumber() &&
          rhsEntity.getValue() < lhsEntity.getValue();
        if (isRhsSmallerThanLhs) {
          return true;
        }
      }
      break;
      case ClauseType::PARENT:
      case ClauseType::PARENT_T:
      {
        const Entity& lhsEntity = params[0];
        const Entity& rhsEntity = params[1];
        if (lhsEntity == rhsEntity) {
          return true;
        }
        const bool isRhsSmallerThanLhs =
          lhsEntity.isNumber() &&
          rhsEntity.isNumber() &&
          rhsEntity.getValue() < lhsEntity.getValue();
        if (isRhsSmallerThanLhs) {
          return true;
        }

        const bool isLhsNotContainerStmt =
          lhsEntity.isSynonym() &&
          !lhsEntity.isStmtSynonym() &&
          !lhsEntity.isWhileSynonym() &&
          !lhsEntity.isIfSynonym();
        if (isLhsNotContainerStmt) {
          return true;
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

    const std::vector<Entity>& queryTargets = query.getTargets();

    Table finalResultTable = Table({ "" });
    finalResultTable.insertRow({ "" });

    // Join each clause result table to finalResultTable
    for (Table& table : clauseResultTables) {
      if (table.empty()) {
        // short circuit
        return Table();
      } else {
        // natural join on finalResultTable
        bool areAllHeadersEmptyStrings = true;
        for (const std::string& header : table.getHeader()) {
          if (!header.empty()) {
            areAllHeadersEmptyStrings = false;
          }
        }

        if (!areAllHeadersEmptyStrings) {
          table.dropColumn(""); // drop empty column - Guaranteed to be only 1 column max
          finalResultTable.naturalJoin(table);
        }
        // Don't need to join if all headers empty strings
      }
    }

    // Join remaining query targets that is not found in the finalResultTable
    for (Entity target : queryTargets) {
      const bool doesTargetExist = finalResultTable.getColumnIndex(target.getValue()) != -1;
      if (!doesTargetExist) {
        Table& entityTable = getTableFromEntity(target);
        entityTable.setHeader({ target.getValue() });
        finalResultTable.naturalJoin(entityTable);
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
    case ClauseType::USES_P:
      clauseResultTable = pkb.getUsesPTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::MODIFIES_S:
      clauseResultTable = pkb.getModifiesSTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::MODIFIES_P:
      clauseResultTable = pkb.getModifiesPTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::CALLS:
      clauseResultTable = pkb.getCallsTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::CALLS_T:
      clauseResultTable = pkb.getCallsTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT:
      clauseResultTable = pkb.getNextTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT_T:
      clauseResultTable = pkb.getNextTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS:
      clauseResultTable = pkb.getAffectsTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS_T:
      clauseResultTable = pkb.getAffectsTTable();
      constructTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PATTERN_ASSIGN:
      clauseResultTable = pkb.getPatternAssignTable();
      constructPatternAssignTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PATTERN_IF:
      clauseResultTable = pkb.getPatternIfTable();
      constructPatternCondTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PATTERN_WHILE:
      clauseResultTable = pkb.getPatternWhileTable();
      constructPatternCondTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::WITH:
      constructWithTableFromClause(clauseResultTable, clause);
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
      if (lhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header1 = lhsEntity.getValue();
      }
      // Short circuit
      if (!canOmitJoinSuchThatLhs(clause.getType(), lhsEntity)) {
        clauseResultTable.innerJoin(getTableFromEntity(lhsEntity), 0, 0); // inner join on first column
      }
    }

    // Do nothing if param is wildcard
    if (!rhsEntity.isWildcard()) {
      if (rhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header2 = rhsEntity.getValue();
      }
      // Short circuit
      if (!canOmitJoinSuchThatRhs(clause.getType(), rhsEntity)) {
        clauseResultTable.innerJoin(getTableFromEntity(rhsEntity), 1, 0); // inner join on second column
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

    if (lhsEntity.isSynonym()) { // Guaranteed to be of type VARIABLE
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

  void PqlEvaluator::constructPatternCondTableFromClause(Table& clauseResultTable, const Clause& clause) const {
    const std::vector<Entity>& params = clause.getParams();

    const Entity& synonymEntity = params[0];
    const Entity& condEntity = params[1];

    std::string header2 = ""; // only second header can have different possible values

    if (condEntity.isSynonym()) { // Guaranteed to be of type VARIABLE
      header2 = condEntity.getValue();
    } else if (condEntity.isName()) {
      Table condTable({ "" });
      condTable.insertRow({ condEntity.getValue() });
      clauseResultTable.innerJoin(condTable, 1, 0); // inner join on second column of table
    }
    // else wildcard. do not join with any tables. 

    clauseResultTable.setHeader({ synonymEntity.getValue(), header2 });
  }

  void PqlEvaluator::constructWithTableFromClause(Table& clauseResultTable, const Clause& clause) const {
    const std::vector<Entity>& params = clause.getParams();
    const Entity& lhsEntity = params[0];
    const Entity& rhsEntity = params[1];

    // LHS and RHS are both numbers or both names
    if (lhsEntity.isName() && rhsEntity.isName() ||
      lhsEntity.isNumber() && rhsEntity.isNumber()) {
      clauseResultTable.setHeader({ "" });
      if (lhsEntity.getValue() == rhsEntity.getValue()) {
        clauseResultTable.insertRow({ "" }); // Dummy row to signify True
      }
      return;
    }

    // LHS is number or name
    if (lhsEntity.isName() || lhsEntity.isNumber()) {
      Table lhsTable({ "" });
      lhsTable.insertRow({ lhsEntity.getValue() });

      if (needsAttrRefMapping(rhsEntity)) {
        clauseResultTable = std::move(getAttrRefMappingTableFromEntity(rhsEntity));
        clauseResultTable.innerJoin(lhsTable, 1, 0); // Two column
        clauseResultTable.setHeader({ rhsEntity.getValue(), "" });
      } else {
        clauseResultTable = std::move(getTableFromEntity(rhsEntity));
        clauseResultTable.innerJoin(lhsTable, 0, 0); // One column
        clauseResultTable.setHeader({ rhsEntity.getValue() });
      }
      return;
    }

    // RHS is number or name
    if (rhsEntity.isName() || rhsEntity.isNumber()) {
      Table rhsTable({ "" });
      rhsTable.insertRow({ rhsEntity.getValue() });

      if (needsAttrRefMapping(lhsEntity)) {
        clauseResultTable = std::move(getAttrRefMappingTableFromEntity(lhsEntity));
        clauseResultTable.innerJoin(rhsTable, 1, 0); // Two column
        clauseResultTable.setHeader({ lhsEntity.getValue(), "" });
      } else {
        clauseResultTable = std::move(getTableFromEntity(lhsEntity));
        clauseResultTable.innerJoin(rhsTable, 0, 0); // One column
        clauseResultTable.setHeader({ lhsEntity.getValue() });
      }
      return;
    }

    // LHS and RHS not constants
    int lhsColumnIdxToJoin = -1;
    if ((lhsEntity.isProgLineSynonym() || lhsEntity.isAttributeRef()) &&
      (rhsEntity.isProgLineSynonym() || rhsEntity.isAttributeRef())) {
      if (needsAttrRefMapping(lhsEntity)) {
        clauseResultTable = std::move(getAttrRefMappingTableFromEntity(lhsEntity)); // Two column
        clauseResultTable.setHeader({ lhsEntity.getValue(), "" });
        lhsColumnIdxToJoin = 1;
      } else {
        clauseResultTable = getTableFromEntity(lhsEntity);
        clauseResultTable.setHeader({ lhsEntity.getValue() }); // One column
        lhsColumnIdxToJoin = 0;
      }

      if (needsAttrRefMapping(rhsEntity)) {
        Table& rhsTable = getAttrRefMappingTableFromEntity(rhsEntity); // Two column
        rhsTable.setHeader({ rhsEntity.getValue(), "" });
        clauseResultTable.innerJoin(rhsTable, lhsColumnIdxToJoin, 1);
      } else {
        Table& rhsTable = getTableFromEntity(rhsEntity);
        clauseResultTable.innerJoin(rhsTable, lhsColumnIdxToJoin, 0); // One column

        if (lhsColumnIdxToJoin == 0) {
          // duplicate first col
          Table finalTable(2);
          for (Row row : clauseResultTable.getData()) {
            finalTable.insertRow({ row[0], row[0] });
          }
          clauseResultTable = std::move(finalTable);
        }
        clauseResultTable.setHeader({ lhsEntity.getValue(), rhsEntity.getValue() });
      }
      return;
    }

    assert(false); // Guaranteed to be the above cases by the PqlParser
  }

  // Get table from given entity 
  // If entity is a line number or variable name, return a table with a row with the entity's value.
  Table PqlEvaluator::getTableFromEntity(const Entity& entity) const {
    switch (entity.getType()) {
    case EntityType::PROG_LINE:
    case EntityType::STMT:
      return pkb.getStmtTable();
    case EntityType::READ:
      return pkb.getReadTable();
    case EntityType::PRINT:
      return pkb.getPrintTable();
    case EntityType::CALL:
      return pkb.getCallTable();
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

  Table PqlEvaluator::getAttrRefMappingTableFromEntity(const Entity& entity) const {
    if (!needsAttrRefMapping(entity)) {
      assert(false);
    }
    switch (entity.getType()) {
    case EntityType::CALL:
      return pkb.getCallProcTable();
    case EntityType::PRINT:
      return pkb.getPrintVarTable();
    case EntityType::READ:
      return pkb.getReadVarTable();
    default:
      assert(false);
      return Table();
    }
  }


  // Extract query result from result table
  void PqlEvaluator::extractResults(const Table& resultTable) const {
    // ===============
    // BOOLEAN Select
    // ===============
    if (query.isBoolean()) {
      if (resultTable.empty()) {
        results.emplace_back("FALSE");
      } else {
        results.emplace_back("TRUE");
      }
      return;
    }

    // =======================
    //  Tuple / Single Select
    // =======================

    // Short-circuit
    if (resultTable.empty()) {
      return;
    }

    const std::vector<Entity>& queryTargets = query.getTargets();
    std::vector<int> columns;
    for (Entity target : queryTargets) {
      columns.emplace_back(resultTable.getColumnIndex(target.getValue()));
    }

    std::unordered_set<std::string> set; // for checking of repeated elements
    set.reserve(resultTable.size()); // optimization to avoid rehashing
    for (const Row& row : resultTable.getData()) {
      std::string outputLine;
      for (int i = 0; i < columns.size(); i++) {
        const int rowColumnIdx = columns[i];
        std::string element;

        if (needsAttrRefMapping(queryTargets[i])) {
          element = mapEntityToAttrRef(queryTargets[i], std::stoi(row[rowColumnIdx]));
        } else {
          element = row[rowColumnIdx];
        }

        outputLine.append(element).append(" ");
      }
      outputLine.pop_back();

      if (set.count(outputLine) == 0) {
        set.emplace(outputLine);
        results.emplace_back(outputLine);
      }
    }
  }

  std::string PqlEvaluator::mapEntityToAttrRef(const Pql::Entity& entity, const int stmtNumber) const {
    switch (entity.getType()) {
    case Pql::EntityType::CALL:
      return pkb.getProcNameFromCallStmt(stmtNumber);
    case Pql::EntityType::READ:
      return pkb.getVarNameFromReadStmt(stmtNumber);
    case Pql::EntityType::PRINT:
      return pkb.getVarNameFromPrintStmt(stmtNumber);
    default:
      assert(false);
      return "";
    }
  }
}
