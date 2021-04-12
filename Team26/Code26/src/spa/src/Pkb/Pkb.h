#pragma once

#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Cfg.h"
#include "Table.h"

class Pkb {
private:
  Cfg::Cfg cfg;

  Table varTable{ 1 };
  Table stmtTable{ 1 };
  Table procTable{ 1 };
  Table constTable{ 1 };

  Table ifTable{ 1 };
  Table whileTable{ 1 };
  Table readTable{ 1 };
  Table printTable{ 1 };
  Table assignTable{ 1 };
  Table callTable{ 1 };

  Table followsTable{ 2 };
  Table followsTTable{ 2 };
  Table parentTable{ 2 };
  Table parentTTable{ 2 };
  Table usesSTable{ 2 };
  Table usesPTable{ 2 };
  Table modifiesSTable{ 2 };
  Table modifiesPTable{ 2 };
  Table callsTable{ 2 };
  Table callsTTable{ 2 };
  Table nextTable{ 2 };
  Table nextTTable{ 2 };
  Table affectsTable{ 2 };
  Table affectsTTable{ 2 };

  Table nextBipTable{ 2 };
  Table nextBipTTable{ 2 };
  Table affectsBipTable{ 2 };
  Table affectsBipTTable{ 2 };

  Table callProcTable{ 2 };
  Table readVarTable{ 2 };
  Table printVarTable{ 2 };

  Table patternAssignTable{ 3 };
  Table patternIfTable{ 2 };
  Table patternWhileTable{ 2 };

  std::unordered_set<int> varIntRefs;
  std::unordered_set<int> stmtIntRefs;
  std::unordered_set<int> procIntRefs;
  std::unordered_set<int> constIntRefs;

  std::unordered_set<int> ifIntRefs;
  std::unordered_set<int> whileIntRefs;
  std::unordered_set<int> readIntRefs;
  std::unordered_set<int> printIntRefs;
  std::unordered_set<int> assignIntRefs;
  std::unordered_set<int> callIntRefs;

  std::unordered_map<int, std::string> intRefToEntityMapper;
  std::unordered_map<std::string, int> entityToIntRefMapper;

  std::unordered_map<int, std::string> callIntRefToProcMapper;
  std::unordered_map<int, std::string> readIntRefToVarMapper;
  std::unordered_map<int, std::string> printIntRefToVarMapper;

  std::unordered_map<std::string, int> procStartMapper;
  std::unordered_map<std::string, std::vector<int>> procEndMapper;
  std::unordered_map<int, std::string> stmtProcMapper;

public:
  /*
   * Constructor of Pkb.
   */
  Pkb();

  /**
   * Initialises the CFGBip.
   * 
   */
  void initialiseCfgBip(const std::list<std::string>& topoProc);

  /**
   * Adds the range of statement numbers that belong to a procedure.
   *
   * @param proc Procedure in question.
   * @param first Statement number of the first statement in the procedure.
   * @param last Last statement number in the procedure. Different from end statement of a procedure.
   */
  void addProcRange(const std::string proc, const int first, const int last);

  /**
   * Adds a procedure's first and last statement numbers in the control flow path.
   *
   * @param proc Procedure in question.
   * @param start Statement number of the first statement in the procedure's control flow path.
   * @param end List of statement numbers of the last statements in the procedure's control flow path.
   */
  void addProcStartEnd(const std::string proc, const int start, const std::vector<int> end);

  /**
   * Adds a directed edge into the CFG and CFGBip. Also populates Next and NextBip relation.
   *
   * @param from Statement number of the statement executed first.
   * @param to Statement number of the statement which can be executed immediately after.
   */
  void addCfgEdge(const int from, const int to);

  /**
   * Adds a variable name into varTable.
   *
   * @param var Variable name to be inserted
   */
  void addVar(const std::string& var);

  /**
   * Adds a statement number into stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addStmt(const int stmtNum);

  /**
   * Adds a procedure name into procTable.
   *
   * @param proc Procedure name to be inserted
   */
  void addProc(const std::string& proc);

  /**
   * Adds a constant into constTable.
   *
   * @param constant Constant String to be inserted
   */
  void addConst(const std::string& constant);

  /**
   * Adds stmtNum into ifTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addIf(const int stmtNum);

  /**
   * Adds stmtNum into whileTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addWhile(const int stmtNum);

  /**
   * Adds stmtNum into readTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addRead(const int stmtNum);

  /**
   * Adds stmtNum into printTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addPrint(const int stmtNum);

  /**
   * Adds stmtNum into assignTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addAssign(const int stmtNum);

  /**
   * Adds stmtNum into callTable and stmtTable.
   *
   * @param stmtNum Statement number to be inserted
   */
  void addCall(const int stmtNum);

  /**
   * Adds the Row {followed, follower} into followsTable.
   *
   * @param followed Statement number of the preceding statement
   * @param follower Statement number of the following statement
   */
  void addFollows(const int followed, const int follower);

  /**
   * Adds the Row {followed, follower} into followsTTable.
   *
   * @param followed Statement number of the preceding statement
   * @param follower Statement number of the following statement
   */
  void addFollowsT(const int followed, const int follower);

  /**
   * Adds the Row {parent, child} into parentTable.
   *
   * @param parent Statement number of the container statement
   * @param child Statement number of the child statement
   */
  void addParent(const int parent, const int child);

  /**
   * Adds the Row {parent, child} into parentTTable.
   *
   * @param parent Statement number of the container statement
   * @param child Statement number of the child statement
   */
  void addParentT(const int parent, const int child);

  /**
   * Adds the Row {stmtNum, var} into usesSTable.
   *
   * @param stmtNum Statement number of the statement which uses var
   * @param var Variable or constant name which is used by the statement
   */
  void addUsesS(const int stmtNum, const std::string& var);

  /**
   * Adds the Row {proc, var} into usesPTable.
   *
   * @param proc Name of procedure which uses var
   * @param var Variable or constant name which is used by the procedure
   */
  void addUsesP(const std::string& proc, const std::string& var);

  /**
   * Adds the Row {stmtNum, var} into modifiesSTable.
   *
   * @param stmtNum Statement number of the statement which modifies var
   * @param var Variable or constant name which is modified by the statement
   */
  void addModifiesS(const int stmtNum, const std::string& var);

  /**
   * Adds the Row {proc, var} into modifiesPTable.
   *
   * @param proc Name of procedure which modifies var
   * @param var Variable or constant name which is modified by the procedure
   */
  void addModifiesP(const std::string& proc, const std::string& var);

  /**
   * Adds the Row {caller, called} into callsTable.
   *
   * @param followed Name of the caller Procedure
   * @param follower Name of the Procedure being called
   */
  void addCalls(const std::string& caller, const std::string& called);

  /**
   * Adds the Row {caller, called} into callsTTable.
   *
   * @param followed Name of the transitive caller Procedure
   * @param follower Name of the Procedure being transitively called
   */
  void addCallsT(const std::string& caller, const std::string& called);

  /**
   * Adds the Row {prev, next} into nextTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed immediately after prev in some execution sequence
   */
  void addNext(const int prev, const int next);

  /**
   * Adds the Row {prev, next} into nextTTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed after prev in some execution sequence
   */
  void addNextT(const int prev, const int next);

  /**
   * Adds the Row {affecter, affected} into affectsTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffects(const int affecter, const int affected);

  /**
   * Adds the Row {affecter, affected} into affectsTTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffectsT(const int affecter, const int affected);

  /**
   * Adds the Row {prev, next} into nextBipTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed immediately after prev in some execution sequence
   */
  void addNextBip(const int prev, const int next);

  /**
   * Adds the Row {prev, next} into nextBipTTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed after prev in some execution sequence
   */
  void addNextBipT(const int prev, const int next);

  /**
   * Adds the Row {affecter, affected} into affectsBipTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffectsBip(const int affecter, const int affected);

  /**
   * Adds the Row {affecter, affected} into affectsBipTTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffectsBipT(const int affecter, const int affected);

  /**
   * Adds the Row {stmtNum, lhs, rhs} into patternAssignTable.
   *
   * @param stmtNum Statement number of the assign statement
   * @param lhs String of the left hand side of the assign statement
   * @param rhs String of the postfix form of the right hand side of the
   * assign statement
   */
  void addPatternAssign(const int stmtNum, const std::string& lhs, const std::string& rhs);

  /**
   * Adds the Row {stmtNum, var} int patternIfTable.
   *
   * @param stmtNum Statement number of the if statement
   * @param var Name of variable within the condition of the if statement
   */
  void addPatternIf(const int stmtNum, const std::string& var);

  /**
   * Adds the Row {stmtNum, var} into patternWhileTable.
   *
   * @param stmtNum Statement number of the while statement
   * @param var Name of variable within the condition of the while statement
   */
  void addPatternWhile(const int stmtNum, const std::string& var);

  /**
  * Adds the Row {stmtNum, proc} into callProcTable.
  *
  * @param stmtNum Statement number of the call statement
  * @param var Name of procedure being called by stmtNum
  */
  void addCallProc(const int stmtNum, const std::string& proc);

  /**
  * Adds the Row {stmtNum, var} into readVarTable.
  *
  * @param stmtNum Statement number of the read statement
  * @param var Name of variable being read
  */
  void addReadVar(const int stmtNum, const std::string& var);

  /**
  * Adds the Row {stmtNum, var} into printVarTable.
  *
  * @param stmtNum Statement number of the print statement
  * @param var Name of variable being printed
  */
  void addPrintVar(const int stmtNum, const std::string& var);

  /**
   * @return varTable
   */
  Table getVarTable() const;

  /**
   * @return stmtTable
   */
  Table getStmtTable() const;

  /**
   * @return procTable
   */
  Table getProcTable() const;

  /**
   * @return constTable
   */
  Table getConstTable() const;

  /**
   * @return ifTable
   */
  Table getIfTable() const;

  /**
   * @return whileTable
   */
  Table getWhileTable() const;

  /**
   * @return readTable
   */
  Table getReadTable() const;

  /**
   * @return printTable
   */
  Table getPrintTable() const;

  /**
   * @return assignTable
   */
  Table getAssignTable() const;

  /**
   * @return followsTable
   */
  Table getFollowsTable() const;

  /**
   * @return followsTTable
   */
  Table getFollowsTTable() const;

  /**
   * @return parentTable
   */
  Table getParentTable() const;

  /**
   * @return parentTTable
   */
  Table getParentTTable() const;

  /**
   * @return usesSTable
   */
  Table getUsesSTable() const;

  /**
   * @return modifiesSTable
   */
  Table getModifiesSTable() const;

  /**
   * @return patternAssignTable
   */
  Table getPatternAssignTable() const;

  /**
   * @return callTable
   */
  Table getCallTable() const;

  /**
   * @return callsTable
   */
  Table getCallsTable() const;

  /**
   * @return callsTTable
   */
  Table getCallsTTable() const;

  /**
   * @return nextTable
   */
  Table getNextTable() const;

  /**
   * @return nextTTable
   */
  Table getNextTTable() const;

  /**
   * @return affectsTable
   */
  Table getAffectsTable() const;

  /**
   * @return affectsTTable
   */
  Table getAffectsTTable() const;

  /**
 * @return nextBipTable
 */
  Table getNextBipTable() const;

  /**
   * @return nextBipTTable
   */
  Table getNextBipTTable() const;

  /**
   * @return affectsBipTable
   */
  Table getAffectsBipTable() const;

  /**
   * @return affectsBipTTable
   */
  Table getAffectsBipTTable() const;

  /**
   * @return usesPTable
   */
  Table getUsesPTable() const;

  /**
   * @return modifiesPTable
   */
  Table getModifiesPTable() const;

  /**
   * @return callProcTable
   */
  Table getCallProcTable() const;

  /**
   * @return readVarTable
   */
  Table getReadVarTable() const;

  /**
   * @return printVarTable
   */
  Table getPrintVarTable() const;

  /**
   * @return patternIfTable
   */
  Table getPatternIfTable() const;

  /**
   * @return patternWhileTable
   */
  Table getPatternWhileTable() const;

  /**
   * @return varIntRefs
   */
  std::unordered_set<int> getVarIntRefs() const;

  /**
   * @return stmtIntRefs
   */
  std::unordered_set<int> getStmtIntRefs() const;

  /**
   * @return procIntRefs
   */
  std::unordered_set<int> getProcIntRefs() const;

  /**
   * @return constIntRefs
   */
  std::unordered_set<int> getConstIntRefs() const;

  /**
   * @return ifIntRefs
   */
  std::unordered_set<int> getIfIntRefs() const;

  /**
   * @return whileIntRefs
   */
  std::unordered_set<int> getWhileIntRefs() const;

  /**
   * @return assignIntRefs
   */
  std::unordered_set<int> getReadIntRefs() const;

  /**
   * @return assignIntRefs
   */
  std::unordered_set<int> getPrintIntRefs() const;

  /**
   * @return assignIntRefs
   */
  std::unordered_set<int> getAssignIntRefs() const;

  /**
   * @return callIntRefs
   */
  std::unordered_set<int> getCallIntRefs() const;

  /**
   * Returns the integer reference of a given entity.
   * Returns -1 if entity does not exist.
   *
   * @param entity Entity.
   * @return Integer reference of the entity.
   */
  int getIntRefFromEntity(const std::string& entity) const;

  /**
   * Returns the entity of the given existing integer reference.
   *
   * @param intRef Entity integer reference.
   * @return Entity of the integer reference.
   */
  std::string getEntityFromIntRef(const int intRef) const;

  /**
   * Returns the integer reference of a given statement number.
   * Returns -1 if statement number does not exist.
   * Equivalent to getIntRefFromEntity(std::to_string(stmtNum))
   *
   * @param stmtNum Statement number.
   * @return Integer reference of the statement number.
   */
  int getIntRefFromStmtNum(const int stmtNum) const;

  /**
   * Returns the stmt of the given existing integer reference.
   * Equivalent to std::stoi(getEntityFromIntRef(intRef))
   * 
   * @param intRef Statement number integer reference.
   * @return Statement number of the integer reference.
   */
  int getStmtNumFromIntRef(const int intRef) const;

  /**
   * Finds and returns the assign statement numbers that are uses the given variable. If no
   * assign statement uses the variable, returns an empty result. This function must be
   * called after the Design Extractor extracts all the design abstractions.
   *
   * @param varName Variable name to check.
   * @return Set of assign statement numbers that uses the variable.
   */
  std::unordered_set<int> getAssignUses(const std::string& varName) const;

  /**
   * Finds and returns the variables that are modified by the given statement number. If no
   * variable is modified, returns an empty result. This function must be called after the
   * Design Extractor extracts all the design abstractions.
   *
   * @param stmtNum Statement number to check.
   * @return Set of variables modified.
   */
  std::unordered_set<std::string> getModifiedBy(const int stmtNum) const;

  /**
   * Get the procedure name called by the given existing call statement number.
   *
   * @param stmtNum Statement number of a call statement.
   * @return Name of procedure being called by stmtNum.
   */
  std::string getProcNameFromCallStmt(const int stmtNum) const;

  /**
   * Get the procedure name called by the given existing call statement number's integer reference.
   *
   * @param intRef Integer reference of a call statement.
   * @return Name of procedure being called by the call statement.
   */
  std::string getProcNameFromCallStmtIntRef(const int intRef) const;

  /**
   * Get the variable name read by the given existing read statement number.
   *
   * @param stmtNum Statement number of a read statement.
   * @return Name of variable being read by stmtNum.
   */
  std::string getVarNameFromReadStmt(const int stmtNum) const;

  /**
   * Get the variable name read by the given existing read statement number's integer reference.
   *
   * @param intRef Integer reference of a read statement
   * @return Name of variable being read by the read statement.
   */
  std::string getVarNameFromReadStmtIntRef(const int intRef) const;

  /**
   * Get the variable name printed by the given existing print statement number.
   *
   * @param stmtNum Statement number of a print statement
   * @return Name of variable being printed by stmtNum
   */
  std::string getVarNameFromPrintStmt(const int stmtNum) const;

  /**
   * Get the variable name printed by the given existing print statement number's integer reference.
   *
   * @param intRef Integer reference of a print statement
   * @return Name of variable being printed by the print statement.
   */
  std::string getVarNameFromPrintStmtIntRef(const int intRef) const;

  /**
   * Finds the set of stmts that can be directly executed after the given stmt number in the CFG.
   * If there are no nodes following, an empty set is returned.
   *
   * @param stmtNum Statement number of interest.
   * @return std::vector<int> List of nodes.
   */
  std::vector<int> getNextStmtsFromCfg(const int stmtNum) const;

  /**
   * Finds the first statement of a given procedure.
   *
   * @param proc Procedure in question.
   * @return int Statement number of the first statement in the procedure.
   */
  int getStartStmtFromProc(const std::string& proc) const;

  /**
   * Finds the list of last statements of a given procedure.
   *
   * @param proc Procedure in question.
   * @return std::vector<int> List of statement number of the last statements in the procedure.
   */
  std::vector<int> getEndStmtsFromProc(const std::string& proc) const;

  /**
   * Finds the procedure corresponding to the target statement number.
   *
   * @param stmt Statement in question.
   * @return std::string Procedure that the statement belongs to.
   */
  std::string getProcFromStmt(const int stmt) const;

  /**
   * Gets the list of pointers to the startNodes of relevant topo-sorted procedures in the CFGBip.
   *
   * @return std::vector<std::shared_ptr<Cfg::BipNode>> List of pointers to the startNodes of relevant topo-sorted procedures in the CFGBip.
   */
  std::vector<std::shared_ptr<Cfg::BipNode>> getStartBipNodes() const;

private:
  /**
   * Adds the given entity to the PKB if not yet added and returns the integer reference of the entity.
   * 
   * @param entity Entity to be added.
   * @return Integer reference of the entity. 
   */
  int addEntity(const std::string& entity);
};
