#pragma once

#include "Table.h"

#include <map>

class Pkb {

public:
  // constructor
	Pkb();

  // add names to table
	void addVar(std::string var);
	void addStmt(int stmtNo);
	void addProc(std::string proc);
	void addConst(std::string constant);

  // add statements to table of corresponding statement type
	void addIf(int stmtNo);
	void addWhile(int stmtNo);
	void addRead(int stmtNo);
	void addPrint(int stmtNo);
	void addAssign(int stmtNo);

	// add relationships
	void addFollows(int follower, int followed);
	void addFollowsT();
  void addFollowsT(int follower, int followed);
	void addParent(int parent, int child);
	void addParentT();
  void addParentT(int parent, int child);
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

	Table getFollower(int stmtNo) const;
  Table getFollowedBy(int stmtNo) const;
  Table getFollowerT(int stmtNo) const;
  Table getFollowedByT(int stmtNo) const;
	Table getParent(int stmtNo) const;
  Table getChild(int stmtNo) const;
	Table getParentT(int stmtNo) const;
  Table getChildT(int stmtNo) const;
	Table getUses(std::string varName) const;
  Table getUsedBy(int stmtNo) const;
  Table getUsedBy(std::string procName) const;
	Table getModifies(std::string varName) const;
	Table getModifiedBy(int stmtNo) const;
  Table getModifiedBy(std::string procName) const; 

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