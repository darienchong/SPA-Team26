#include "Pkb.h"

#include <assert.h>

#include <string>
#include <utility>
#include <vector>
#include <unordered_set>
#include <stdexcept>

#include "Table.h"
#include "Cfg.h"
#include "CfgBip.h"

Pkb::Pkb() {}

void Pkb::addProcRange(const std::string proc, const int first, const int last) {
  for (int stmt = first; stmt <= last; ++stmt) {
    stmtProcMapper.emplace(stmt, proc);
  }
}

void Pkb::addProcStartEnd(const std::string proc, const int start, const std::vector<int> end) {
  procStartMapper.emplace(proc, start);
  procEndMapper.emplace(proc, end);
}

void Pkb::addCfgBipEdge(const int from, const int to, const int label, const Cfg::NodeType type) {
  cfgBip.addBipEdge(from, to, label, type);
  if (type != Cfg::NodeType::DUMMY && type != Cfg::NodeType::BRANCH_BACK) {
    addNextBip(from, to);
  }
}

void Pkb::addCfgEdge(const int from, const int to) {
  if (to < 0) {
    assert(from > 0);
    cfg.addEdge(from, to);
    if (getCallTable().getData().count({ std::to_string(from) }) == 0) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::DUMMY);
    }
  }
  else {
    cfg.addEdge(from, to);
    addNext(from, to);
    if (getCallTable().getData().count({ std::to_string(from) }) == 0) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::NORMAL);
      addNextBip(from, to);
    }
  }
}

void Pkb::addModifiesP(std::string proc, std::string var) {
  std::vector<std::string> vect{ proc, std::move(var) };
  modifiesPTable.insertRow(vect);
}

void Pkb::addUsesP(std::string proc, std::string var) {
  std::vector<std::string> vect{ proc, std::move(var) };
  usesPTable.insertRow(vect);
}

void Pkb::addPatternIf(int stmtNum, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNum), std::move(var) };
  patternIfTable.insertRow(vect);
}

void Pkb::addPatternWhile(int stmtNum, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNum), std::move(var) };
  patternWhileTable.insertRow(vect);
}

void Pkb::addCall(int stmtNo) {
  std::vector<std::string> vect{ std::to_string(stmtNo) };
  callTable.insertRow(vect);
  stmtTable.insertRow(vect);
}

void Pkb::addCalls(std::string caller, std::string called) {
  std::vector<std::string> vect{ caller, called };
  callsTable.insertRow(vect);
}

void Pkb::addCallsT(std::string caller, std::string called) {
  std::vector<std::string> vect{ caller, called };
  callsTTable.insertRow(vect);
}

void Pkb::addNext(int prev, int next) {
  std::vector<std::string> vect{ std::to_string(prev), std::to_string(next) };
  nextTable.insertRow(vect);
}

void Pkb::addNextT(int prev, int next) {
  std::vector<std::string> vect{ std::to_string(prev), std::to_string(next) };
  nextTTable.insertRow(vect);
}
void Pkb::addAffects(int affecter, int affected) {
  std::vector<std::string> vect{ std::to_string(affecter), std::to_string(affected) };
  affectsTable.insertRow(vect);
}
void Pkb::addAffectsT(int affecter, int affected) {
  std::vector<std::string> vect{ std::to_string(affecter), std::to_string(affected) };
  affectsTTable.insertRow(vect);
}

void Pkb::addNextBip(int prev, int next) {
  std::vector<std::string> vect{ std::to_string(prev), std::to_string(next) };
  nextBipTable.insertRow(vect);
}

void Pkb::addNextBipT(int prev, int next) {
  std::vector<std::string> vect{ std::to_string(prev), std::to_string(next) };
  nextBipTTable.insertRow(vect);
}
void Pkb::addAffectsBip(int affecter, int affected) {
  std::vector<std::string> vect{ std::to_string(affecter), std::to_string(affected) };
  affectsBipTable.insertRow(vect);
}
void Pkb::addAffectsBipT(int affecter, int affected) {
  std::vector<std::string> vect{ std::to_string(affecter), std::to_string(affected) };
  affectsBipTTable.insertRow(vect);
}

void Pkb::addCallProc(int stmtNo, std::string proc) {
  std::vector<std::string> vect{ std::to_string((stmtNo)), proc };
  callProcTable.insertRow(vect);
  callProcMapper[stmtNo] = proc;
}

void Pkb::addReadVar(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string((stmtNo)), var };
  readVarTable.insertRow(vect);
  readVarMapper[stmtNo] = var;
}

void Pkb::addPrintVar(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string((stmtNo)), var };
  printVarTable.insertRow(vect);
  printVarMapper[stmtNo] = var;
}

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
  assert(followed < follower);
  std::vector<std::string> vect{ std::to_string(followed), std::to_string(follower) };
  followsTable.insertRow(vect);
}

void Pkb::addFollowsT(int followed, int follower) {
  assert(followed < follower);
  std::vector<std::string> vect{ std::to_string(followed), std::to_string(follower) };
  followsTTable.insertRow(vect);
}

void Pkb::addParent(int parent, int child) {
  assert(parent < child);
  std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
  parentTable.insertRow(vect);
}

void Pkb::addParentT(int parent, int child) {
  assert(parent < child);
  std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
  parentTTable.insertRow(vect);
}

void Pkb::addUsesS(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(var) };
  usesSTable.insertRow(vect);
}

void Pkb::addModifiesS(int stmtNo, std::string var) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(var) };
  modifiesSTable.insertRow(vect);
}

void Pkb::addPatternAssign(int stmtNo, std::string lhs, std::string rhs) {
  std::vector<std::string> vect{ std::to_string(stmtNo), std::move(lhs), std::move(rhs) };
  patternAssignTable.insertRow(vect);
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
Table Pkb::getUsesSTable() const { return usesSTable; }
Table Pkb::getModifiesSTable() const { return modifiesSTable; }
Table Pkb::getPatternAssignTable() const { return patternAssignTable; }

Table Pkb::getCallTable() const { return callTable; }
Table Pkb::getCallsTable() const { return callsTable; }
Table Pkb::getCallsTTable() const { return callsTTable; }
Table Pkb::getNextTable() const { return nextTable; }
Table Pkb::getNextTTable() const { return nextTTable; }
Table Pkb::getAffectsTable() const { return affectsTable; }
Table Pkb::getAffectsTTable() const { return affectsTTable; }
Table Pkb::getNextBipTable() const { return nextBipTable; }
Table Pkb::getNextBipTTable() const { return nextBipTTable; }
Table Pkb::getAffectsBipTable() const { return affectsBipTable; }
Table Pkb::getAffectsBipTTable() const { return affectsBipTTable; }
Table Pkb::getUsesPTable() const { return usesPTable; }
Table Pkb::getModifiesPTable() const { return modifiesPTable; }
Table Pkb::getCallProcTable() const { return callProcTable; }
Table Pkb::getReadVarTable() const { return readVarTable; }
Table Pkb::getPrintVarTable() const { return printVarTable; }
Table Pkb::getPatternIfTable() const { return patternIfTable; }
Table Pkb::getPatternWhileTable() const { return patternWhileTable; }

std::unordered_set<int> Pkb::getAssignUses(const std::string& varName) const {
  Table filterTable = usesSTable;
  filterTable.filterColumn(1, { varName });
  filterTable.innerJoin(assignTable, 0, 0);
  std::unordered_set<int> stmtNumbers;
  for (Row row : filterTable.getData()) {
    stmtNumbers.insert(std::stoi(row[0]));
  }
  return stmtNumbers;
}

std::unordered_set<std::string> Pkb::getModifiedBy(const int stmtNo) const {
  Table filterTable = modifiesSTable;
  filterTable.filterColumn(0, { std::to_string(stmtNo) });
  std::unordered_set<std::string> variablesModified;
  for (Row row : filterTable.getData()) {
    variablesModified.insert(row[1]);
  }
  return variablesModified;
}

std::string Pkb::getProcNameFromCallStmt(const int stmtNo) const {
  const bool isValidCallStmt = callTable.contains({ std::to_string(stmtNo) });
  if (isValidCallStmt) {
    return callProcMapper.at(stmtNo);
  } else {
    return "";
  }
}

std::string Pkb::getVarNameFromReadStmt(const int stmtNo) const {
  const bool isValidReadStmt = readTable.contains({ std::to_string(stmtNo) });
  if (isValidReadStmt) {
    return readVarMapper.at(stmtNo);
  } else {
    return "";
  }
}

std::string Pkb::getVarNameFromPrintStmt(const int stmtNo) const {
  const bool isValidPrintStmt = printTable.contains({ std::to_string(stmtNo) });
  if (isValidPrintStmt) {
    return printVarMapper.at(stmtNo);
  } else {
    return "";
  }
}

std::vector<int> Pkb::getNextStmtsFromCfg(const int stmtNo) const {
  return cfg.getNeighbours(stmtNo);
}

std::vector<Cfg::BipNode> Pkb::getNextStmtsFromCfgBip(const int stmtNo) const {
  return cfgBip.getNeighbours(stmtNo);
}

int Pkb::getStartStmtFromProc(const std::string proc) const {
  return procStartMapper.at(proc);
}

std::vector<int> Pkb::getEndStmtsFromProc(const std::string proc) const {
  return procEndMapper.at(proc);
}

std::string Pkb::getProcFromStmt(const int stmt) const {
  if (stmtProcMapper.count(stmt) == 1) {
    return stmtProcMapper.at(stmt);
  }
  return "";
}
