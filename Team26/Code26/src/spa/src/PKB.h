#pragma once

#include "Table.h"

#include <map>

class PKB {

public:
  // constructor
	PKB();

  // add names to table
	void addVar(std::string var);
	void addStmt(int stmtNo);
	void addProc(std::string proc);
  void addConst(std::string constValue);

  // add statements to table of corresponding statement type
	void addIf(int stmtNo);
	void addWhile(int stmtNo);
	void addRead(int stmtNo);
	void addPrint(int stmtNo);
	void addAssign(int stmtNo);

	// add relationships
	void addFollows(int follower, int followed);
	void addFollowsT();
	void addParent(int parent, int child);
	void addParentT();
	void addUses(int stmtNo, std::string var);
	void addUses(std::string proc, std::string var);
	void addIndirectUses();
	void addModifies(int stmtNo, std::string var);
	void addModifies(std::string proc, std::string var);
	void addIndirectModifies();
	void addPatternAssign(int stmtNo, std::string lhs, std::string rhs);

	// get Table of names
	Table getVarTable() const;
	Table getStmtTable() const;
	Table getProcTable() const;
	Table getConstTable() const;

	// get Table of statement numbers
	Table getIfTable() const;
	Table getWhileTable() const;
	Table getReadTable() const;
	Table getPrintTable() const;
	Table getAssignTable() const;

	// get Table of relations
	Table getFollowsTable() const;
	Table getFollowsTTable() const;
	Table getParentTable() const;
	Table getParentTTable() const;
	Table getUsesTable() const;
	Table getModifiesTable() const;
	Table getPatternAssignTable() const;

	Table getFollows(int n) const;
  Table getFollowedBy(int n) const;
  Table getFollowsT(int n) const;
  Table getFollowedTBy(int n) const;
	Table getParent(int n) const;
  Table getChild(int n) const;
	Table getParentT(int n) const;
  Table getChildT(int n) const;
	Table getUses(std::string str) const;
  Table getUsedBy(int n) const;
	Table getModifies(std::string str) const;
	Table getModifiedBy(int n) const;






private:
	Table varTable{ std::vector<std::string> {"varName"} };
	Table stmtTable{ std::vector<std::string> {"stmtNo"} };
	Table procTable{ std::vector<std::string> {"procName"} };
	Table constTable{std::vector<std::string> {"constName"}};

	Table ifTable{ std::vector<std::string> {"stmtNo"} };
	Table whileTable{ std::vector<std::string> {"stmtNo"} };
	Table readTable{ std::vector<std::string> {"stmtNo"} };
	Table printTable{ std::vector<std::string> {"stmtNo"} };
	Table assignTable{ std::vector<std::string> {"stmtNo"} };

	Table followsTable{ std::vector<std::string> {"followed", "follower"} };
	Table followsTTable{ std::vector<std::string> {"followed", "follower"} };
	Table parentTable{ std::vector<std::string> {"parent", "child"} };
	Table parentTTable{ std::vector<std::string> {"parent", "child"} };
	Table usesTable{ std::vector<std::string> {"user", "used"} };
	Table modifiesTable{ std::vector<std::string> {"modifier", "modified"} };
	Table patternAssignTable{ std::vector<std::string> {"stmtNo", "lhs", "rhs"} };
};