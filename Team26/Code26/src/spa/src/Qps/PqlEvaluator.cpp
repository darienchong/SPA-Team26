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
#include "PqlOptimizer.h"

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
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE; // stmtRef
    case Pql::ClauseType::AFFECTS:
    case Pql::ClauseType::AFFECTS_T:
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE ||
        entity.getType() == Pql::EntityType::ASSIGN; // stmtRef + assignStmts

    case Pql::ClauseType::MODIFIES_P:
    case Pql::ClauseType::USES_P:
    case Pql::ClauseType::CALLS:
    case Pql::ClauseType::CALLS_T:
      return entity.getType() == Pql::EntityType::PROCEDURE; // procRef
    default:
      return false;
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
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE; // stmtRef
    case Pql::ClauseType::AFFECTS:
    case Pql::ClauseType::AFFECTS_T:
      return entity.getType() == Pql::EntityType::STMT ||
        entity.getType() == Pql::EntityType::PROG_LINE ||
        entity.getType() == Pql::EntityType::ASSIGN; // stmtRef + assignStmts

    case Pql::ClauseType::MODIFIES_S:
    case Pql::ClauseType::USES_S:
    case Pql::ClauseType::MODIFIES_P:
    case Pql::ClauseType::USES_P:
      return entity.getType() == Pql::EntityType::VARIABLE; // varRef

    case Pql::ClauseType::CALLS:
    case Pql::ClauseType::CALLS_T:
      return entity.getType() == Pql::EntityType::PROCEDURE; // procRef
    default:
      return false;
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
        if (!lhsEntity.isWildcard() && lhsEntity == rhsEntity) {
          return true;
        }

        const bool isRhsSmallerThanLhs =
          lhsEntity.isNumber() &&
          rhsEntity.isNumber() &&
          stoll(rhsEntity.getValue()) < stoll(lhsEntity.getValue());
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
        if (!lhsEntity.isWildcard() && lhsEntity == rhsEntity) {
          return true;
        }
        const bool isRhsSmallerThanLhs =
          lhsEntity.isNumber() &&
          rhsEntity.isNumber() &&
          stoll(rhsEntity.getValue()) < stoll(lhsEntity.getValue());
        if (isRhsSmallerThanLhs) {
          return true;
        }

        const bool isLhsNotContainerStmt =
          lhsEntity.isSynonym() &&
          !lhsEntity.isStmtSynonym() &&
          !lhsEntity.isWhileSynonym() &&
          !lhsEntity.isIfSynonym() &&
          !lhsEntity.isProgLineSynonym();
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
      if (clauseResult.empty()) {
        // short circuit
        return Table();
      }
      clauseResultTables.emplace_back(clauseResult);
    }

    Table finalResultTable = Table({ "" });
    finalResultTable.insertRow({ 0 }); // dummy row


    if (!clauses.empty()) {
      // Initialise optimizer to get order of joining tables
      Optimizer optimizer(clauseResultTables);
      std::vector<int> order = optimizer.getOptimizedOrder();

       // Join each clause result table to finalResultTable
      for (size_t i = 0; i < clauseResultTables.size(); i++) {
        const int tableIndex = order[i];

        // natural join on finalResultTable
        bool areAllHeadersEmptyStrings = true;
        for (const std::string& header : clauseResultTables[tableIndex].getHeader()) {
          if (!header.empty()) {
            areAllHeadersEmptyStrings = false;
          }
        }

        if (!areAllHeadersEmptyStrings) {
          clauseResultTables[tableIndex].dropColumn(""); // drop empty column - Guaranteed to be only 1 column max
          finalResultTable.naturalJoin(clauseResultTables[tableIndex]);
        }
        // Don't need to join if all headers empty strings
      }
    }

    // Join remaining query targets that is not found in the finalResultTable
    const std::vector<Entity>& queryTargets = query.getTargets();
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

        // Short circuit
        if (!canOmitJoinSuchThatLhs(clause.getType(), lhsEntity)) {
          clauseResultTable.innerJoin(getTableFromEntity(lhsEntity), 0, 0); // inner join on first column
        }
      } else if (lhsEntity.isName()) {
        clauseResultTable.filterColumn(0, { pkb.getIntRefFromEntity(lhsEntity.getValue()) });
      } else if (lhsEntity.isNumber()) {
        clauseResultTable.filterColumn(0, { std::stoi(lhsEntity.getValue()) });
      } else {
        assert(false);
      }
    }

    // If LHS synonym == RHS synonym, need to make sure table value LHS = RHS
    if (lhsEntity.isSynonym() && lhsEntity == rhsEntity) {
      for (const Row& row : clauseResultTable.getData()) {
        const bool isLhsEqualRhs = row[0] == row[1];
        if (!isLhsEqualRhs) {
          clauseResultTable.deleteRow(row);
        }
      }
      clauseResultTable.setHeader({ header1, "" });
      return;
    }

    // Do nothing if param is wildcard
    if (!rhsEntity.isWildcard()) {
      if (rhsEntity.isSynonym()) {
        // Update header name to reflect name of synonym
        header2 = rhsEntity.getValue();

        // Short circuit
        if (!canOmitJoinSuchThatRhs(clause.getType(), rhsEntity)) {
          clauseResultTable.innerJoin(getTableFromEntity(rhsEntity), 1, 0); // inner join on second column
        }
      } else if (rhsEntity.isName()) {
        clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(rhsEntity.getValue()) });
      } else if (rhsEntity.isNumber()) {
        clauseResultTable.filterColumn(1, { std::stoi(rhsEntity.getValue()) });
      } else {
        assert(false);
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
      clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(lhsEntity.getValue()) }); // filter second column 
    }
    // else wildcard. do not join with any tables. 

    std::string postfixExpr = rhsEntity.getValue();
    if (rhsEntity.isExpression()) {
      clauseResultTable.filterColumn(2, { pkb.getIntRefFromEntity(postfixExpr) }); // filter third column
    } else if (rhsEntity.isSubExpression()) {
      for (const Row& row : clauseResultTable.getData()) {
        const bool doesNotMatch = pkb.getEntityFromIntRef(row[2]).find(postfixExpr) == std::string::npos;
        if (doesNotMatch) {
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
      clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(condEntity.getValue()) }); // filter second column
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
        clauseResultTable.insertRow({ 0 }); // Dummy row to signify True
      }
      return;
    }

    // LHS and RHS both not constants - Special case: stmt.stmt#/prog_line = constant.value
    if ((lhsEntity.isProgLineSynonym() || 
      lhsEntity.getAttributeRefType() == AttributeRefType::STMT_NUMBER) && 
      rhsEntity.getAttributeRefType() == AttributeRefType::VALUE) {
      Table& lhsTable = getTableFromEntity(lhsEntity);
      Table finalTable(2);
      for (Row row : lhsTable.getData()) {
        const int intRef = pkb.getIntRefFromEntity(std::to_string(row[0]));
        if (intRef != -1) {
          finalTable.insertRow({ row[0], intRef });
        }
      }
      finalTable.setHeader({ lhsEntity.getValue(), rhsEntity.getValue() });
      clauseResultTable = std::move(finalTable);
      return;
    } else if ((rhsEntity.isProgLineSynonym() || 
      rhsEntity.getAttributeRefType() == AttributeRefType::STMT_NUMBER) && 
      lhsEntity.getAttributeRefType() == AttributeRefType::VALUE) {
      Table& rhsTable = getTableFromEntity(rhsEntity);
      Table finalTable(2);
      for (Row row : rhsTable.getData()) {
        const int intRef = pkb.getIntRefFromEntity(std::to_string(row[0]));
        if (intRef != -1) {
          finalTable.insertRow({ row[0], intRef });
        }
      }
      finalTable.setHeader({ rhsEntity.getValue(), lhsEntity.getValue() });
      clauseResultTable = std::move(finalTable);
      return;
    }

    // LHS and RHS both not constants - Other case
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

    // LHS is number or name - Guaranteed by the PqlParser
    const bool isLhsNameOrNum = lhsEntity.isName() || lhsEntity.isNumber();
    const Entity& synonymEntity = isLhsNameOrNum ? rhsEntity : lhsEntity;
    const Entity& constantEntity = isLhsNameOrNum ? lhsEntity : rhsEntity;

    const bool isReferenceToStmtNumber = synonymEntity.getAttributeRefType() == AttributeRefType::STMT_NUMBER ||
      synonymEntity.getType() == EntityType::PROG_LINE;
    const int filterValue = isReferenceToStmtNumber
      ? std::stoi(constantEntity.getValue())
      : pkb.getIntRefFromEntity(constantEntity.getValue());

    if (needsAttrRefMapping(synonymEntity)) {
      clauseResultTable = std::move(getAttrRefMappingTableFromEntity(synonymEntity)); // Two column table
      clauseResultTable.filterColumn(1, { filterValue });
      clauseResultTable.setHeader({ synonymEntity.getValue(), "" });
    } else {
      clauseResultTable = std::move(getTableFromEntity(synonymEntity)); // One column table
      clauseResultTable.filterColumn(0, { filterValue });
      clauseResultTable.setHeader({ synonymEntity.getValue() });
    }
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
    default:
      assert(false);
      return Table();
    }
  }

  Table PqlEvaluator::getAttrRefMappingTableFromEntity(const Entity& entity) const {
    assert(needsAttrRefMapping(entity));
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
      for (size_t i = 0; i < columns.size(); i++) {
        const int rowColumnIdx = columns[i];
        outputLine
          .append(mapIntRefToResultValue(queryTargets[i], row[rowColumnIdx]))
          .append(" ");
      }
      outputLine.pop_back();

      if (set.count(outputLine) == 0) {
        set.emplace(outputLine);
        results.emplace_back(outputLine);
      }
    }
  }

  std::string PqlEvaluator::mapIntRefToResultValue(const Pql::Entity& entity, const int intRef) const {
    if (needsAttrRefMapping(entity)) {
      switch (entity.getType()) {
      case Pql::EntityType::CALL:
        return pkb.getProcNameFromCallStmt(intRef);
      case Pql::EntityType::READ:
        return pkb.getVarNameFromReadStmt(intRef);
      case Pql::EntityType::PRINT:
        return pkb.getVarNameFromPrintStmt(intRef);
      default:
        assert(false);
        return "";
      }
    }

    switch (entity.getType()) {
    case EntityType::PROG_LINE:
    case EntityType::STMT:
    case EntityType::READ:
    case EntityType::PRINT:
    case EntityType::CALL:
    case EntityType::WHILE:
    case EntityType::IF:
    case EntityType::ASSIGN:
      return std::to_string(intRef);

    case EntityType::VARIABLE:
    case EntityType::CONSTANT:
    case EntityType::PROCEDURE:
      return pkb.getEntityFromIntRef(intRef);

    default:
      assert(false);
      return "";
    }
  }
}
