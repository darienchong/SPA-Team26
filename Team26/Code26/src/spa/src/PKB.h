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


private:
	Table varTable{ 1 };
	Table stmtTable{ 1 };
	Table procTable{ 1 };
	Table constTable{1};

	Table ifTable{ 1 };
	Table whileTable{ 1 };
	Table readTable{ 1 };
	Table printTable{ 1 };
	Table assignTable{ 1 };

	Table followsTable{ 2 };
	Table followsTTable{ 2 };
	Table parentTable{ 2 };
	Table parentTTable{ 2 };
	Table usesTable{ 2 };
	Table modifiesTable{ 2 };
	Table patternAssignTable{ 3 };
};