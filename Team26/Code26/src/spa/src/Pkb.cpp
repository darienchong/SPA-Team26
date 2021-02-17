#include "Pkb.h"

#include <string>
#include <utility>
#include <vector>

#include "Table.h"

Pkb::Pkb() = default;

void Pkb::addVar(std::string var) {
  std::vector<std::string> vect{ std::move(var) };
  varTable.insertRow(vect);
}

void Pkb::addStmt(int stmtNo) {
  std::vector<std::string> vect{ std::to_string(stmtNo) };
  stmtTable.insertRow(vect);
}

void Pkb::addProc(std::string proc) {
  std::vector<std::string> vect{ std::move(proc) };
  procTable.insertRow(vect);
}

void Pkb::addConst(std::string constValue) {
  std::vector<std::string> vect{ std::move(constValue) };
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

void Pkb::addFollowsT(int followed, int follower) {
  if (followed >= follower) {
    throw "Follower should come after followed";
  }
  std::vector<std::string> vect{ std::to_string(followed), std::to_string(follower) };
  followsTTable.insertRow(vect);
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

void Pkb::addParentT(int parent, int child) {
  if (parent >= child) {
    throw "Parent should come before child";
  }
  std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
  parentTTable.insertRow(vect);
}

void Pkb::addUses(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(var) };
  usesTable.insertRow(vect);
}

void Pkb::addUses(std::string proc, std::string var) {
  std::vector<std::string> vect{ std::move(proc) , std::move(var) };
  usesTable.insertRow(vect);
}

void Pkb::addModifies(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(var) };
  modifiesTable.insertRow(vect);
}

void Pkb::addModifies(std::string proc, std::string var) {
  std::vector<std::string> vect{ std::move(proc) , std::move(var) };
  modifiesTable.insertRow(vect);
}

void Pkb::addPatternAssign(int stmtNo, std::string lhs, std::string rhs) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(lhs), std::move(rhs) };
  patternAssignTable.insertRow(vect);
}

void Pkb::addIndirectUses() {
  if (parentTTable.empty()) {
    Pkb::addParentT();
  }
  fillIndirectRelation(usesTable);
}

void Pkb::addIndirectModifies() {
  if (parentTTable.empty()) {
    Pkb::addParentT();
  }
  fillIndirectRelation(modifiesTable);
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

Table Pkb::getFollower(int stmtNo) const {
  Table filterTable = followsTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getFollowedBy(int stmtNo) const {
  Table filterTable = followsTable;
  filterTable.filterColumn(1, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getFollowerT(int stmtNo) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getFollowedByT(int stmtNo) const {
  Table filterTable = followsTTable;
  filterTable.filterColumn(1, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getParent(int stmtNo) const {
  Table filterTable = parentTable;
  filterTable.filterColumn(1, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getChild(int stmtNo) const {
  Table filterTable = parentTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getParentT(int stmtNo) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn(1, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getChildT(int stmtNo) const {
  Table filterTable = parentTTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getUses(std::string varName) const {
  Table filterTable = usesTable;
  filterTable.filterColumn(1, std::set<std::string> {std::move(varName)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getUsedBy(int stmtNo) const {
  Table filterTable = usesTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getUsedBy(std::string procName) const {
  Table filterTable = usesTable;
  filterTable.filterColumn(0, std::set<std::string> {std::move(procName)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getModifies(std::string varName) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn(1, std::set<std::string> {std::move(varName)});
  filterTable.dropColumn(1);
  return filterTable;
}

Table Pkb::getModifiedBy(int stmtNo) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn(0, std::set<std::string> {std::to_string(stmtNo)});
  filterTable.dropColumn(0);
  return filterTable;
}

Table Pkb::getModifiedBy(std::string procName) const {
  Table filterTable = modifiesTable;
  filterTable.filterColumn(0, std::set<std::string> {std::move(procName)});
  filterTable.dropColumn(0);
  return filterTable;
}

void Pkb::fillIndirectRelation(Table& toUpdateTable) {
  if (toUpdateTable.getHeader().size() != 2 || parentTTable.getHeader().size() != 2) {
    throw "Tables must have 2 columns.";
  }
  Table newParentTTable = parentTTable;
  newParentTTable.innerJoin(toUpdateTable, 1, 0); //resulting table header = {parent, child, varName}
  newParentTTable.dropColumn(1);
  toUpdateTable.concatenate(newParentTTable);
}
