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

void Pkb::addConst(std::string constValue) {
  std::vector<std::string> vect{ constValue };
  constTable.insertRow(vect);
}

void Pkb::addIf(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	ifTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addWhile(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	whileTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addRead(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	readTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addPrint(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	printTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addAssign(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	assignTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addFollows(int followed, int follower) {
	if (followed >= follower) {
		throw "Follower should come after followed";
	}
	std::vector<std::string> vect{ std::to_string(followed), std::to_string(follower) };
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

void Pkb::addIndirectUses() {
  if (parentTTable.empty()) {
    Pkb::addParentT();
  }
  usesTable.fillIndirectRelation(parentTTable);
}

void Pkb::addIndirectModifies() {
  if (parentTTable.empty()) {
    Pkb::addParentT();
  }
  modifiesTable.fillIndirectRelation(parentTTable);
}

// Getters
Table Pkb::getVarTable() const { return varTable; }
Table Pkb::getStmtTable() const { return stmtTable; }
Table Pkb::getProcTable() const { return procTable; }
Table Pkb::getConstTable() const { return constTable; }
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

Table Pkb::getFollower(int n) const {
  Table filterTable = followsTable;
  filterTable.filterColumn("followed", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("followed");
  return filterTable;
}

Table Pkb::getFollowedBy(int n) const {
  Table filterTable = followsTable;
  filterTable.filterColumn("follower", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("follower");
  return filterTable;
}

Table Pkb::getFollowerT(int n) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn("followed", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("followed");
  return filterTable;
}

Table Pkb::getFollowedByT(int n) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn("follower", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("follower");
  return filterTable;
}

Table Pkb::getParent(int n) const {
  Table filterTable = parentTable;
  filterTable.filterColumn("child", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("child");
  return filterTable;
}

Table Pkb::getChild(int n) const {
  Table filterTable = parentTable;
  filterTable.filterColumn("parent", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("parent");
  return filterTable;
}

Table Pkb::getParentT(int n) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn("child", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("child");
  return filterTable;
}

Table Pkb::getChildT(int n) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn("parent", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("parent");
  return filterTable;
}

Table Pkb::getUses(std::string str) const {
  Table filterTable = usesTable;
  filterTable.filterColumn("used", std::set<std::string> {str});
  filterTable.dropColumn("used");
  return filterTable;
}

Table Pkb::getUsedBy(int n) const {
  Table filterTable = usesTable;
  filterTable.filterColumn("user", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("user");
  return filterTable;
}

Table Pkb::getUsedBy(std::string str) const {
  Table filterTable = usesTable;
  filterTable.filterColumn("user", std::set<std::string> {str});
  filterTable.dropColumn("user");
  return filterTable;
}

Table Pkb::getModifies(std::string str) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn("modified", std::set<std::string> {str});
  filterTable.dropColumn("modified");
  return filterTable;
}

Table Pkb::getModifiedBy(int n) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn("modifier", std::set<std::string> {std::to_string(n)});
  filterTable.dropColumn("modifier");
  return filterTable;
}

Table Pkb::getModifiedBy(std::string str) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn("modifier", std::set<std::string> {str});
  filterTable.dropColumn("modifier");
  return filterTable;
}
