#pragma once

#include "TableStub.h"
#include <unordered_set>
#include <vector>

class PKB {
public:
	PKB();
	void addVar(std::string var);
	void addStmt(int stmtNo);
	void addProc(std::string proc);

	void addIf(int stmtNo);
	void addWhile(int stmtNo);
	void addRead(int stmtNo);
	void addPrint(int stmtNo);
	void addAssign(int stmtNo);

	void addFollows(int follower, int followed);
	void addParent(int parent, int child);
	void addUses(int stmtNo, std::string var);
	void addUses(std::string proc, std::string var);
	void addModifies(int stmtNo, std::string var);
	void addModifies(std::string proc, std::string var);

private:
	TableStub varTable;
	TableStub stmtTable;
	TableStub procTable;

	TableStub ifTable;
	TableStub whileTable;
	TableStub readTable;
	TableStub printTable;
	TableStub assignTable;

	TableStub followsTable;
	TableStub parentTable;
	TableStub usesTable;
	TableStub modifiesTable;
};