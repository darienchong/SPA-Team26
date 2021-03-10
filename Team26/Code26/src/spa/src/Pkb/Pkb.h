#pragma once

#include "Table.h"

#include <map>

class Pkb {

public:
  /*
   * Constructor of Pkb.
   */
  Pkb();

  /**
   * Adds a variable name into varTable.
   *
   * @param var Variable name to be inserted
   */
  void addVar(std::string var);

  /**
   * Adds a statement number into stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addStmt(int stmtNo);

  /**
   * Adds a procedure name into procTable.
   *
   * @param proc Procedure name to be inserted
   */
  void addProc(std::string proc);

  /**
   * Adds a constant into constTable.
   *
   * @param constant Constant String to be inserted
   */
  void addConst(std::string constant);

  /**
   * Adds stmtNo into ifTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addIf(int stmtNo);

  /**
   * Adds stmtNo into whileTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addWhile(int stmtNo);

  /**
   * Adds stmtNo into readTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addRead(int stmtNo);

  /**
   * Adds stmtNo into printTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addPrint(int stmtNo);

  /**
   * Adds stmtNo into assignTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addAssign(int stmtNo);

  /**
   * Adds stmtNo into callTable and stmtTable.
   *
   * @param stmtNo Statement number to be inserted
   */
  void addCall(int stmtNo);

  /**
   * Adds the Row {followed, follower} into followsTable.
   *
   * @param followed Statement number of the preceding statement
   * @param follower Statement number of the following statement
   */
  void addFollows(int followed, int follower);

  /**
   * Adds the Row {followed, follower} into followsTTable.
   *
   * @param followed Statement number of the preceding statement
   * @param follower Statement number of the following statement
   */
  void addFollowsT(int followed, int follower);

  /**
   * Adds the Row {parent, child} into parentTable.
   *
   * @param parent Statement number of the container statement
   * @param child Statement number of the child statement
   */
  void addParent(int parent, int child);

  /**
   * Adds the Row {parent, child} into parentTTable.
   *
   * @param parent Statement number of the container statement
   * @param child Statement number of the child statement
   */
  void addParentT(int parent, int child);

  /**
   * Adds the Row {stmtNo, var} into usesSTable.
   *
   * @param stmtNo Statement number of the statement which uses var
   * @param var Variable or constant name which is used by the statement
   */
  void addUsesS(int stmtNo, std::string var);

  /**
   * Adds the Row {proc, var} into usesPTable.
   *
   * @param proc Name of procedure which uses var
   * @param var Variable or constant name which is used by the procedure
   */
  void addUsesP(std::string proc, std::string var);

  /**
   * Adds the Row {stmtNo, var} into modifiesSTable.
   *
   * @param stmtNo Statement number of the statement which modifies var
   * @param var Variable or constant name which is modified by the statement
   */
  void addModifiesS(int stmtNo, std::string var);

  /**
   * Adds the Row {proc, var} into modifiesPTable.
   *
   * @param proc Name of procedure which modifies var
   * @param var Variable or constant name which is modified by the procedure
   */
  void addModifiesP(std::string proc, std::string var);

  /**
   * Adds the Row {caller, called} into callsTable.
   *
   * @param followed Name of the caller Procedure
   * @param follower Name of the Procedure being called
   */
  void addCalls(std::string caller, std::string called);

  /**
   * Adds the Row {caller, called} into callsTTable.
   *
   * @param followed Name of the transitive caller Procedure
   * @param follower Name of the Procedure being transitively called
   */
  void addCallsT(std::string caller, std::string called);

  /**
   * Adds the Row {prev, next} into nextTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed immediately after prev in some execution sequence
   */
  void addNext(int prev, int next);

  /**
   * Adds the Row {prev, next} into nextTTable.
   *
   * @param prev Statement number of the statement executed first
   * @param next Statement number of the statement which can be executed after prev in some execution sequence
   */
  void addNextT(int prev, int next);

  /**
   * Adds the Row {affecter, affected} into affectsTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffect(int affecter, int affected);

  /**
   * Adds the Row {affecter, affected} into affectsTTable.
   *
   * @param prev Statement number of the statement which modifies a variable used in affected
   * @param next Statement number of the statement uses a variable modified by prev
   */
  void addAffectT(int affecter, int affected);


  /**
   * Adds the Row {stmtNo, lhs, rhs} into patterTable.
   *
   * @param stmtNo Statement number of the assign statement
   * @param lhs String of the left hand side of the assign statement
   * @param rhs String of the postfix form of the right hand side of the
   * assign statement
   */
  void addPatternAssign(int stmtNo, std::string lhs, std::string rhs);

  /**
   * Adds the Row {stmtNo, var} int patternIfTable.
   *
   * @param stmtNo Statement number of the if statement
   * @param var Name of variable within the condition of the if statement
   */
  void addPatternIf(int stmtNum, std::string var);

  /**
   * Adds the Row {stmtNo, var} into patternWhileTable.
   *
   * @param stmtNo Statement number of the while statement
   * @param var Name of variable within the condition of the while statement
   */
  void addPatternWhile(int stmtNum, std::string var);

  /**
  * Adds the Row {stmtNo, proc} into callProcTable.
  *
  * @param stmtNo Statement number of the call statement
  * @param var Name of procedure being called by stmtNo
  */
  void addCallProc(int stmtNo, std::string proc);

  /**
  * Adds the Row {stmtNo, var} into readVarTable.
  *
  * @param stmtNo Statement number of the read statement
  * @param var Name of variable being read
  */
  void addReadVar(int stmtNo, std::string var);

  /**
  * Adds the Row {stmtNo, var} into printVarTable.
  *
  * @param stmtNo Statement number of the print statement
  * @param var Name of variable being printed
  */
  void addPrintVar(int stmtNo, std::string var);

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
   * @return affectTTable
   */
  Table getAffectsTTable() const;

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
   * Finds statement number which directly follow {@param stmtNo} and returns it as
   * a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the follower.
   * @return Table containing the follower.
   */
  Table getFollower(int stmtNo) const;

  /**
   * Finds the statement number which directly precede {@param stmtNo} and returns it as
   * a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the preceding statement number.
   * @return Table containing the preceding statement number.
   */
  Table getFollowedBy(int stmtNo) const;

  /**
   * Finds all statement number that directly or indirectly follow {@param stmtNo} and returns
   * them as a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the followers.
   * @return Table containing the followers.
   */
  Table getFollowerT(int stmtNo) const;

  /**
   * Finds all statement number which directly precede {@param stmtNo} and returns them as
   * a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the preceding statement numbers.
   * @return Table containing the preceding statement numbers.
   */
  Table getFollowedByT(int stmtNo) const;

  /**
   * Finds statement number which is the direct parent of  {@param stmtNo} and returns
   * it as a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the parent.
   * @return Table containing the parent statement number.
   */
  Table getParent(int stmtNo) const;

  /**
   * Finds statement number which is the direct child of  {@param stmtNo} and returns
   * it as a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the child.
   * @return Table containing the child statement number.
   */
  Table getChild(int stmtNo) const;

  /**
   * Finds all statement number which are the ancestor of  {@param stmtNo} and returns
   * them as a Table with one column.
   *
   * @param stmtNo Statement number for which we will get the ancestors.
   * @return Table containing the ancestor statement numbers.
   */
  Table getParentT(int stmtNo) const;

  /**
  * Finds all statement number which are the descendant of  {@param stmtNo} and returns
  * them as a Table with one column.
  *
  * @param stmtNo Statement number for which we will get the descendants.
  * @return Table containing the descendant statement numbers.
  */
  Table getChildT(int stmtNo) const;

  /**
   * Finds statement numbers and procedure names that use varName and returns them as a
   * Table with one column. The function addIndirectUses() must be called before calling
   * this function in order for this function to also return the indirect uses relations.
   *
   * @param varName Variable name for which we get the users.
   * @return Table containing the line number or procedure name of users.
   */
  Table getUses(std::string varName) const;

  /**
   * Finds all variables that are used by the stmtNo and returns them as a Table with
   * one column. The function addIndirectUses() must be called before calling
   * this function in order for this function to also return the indirect uses relations.
   *
   * @param stmtNo Statement number for which we will get the variables used.
   * @return Table containing the variable names that is used by the statement.
   */
  Table getUsedBy(int stmtNo) const;

  /**
   * Finds statement numbers and procedure names that modify varName and returns them as a
   * Table with one column. The function addIndirectModify() must be called before calling
   * this function in order for this function to also return the indirect modify relations.
   *
   * @param varName Variable name for which we get the modifiers.
   * @return Table containing the line number or procedure name of modifiers.
   */
  Table getModifies(std::string varName) const;

  /**
   * Finds all variables that are modified by the stmtNo and returns them as a Table with
   * one column. The function addIndirectModify() must be called before calling
   * this function in order for this function to also return the indirect modify relations.
   *
   * @param stmtNo Statement number for which we will get the variables modified.
   * @return Table containing the variable names that is modified by the statement.
   */
  Table getModifiedBy(int stmtNo) const;


private:
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

  Table callProcTable{ 2 };
  Table readVarTable{ 2 };
  Table printVarTable{ 2 };

  Table patternAssignTable{ 3 };
  Table patternIfTable{ 2 };
  Table patternWhileTable{ 2 };
};