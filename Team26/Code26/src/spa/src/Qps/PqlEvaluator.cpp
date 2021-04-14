#include "PqlEvaluator.h"

#include <assert.h>

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Pkb.h"
#include "PqlOptimizer.h"
#include "PqlPreprocessor.h"
#include "PqlQuery.h"
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

  /**
   * Helper function to get procedure name from call stmt's integer reference with
   * the given PKB and integer reference.
   *
   * @param pkb PKB.
   * @param intRef Integer Reference.
   * @return Procedure name.
  */
  std::string getProcNameFromCallStmtIntRef(const Pkb& pkb, const int intRef) {
    return pkb.getProcNameFromCallStmtIntRef(intRef);
  }

  /**
   * Helper function to get variable name from read stmt's integer reference with
   * the given PKB and integer reference.
   *
   * @param pkb PKB.
   * @param intRef Integer Reference.
   * @return Variable name.
  */
  std::string getVarNameFromReadStmtIntRef(const Pkb& pkb, const int intRef) {
    return pkb.getVarNameFromReadStmtIntRef(intRef);
  }

  /**
   * Helper function to get variable name from print stmt's integer reference with
   * the given PKB and integer reference.
   *
   * @param pkb PKB.
   * @param intRef Integer Reference.
   * @return Variable name.
  */
  std::string getVarNameFromPrintStmtIntRef(const Pkb& pkb, const int intRef) {
    return pkb.getVarNameFromPrintStmtIntRef(intRef);
  }

  /**
   * Helper function to get entity value from its integer reference with
   * the given PKB and integer reference.
   *
   * @param pkb PKB.
   * @param intRef Integer Reference.
   * @return Entity value.
  */
  std::string getEntityFromIntRef(const Pkb& pkb, const int intRef) {
    return pkb.getEntityFromIntRef(intRef);
  }

  /**
   * Helper function to get the mapping function for mapping the table element to the select target result.
   *
   * @param entity Given entity to be mapped.
   * @return Function that will be used for mapping the table element to the select target result.
   */
  std::string(*getMappingFunction(const Pql::Entity& entity)) (const Pkb&, const int) {
    if (needsAttrRefMapping(entity)) {
      switch (entity.getType()) {
      case Pql::EntityType::CALL:
        return getProcNameFromCallStmtIntRef;
      case Pql::EntityType::READ:
        return getVarNameFromReadStmtIntRef;
      case Pql::EntityType::PRINT:
        return getVarNameFromPrintStmtIntRef;
      default:
        assert(false);
        return nullptr;
      }
    }
    return getEntityFromIntRef;
  }
}

namespace Pql {
  // Constructor
  PqlEvaluator::PqlEvaluator(Pkb& pkb, Query& query, std::list<std::string>& results)
    : clauses(query.getClauses()), targets(query.getTargets()), pkb(pkb), isQueryBoolean(query.isBoolean()), results(results) {
    for (const Entity& target : targets) {
      targetSynonymsSet.emplace(target.getValue());
    }
  }

  // Executes query and extract results
  void PqlEvaluator::evaluateQuery() {
    PqlPreprocessor preprocessor(clauses, targets);
    ClauseGroups clauseGroups = preprocessor.generateClauseGroups();

    if (!executeNoSynonymClauses(clauseGroups.withoutSynonyms)) {
      extractResults(Table()); // no results
      return;
    }

    if (!executeDisconnectedClauses(clauseGroups.disconnected)) {
      extractResults(Table()); // no results
      return;
    }

    extractResults(executeConnectedClauses(clauseGroups.connected, clauseGroups.unusedTargets));
  }

  bool PqlEvaluator::executeNoSynonymClauses(const std::unordered_set<int>& clauseIdxs) const {
    for (const int idx : clauseIdxs) {
      const Clause& clause = clauses[idx];
      const Table& clauseResult = executeClause(clause);
      if (clauseResult.empty()) {
        // Short circuit
        return false;
      }
    }
    return true;
  }

  bool PqlEvaluator::executeDisconnectedClauses(const std::vector< std::unordered_set<int>>& clauseGroupsIdxs) const {
    for (const std::unordered_set<int>& idxs : clauseGroupsIdxs) {
      std::vector<Table> clauseResultTables;
      clauseResultTables.reserve(idxs.size());
      for (const int idx : idxs) {
        const Clause& clause = clauses[idx];
        Table& clauseResult = executeClause(clause);
        if (clauseResult.empty()) {
          // Short circuit
          return false;
        }
        bool areAllHeadersEmptyStrings = true;
        for (const std::string& header : clauseResult.getHeader()) {
          if (!header.empty()) {
            areAllHeadersEmptyStrings = false;
          }
        }
        if (!areAllHeadersEmptyStrings) { // Don't need to add to group if all headers empty strings
          clauseResult.dropColumn(""); // drop empty column - Guaranteed to be only 1 column max
          clauseResultTables.emplace_back(std::move(clauseResult));
        }
      }

      Optimizer optimizer(clauseResultTables);
      const std::vector<int>& order = optimizer.getOptimizedOrder();

      const int firstIdx = order[0];
      Table& groupResultTable = clauseResultTables[firstIdx];

      // Join each clause result table to groupResultTable
      for (size_t i = 1; i < clauseResultTables.size(); i++) {
        const int tableIndex = order[i];
        groupResultTable.naturalJoin(clauseResultTables[tableIndex]);
      }

      if (groupResultTable.empty()) {
        return false;
      }
    }

    // Non of the groupResultTable is empty
    return true;
  }

  Table PqlEvaluator::executeConnectedClauses(
    const std::vector< std::unordered_set<int>>& clauseGroupsIdxs,
    const std::unordered_set<Entity>& unusedTargets) const {

    std::vector<Table> groupResultTables; // Stores all tables to be cross joined
    groupResultTables.reserve(clauseGroupsIdxs.size() + unusedTargets.size());

    for (const std::unordered_set<int>& idxs : clauseGroupsIdxs) {
      std::vector<Table> clauseResultTables;
      clauseResultTables.reserve(idxs.size());
      for (const int idx : idxs) {
        const Clause& clause = clauses[idx];
        Table& clauseResult = executeClause(clause);
        if (clauseResult.empty()) {
          // short circuit
          return Table();
        }
        bool areAllHeadersEmptyStrings = true;
        for (const std::string& header : clauseResult.getHeader()) {
          if (!header.empty()) {
            areAllHeadersEmptyStrings = false;
          }
        }
        if (!areAllHeadersEmptyStrings) { // Don't need to add to group if all headers empty strings
          clauseResult.dropColumn(""); // drop empty column - Guaranteed to be only 1 column max
          clauseResultTables.emplace_back(std::move(clauseResult));
        }
      }

      // Join all clauseResultTables to get groupResultTable
      Optimizer optimizer(clauseResultTables);
      const std::vector<int>& order = optimizer.getOptimizedOrder();

      const int firstIdx = order[0];
      Table& groupResultTable = clauseResultTables[firstIdx];

      // Join each clause result table to groupResultTable
      for (size_t i = 1; i < clauseResultTables.size(); i++) {
        const int tableIndex = order[i];
        groupResultTable.naturalJoin(clauseResultTables[tableIndex]);
      }

      // Remove non-targeted columns for each groupResultTable before adding to groupResultTables
      groupResultTable.filterHeaders(targetSynonymsSet);
      groupResultTables.emplace_back(std::move(groupResultTable));
    }

    // Add remaining unusedTargets enitiy tables groupResultTables
    std::unordered_set<std::string> addedTargetSynonyms;
    addedTargetSynonyms.reserve(unusedTargets.size());
    for (const Entity& target : unusedTargets) {
      const std::string& synonymName = target.getValue();
      if (addedTargetSynonyms.count(synonymName) == 0) {
        Table& entityTable = getTableFromEntity(target);
        entityTable.setHeader({ synonymName });
        groupResultTables.emplace_back(std::move(entityTable));
        addedTargetSynonyms.emplace(synonymName);
      }
    }

    // Empty groupResultTables signifies TRUE
    if (groupResultTables.empty()) {
      Table finalResultTable = Table({ "" });
      finalResultTable.insertRow({ 0 }); // dummy row
      return finalResultTable;
    }

    // Join each groupResultTable to finalResultTable
    Optimizer optimizer(groupResultTables);
    const std::vector<int>& order = optimizer.getOptimizedOrder();
    const int firstIdx = order[0];
    Table& finalResultTable = groupResultTables[firstIdx];
    for (size_t i = 1; i < groupResultTables.size(); i++) {
      const int tableIndex = order[i];
      finalResultTable.crossJoin(groupResultTables[tableIndex]); // Guaranteed to be cross join
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
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::FOLLOWS_T:
      clauseResultTable = pkb.getFollowsTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PARENT:
      clauseResultTable = pkb.getParentTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::PARENT_T:
      clauseResultTable = pkb.getParentTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::USES_S:
      clauseResultTable = pkb.getUsesSTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::USES_P:
      clauseResultTable = pkb.getUsesPTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::MODIFIES_S:
      clauseResultTable = pkb.getModifiesSTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::MODIFIES_P:
      clauseResultTable = pkb.getModifiesPTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::CALLS:
      clauseResultTable = pkb.getCallsTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::CALLS_T:
      clauseResultTable = pkb.getCallsTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT:
      clauseResultTable = pkb.getNextTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT_T:
      clauseResultTable = pkb.getNextTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS:
      clauseResultTable = pkb.getAffectsTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS_T:
      clauseResultTable = pkb.getAffectsTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT_BIP:
      clauseResultTable = pkb.getNextBipTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::NEXT_BIP_T:
      clauseResultTable = pkb.getNextBipTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS_BIP:
      clauseResultTable = pkb.getAffectsBipTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
      break;
    case ClauseType::AFFECTS_BIP_T:
      clauseResultTable = pkb.getAffectsBipTTable();
      constructSuchThatTableFromClause(clauseResultTable, clause);
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
  void PqlEvaluator::constructSuchThatTableFromClause(Table& clauseResultTable, const Clause& clause) const {
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
        if (!canOmitJoinSuchThatLhs(clause.getType(), lhsEntity)) { // Shortcircuit
          clauseResultTable.filterColumn(0, getValuesFromEntity(lhsEntity));
        }
      } else if (lhsEntity.isName() || lhsEntity.isNumber()) {
        clauseResultTable.filterColumn(0, { pkb.getIntRefFromEntity(lhsEntity.getValue()) });
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
        if (!canOmitJoinSuchThatRhs(clause.getType(), rhsEntity)) { // Shortcircuit
          clauseResultTable.filterColumn(1, getValuesFromEntity(rhsEntity));
        }
      } else if (rhsEntity.isName() || rhsEntity.isNumber()) {
        clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(rhsEntity.getValue()) });
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

    const std::string header1 = synonymEntity.getValue();
    std::string header2 = ""; // only second header can have different possible values

    if (lhsEntity.isSynonym()) { // Guaranteed to be of type VARIABLE
      header2 = lhsEntity.getValue();
      clauseResultTable.filterColumn(1, getValuesFromEntity(lhsEntity));
    } else if (lhsEntity.isName()) {
      clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(lhsEntity.getValue()) });
    }
    // else wildcard. do not join with any tables. 

    std::string postfixExpr = rhsEntity.getValue();
    if (rhsEntity.isExpression()) {
      clauseResultTable.filterColumn(2, { pkb.getIntRefFromEntity(postfixExpr) });
    } else if (rhsEntity.isSubExpression()) {
      // Manual filtering for sub expressions
      for (const Row& row : clauseResultTable.getData()) {
        const bool doesNotMatch = pkb.getEntityFromIntRef(row[2]).find(postfixExpr) == std::string::npos;
        if (doesNotMatch) {
          clauseResultTable.deleteRow(row);
        }
      }
    }
    // else wildcard. do not join with any tables. 

    clauseResultTable.dropColumn(2); // drop third column
    clauseResultTable.setHeader({ header1, header2 });
  }

  void PqlEvaluator::constructPatternCondTableFromClause(Table& clauseResultTable, const Clause& clause) const {
    const std::vector<Entity>& params = clause.getParams();

    const Entity& synonymEntity = params[0];
    const Entity& condEntity = params[1];

    const std::string header1 = synonymEntity.getValue();
    std::string header2 = ""; // only second header can have different possible values

    if (condEntity.isSynonym()) { // Guaranteed to be of type VARIABLE
      header2 = condEntity.getValue();
      clauseResultTable.filterColumn(1, getValuesFromEntity(condEntity));
    } else if (condEntity.isName()) {
      clauseResultTable.filterColumn(1, { pkb.getIntRefFromEntity(condEntity.getValue()) });
    }
    // else wildcard. do not join with any tables. 

    clauseResultTable.setHeader({ header1, header2 });
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

    // LHS and RHS both not constants
    int lhsColumnIdxToJoin = -1;
    if ((lhsEntity.isProgLineSynonym() || lhsEntity.isAttributeRef()) &&
      (rhsEntity.isProgLineSynonym() || rhsEntity.isAttributeRef())) {
      if (needsAttrRefMapping(lhsEntity)) {
        clauseResultTable = std::move(getAttrRefMappingTableFromEntity(lhsEntity)); // Two column
        clauseResultTable.setHeader({ lhsEntity.getValue(), "" });
        lhsColumnIdxToJoin = 1;
      } else {
        clauseResultTable = std::move(getTableFromEntity(lhsEntity));
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
    const std::unordered_set<int> filterValueSet{ pkb.getIntRefFromEntity(constantEntity.getValue()) };
    const std::string& synonymName = synonymEntity.getValue();
    if (needsAttrRefMapping(synonymEntity)) {
      clauseResultTable = std::move(getAttrRefMappingTableFromEntity(synonymEntity)); // Two column table
      clauseResultTable.filterColumn(1, filterValueSet);
      clauseResultTable.setHeader({ synonymName, "" });
    } else {
      clauseResultTable = std::move(getTableFromEntity(synonymEntity)); // One column table
      clauseResultTable.filterColumn(0, filterValueSet);
      clauseResultTable.setHeader({ synonymName });
    }
  }

  // Get table from given synonym entity 
  Table PqlEvaluator::getTableFromEntity(const Entity& synonymEntity) const {
    switch (synonymEntity.getType()) {
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
    default:
      assert(false);
      return Table();
    }
  }

  std::unordered_set<int> PqlEvaluator::getValuesFromEntity(const Entity& synonymEntity) const {
    switch (synonymEntity.getType()) {
    case EntityType::PROG_LINE:
    case EntityType::STMT:
      return pkb.getStmtIntRefs();
    case EntityType::READ:
      return pkb.getReadIntRefs();
    case EntityType::PRINT:
      return pkb.getPrintIntRefs();
    case EntityType::CALL:
      return pkb.getCallIntRefs();
    case EntityType::WHILE:
      return pkb.getWhileIntRefs();
    case EntityType::IF:
      return pkb.getIfIntRefs();
    case EntityType::ASSIGN:
      return pkb.getAssignIntRefs();
    case EntityType::VARIABLE:
      return pkb.getVarIntRefs();
    case EntityType::CONSTANT:
      return pkb.getConstIntRefs();
    case EntityType::PROCEDURE:
      return pkb.getProcIntRefs();
    default:
      assert(false);
      return {};
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

  void PqlEvaluator::extractResults(const Table& resultTable) const {
    // ===============
    // BOOLEAN Select
    // ===============
    if (isQueryBoolean) {
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

    const int numTargets = targets.size();

    // Header -> Column index mapping
    std::unordered_map<std::string, int> headerToColIdxMapping;
    std::vector<std::string>& headers = resultTable.getHeader();
    for (size_t i = 0; i < headers.size(); i++) {
      headerToColIdxMapping[headers[i]] = i;
    }

    // Query target index -> Table Column index mapping
    std::vector<int> targetToTableColIdxMapping;
    targetToTableColIdxMapping.reserve(numTargets);

    // Query target index -> mapping function
    std::vector<std::string(*)(const Pkb&, const int)> targetToFunctionMapping;
    targetToFunctionMapping.reserve(numTargets);

    for (const Entity& target : targets) {
      targetToTableColIdxMapping.emplace_back(headerToColIdxMapping[target.getValue()]);
      targetToFunctionMapping.emplace_back(getMappingFunction(target));
    }

    // Insert results
    std::unordered_set<std::string> set; // for checking of repeated elements
    set.reserve(resultTable.size()); // optimization to avoid rehashing
    for (const Row& row : resultTable.getData()) {
      std::string outputLine;
      for (int i = 0; i < numTargets; i++) {
        const int tableColIdx = targetToTableColIdxMapping[i];
        std::string(*mappingFunction)(const Pkb&, const int) = targetToFunctionMapping[i];
        outputLine
          .append(mappingFunction(pkb, row[tableColIdx]))
          .append(" ");
      }
      outputLine.pop_back();

      if (set.count(outputLine) == 0) {
        set.emplace(outputLine);
        results.emplace_back(outputLine);
      }
    }
  }
}
