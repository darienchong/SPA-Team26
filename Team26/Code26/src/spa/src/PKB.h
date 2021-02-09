#pragma once

#include "Table.h"

#include <map>
#include <vector>

class PKB {
public:
  // constructor
	PKB();

  // add names to table
	void addVar(std::string var);
	void addStmt(int stmtNo);
	void addProc(std::string proc);

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
	void addModifies(int stmtNo, std::string var);
	void addModifies(std::string proc, std::string var);
	void addPatternAssign(int stmtNo, std::string lhs, std::string rhs);

	// get Table of names
	Table getVarTable();
	Table getStmtTable();
	Table getProcTable();

	// get Table of statement numbers
	Table getIfTable();
	Table getWhileTable();
	Table getReadTable();
	Table getPrintTable();
	Table getAssignTable();

	// get Table of relations
	Table getFollowsTable();
	Table getFollowsTTable();
	Table getParentTable();
	Table getParentTTable();
	Table getUsesTable();
	Table getModifiesTable();
	Table getPatternAssignTable();

private:
	Table varTable;
	Table stmtTable;
	Table procTable;

	Table ifTable;
	Table whileTable;
	Table readTable;
	Table printTable;
	Table assignTable;

	Table followsTable{ 2 };
	Table followsTTable{ 2 };
	Table parentTable{ 2 };
	Table parentTTable{ 2 };
	Table usesTable{ 2 };
	Table modifiesTable{ 2 };
	Table patternAssignTable{ 3 };
};