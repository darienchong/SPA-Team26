#include "PKB.h"

#include <string>
#include <vector>

#include "Table.h"

PKB::PKB() {}

void PKB::addVar(std::string var) {
	std::vector<std::string> vect{ var };
	varTable.insertRow(vect);
}

void PKB::addStmt(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	stmtTable.insertRow(vect);
}

void PKB::addProc(std::string proc) {
	std::vector<std::string> vect{ proc };
	procTable.insertRow(vect);
}

void PKB::addConst(std::string constValue) {
  std::vector<std::string> vect{ constValue };
  constTable.insertRow(vect);
}

void PKB::addIf(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	ifTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void PKB::addWhile(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	whileTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void PKB::addRead(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	readTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void PKB::addPrint(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	printTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void PKB::addAssign(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	assignTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void PKB::addFollows(int follower, int followed) {
	if (follower >= followed) {
		throw "Follower should come before followed";
	}
	std::vector<std::string> vect{ std::to_string(follower), std::to_string(followed) };
	followsTable.insertRow(vect);
}

void PKB::addFollowsT() {
	followsTTable.fillTransitiveTable(followsTable);
}

void PKB::addParent(int parent, int child) {
	if (parent >= child) {
		throw "Parent should come before child";
	}
	std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
	parentTable.insertRow(vect);
}

void PKB::addParentT() {
	parentTTable.fillTransitiveTable(parentTable);
}

void PKB::addUses(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	usesTable.insertRow(vect);
}

void PKB::addUses(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	usesTable.insertRow(vect);
}

void PKB::addModifies(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	modifiesTable.insertRow(vect);
}

void PKB::addModifies(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	modifiesTable.insertRow(vect);
}

void PKB::addPatternAssign(int stmtNo, std::string lhs, std::string rhs) {
	std::vector<std::string> vect{ std::to_string(stmtNo), lhs, rhs };
	patternAssignTable.insertRow(vect);
}

void PKB::addIndirectUses() {
  if (parentTTable.empty()) {
    PKB::addParentT();
  }
  usesTable.fillIndirectRelation(parentTTable);
}

void PKB::addIndirectModifies() {
  if (parentTTable.empty()) {
    PKB::addParentT();
  }
  modifiesTable.fillIndirectRelation(parentTTable);
}

// Getters
Table PKB::getVarTable() const { return varTable; }
Table PKB::getStmtTable() const { return stmtTable; }
Table PKB::getProcTable() const { return procTable; }
Table PKB::getConstTable() const { return constTable; }
Table PKB::getIfTable() const { return ifTable; }
Table PKB::getWhileTable() const { return whileTable; }
Table PKB::getReadTable() const { return readTable; }
Table PKB::getPrintTable() const { return printTable; }
Table PKB::getAssignTable() const { return assignTable; }
Table PKB::getFollowsTable() const { return followsTable; }
Table PKB::getFollowsTTable() const { return followsTTable; }
Table PKB::getParentTable() const { return parentTable; }
Table PKB::getParentTTable() const { return parentTTable; }
Table PKB::getUsesTable() const { return usesTable; }
Table PKB::getModifiesTable() const { return modifiesTable; }
Table PKB::getPatternAssignTable() const { return patternAssignTable; }

Table PKB::getFollows(int n) const {
  Table filterTable = followsTable;
  filterTable.filterColumn("follower", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("followed");
  return filterTable;
}

Table PKB::getFollowedBy(int n) const {
  Table filterTable = followsTable;
  filterTable.filterColumn("followed", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("follower");
  return filterTable;
}

Table PKB::getFollowsT(int n) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn("follower", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("followed");
  return filterTable;
}

Table PKB::getFollowedTBy(int n) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn("followed", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("follower");
  return filterTable;
}

Table PKB::getParent(int n) const {
  Table filterTable = parentTable;
  filterTable.filterColumn("child", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("parent");
  return filterTable;
}

Table PKB::getChild(int n) const {
  Table filterTable = parentTable;
  filterTable.filterColumn("parent", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("child");
  return filterTable;
}

Table PKB::getParentT(int n) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn("child", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("parent");
  return filterTable;
}

Table PKB::getChildT(int n) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn("parent", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("child");
  return filterTable;
}

Table PKB::getUses(std::string str) const {
  Table filterTable = usesTable;
  filterTable.filterColumn("used", std::set<std::string> {str});
  filterTable.dropColumn("user");
  return filterTable;
}

Table PKB::getUsedBy(int n) const {
  Table filterTable = usesTable;
  filterTable.filterColumn("user", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("used");
  return filterTable;
}

Table PKB::getModifies(std::string str) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn("modified", std::set<std::string> {str});
  filterTable.dropColumn("modifies");
  return filterTable;
}

Table PKB::getModifiedBy(int n) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn("modifies", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("modified");
  return filterTable;
}
