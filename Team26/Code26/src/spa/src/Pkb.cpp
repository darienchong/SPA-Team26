#include "Pkb.h"

#include <string>
#include <vector>

#include "Table.h"

Pkb::Pkb() {}

void Pkb::addVar(std::string var) {
	std::vector<std::string> vect{ var };
	varTable.insertRow(vect);
}

void Pkb::addStmt(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	stmtTable.insertRow(vect);
}

void Pkb::addProc(std::string proc) {
	std::vector<std::string> vect{ proc };
	procTable.insertRow(vect);
}

void Pkb::addIf(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	ifTable.insertRow(vect);
}

void Pkb::addWhile(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	whileTable.insertRow(vect);
}

void Pkb::addRead(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	readTable.insertRow(vect);
}

void Pkb::addPrint(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	printTable.insertRow(vect);
}

void Pkb::addAssign(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	assignTable.insertRow(vect);
}

void Pkb::addFollows(int follower, int followed) {
	if (follower >= followed) {
		throw "Follower should come before followed";
	}
	std::vector<std::string> vect{ std::to_string(follower), std::to_string(followed) };
	followsTable.insertRow(vect);
}

void Pkb::addFollowsT() {
	followsTTable.fillTransitiveTable(followsTable);
}

void Pkb::addParent(int parent, int child) {
	if (parent >= child) {
		throw "Parent should come before child";
	}
	std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
	parentTable.insertRow(vect);
}

void Pkb::addParentT() {
	parentTTable.fillTransitiveTable(parentTable);
}

void Pkb::addUses(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	usesTable.insertRow(vect);
}

void Pkb::addUses(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	usesTable.insertRow(vect);
}

void Pkb::addModifies(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	modifiesTable.insertRow(vect);
}

void Pkb::addModifies(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	modifiesTable.insertRow(vect);
}

void Pkb::addPatternAssign(int stmtNo, std::string lhs, std::string rhs) {
	std::vector<std::string> vect{ std::to_string(stmtNo), lhs, rhs };
	patternAssignTable.insertRow(vect);
}

// Getters
Table Pkb::getVarTable() const { return varTable; }
Table Pkb::getStmtTable() const { return stmtTable; }
Table Pkb::getProcTable() const { return procTable; }
Table Pkb::getIfTable() const { return ifTable; }
Table Pkb::getWhileTable() const { return whileTable; }
Table Pkb::getReadTable() const { return readTable; }
Table Pkb::getPrintTable() const { return printTable; }
Table Pkb::getAssignTable() const { return assignTable; }
Table Pkb::getFollowsTable() const { return followsTable; }
Table Pkb::getFollowsTTable() const { return followsTTable; }
Table Pkb::getParentTable() const { return parentTable; }
Table Pkb::getParentTTable() const { return parentTTable; }
Table Pkb::getUsesTable() const { return usesTable; }
Table Pkb::getModifiesTable() const { return modifiesTable; }
Table Pkb::getPatternAssignTable() const { return patternAssignTable; }
