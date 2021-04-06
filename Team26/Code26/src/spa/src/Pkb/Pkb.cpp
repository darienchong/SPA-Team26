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
    if (getCallTable().getData().count({ from }) == 0) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::DUMMY);
    }
  } else {
    cfg.addEdge(from, to);
    addNext(from, to);
    if (getCallTable().getData().count({ from }) == 0) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::NORMAL);
      addNextBip(from, to);
    }
  }
}

void Pkb::addPatternIf(const int stmtNum, const std::string& var) {
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  patternIfTable.insertRow({ stmtNum, entityToIntMapper.at(var) });
}

void Pkb::addPatternWhile(const int stmtNum, const std::string& var) {
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  patternWhileTable.insertRow({ stmtNum, entityToIntMapper.at(var) });
}

void Pkb::addCall(const int stmtNum) {
  callTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addCalls(const std::string& caller, const std::string& called) {
  if (entitySet.count(caller) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(caller);
    intToEntityMapper.emplace(intRef, caller);
    entityToIntMapper.emplace(caller, intRef);
  }
  if (entitySet.count(called) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(called);
    intToEntityMapper.emplace(intRef, called);
    entityToIntMapper.emplace(called, intRef);
  }
  callsTable.insertRow({ entityToIntMapper.at(caller), entityToIntMapper.at(called) });
}

void Pkb::addCallsT(const std::string& caller, const std::string& called) {
  if (entitySet.count(caller) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(caller);
    intToEntityMapper.emplace(intRef, caller);
    entityToIntMapper.emplace(caller, intRef);
  }
  if (entitySet.count(called) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(called);
    intToEntityMapper.emplace(intRef, called);
    entityToIntMapper.emplace(called, intRef);
  }
  callsTTable.insertRow({ entityToIntMapper.at(caller), entityToIntMapper.at(called) });
}

void Pkb::addNext(const int prev, const int next) {
  nextTable.insertRow({ prev, next });
}

void Pkb::addNextT(const int prev, const int next) {
  nextTTable.insertRow({ prev, next });
}
void Pkb::addAffects(const int affecter, const int affected) {
  affectsTable.insertRow({ affecter, affected });
}
void Pkb::addAffectsT(const int affecter, const int affected) {
  affectsTTable.insertRow({ affecter, affected });
}

void Pkb::addNextBip(const int prev, const int next) {
  nextBipTable.insertRow({ prev, next });
}

void Pkb::addNextBipT(const int prev, const int next) {
  nextBipTTable.insertRow({ prev, next });
}
void Pkb::addAffectsBip(const int affecter, const int affected) {
  affectsBipTable.insertRow({ affecter, affected });
}
void Pkb::addAffectsBipT(const int affecter, const int affected) {
  affectsBipTTable.insertRow({ affecter, affected });
}

void Pkb::addCallProc(const int stmtNum, const std::string& proc) {
  callProcMapper[stmtNum] = proc;
  if (entitySet.count(proc) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(proc);
    intToEntityMapper.emplace(intRef, proc);
    entityToIntMapper.emplace(proc, intRef);
  }
  callProcTable.insertRow({ stmtNum, entityToIntMapper[proc] });
}

void Pkb::addReadVar(const int stmtNum, const std::string& var) {
  readVarMapper[stmtNum] = var;
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  readVarTable.insertRow({ stmtNum, entityToIntMapper[var] });
}

void Pkb::addPrintVar(const int stmtNum, const std::string& var) {
  printVarMapper[stmtNum] = var;
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  printVarTable.insertRow({ stmtNum, entityToIntMapper[var] });
}

void Pkb::addVar(const std::string& var) {
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  varTable.insertRow({ entityToIntMapper[var] });
}

void Pkb::addStmt(const int stmtNum) {
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addProc(const std::string& proc) {
  if (entitySet.count(proc) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(proc);
    intToEntityMapper.emplace(intRef, proc);
    entityToIntMapper.emplace(proc, intRef);
  }
  procTable.insertRow({ entityToIntMapper[proc] });
}

void Pkb::addConst(const std::string& constValue) {
  if (entitySet.count(constValue) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(constValue);
    intToEntityMapper.emplace(intRef, constValue);
    entityToIntMapper.emplace(constValue, intRef);
  }
  constTable.insertRow({ entityToIntMapper[constValue] });
}

void Pkb::addIf(const int stmtNum) {
  ifTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addWhile(const int stmtNum) {
  whileTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addRead(const int stmtNum) {
  readTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addPrint(const int stmtNum) {
  printTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addAssign(const int stmtNum) {
  assignTable.insertRow({ stmtNum });
  stmtTable.insertRow({ stmtNum });
}

void Pkb::addFollows(const int followed, const int follower) {
  assert(followed < follower);
  followsTable.insertRow({ followed, follower });
}

void Pkb::addFollowsT(const int followed, const int follower) {
  assert(followed < follower);
  followsTTable.insertRow({ followed, follower });
}

void Pkb::addParent(const int parent, const int child) {
  assert(parent < child);
  parentTable.insertRow({ parent, child });
}

void Pkb::addParentT(const int parent, const int child) {
  assert(parent < child);
  parentTTable.insertRow({ parent, child });
}

void Pkb::addUsesS(const int stmtNum, const std::string& var) {
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  usesSTable.insertRow({ stmtNum, entityToIntMapper[var] });
}

void Pkb::addModifiesS(const int stmtNum, const std::string& var) {
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  modifiesSTable.insertRow({ stmtNum, entityToIntMapper[var] });
}

void Pkb::addModifiesP(const std::string& proc, const std::string& var) {
  if (entitySet.count(proc) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(proc);
    intToEntityMapper.emplace(intRef, proc);
    entityToIntMapper.emplace(proc, intRef);
  }
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  modifiesPTable.insertRow({ entityToIntMapper[proc], entityToIntMapper[var] });
}

void Pkb::addUsesP(const std::string& proc, const std::string& var) {
  if (entitySet.count(proc) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(proc);
    intToEntityMapper.emplace(intRef, proc);
    entityToIntMapper.emplace(proc, intRef);
  }
  if (entitySet.count(var) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(var);
    intToEntityMapper.emplace(intRef, var);
    entityToIntMapper.emplace(var, intRef);
  }
  usesPTable.insertRow({ entityToIntMapper[proc], entityToIntMapper[var] });
}

void Pkb::addPatternAssign(const int stmtNum, const std::string& lhs, const std::string& rhs) {
  if (entitySet.count(lhs) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(lhs);
    intToEntityMapper.emplace(intRef, lhs);
    entityToIntMapper.emplace(lhs, intRef);
  }
  if (entitySet.count(rhs) == 0) {
    const int intRef = entitySet.size();
    entitySet.insert(rhs);
    intToEntityMapper.emplace(intRef, rhs);
    entityToIntMapper.emplace(rhs, intRef);
  }
  patternAssignTable.insertRow({ stmtNum, entityToIntMapper[lhs], entityToIntMapper[rhs] });
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

int Pkb::getIntRefFromEntity(const std::string& entity) const {
  if (entitySet.count(entity) == 0) {
    return -1;
  }
  return entityToIntMapper.at(entity);
}

std::string Pkb::getEntityFromIntRef(const int entity) const {
  assert(intToEntityMapper.count(entity) == 1);
  return intToEntityMapper.at(entity);
}

std::unordered_set<int> Pkb::getAssignUses(const std::string& varName) const {
  Table filterTable = usesSTable;
  filterTable.filterColumn(1, { entityToIntMapper.at(varName) });
  filterTable.innerJoin(assignTable, 0, 0);
  std::unordered_set<int> stmtNumbers;
  for (Row row : filterTable.getData()) {
    stmtNumbers.insert(row[0]);
  }
  return stmtNumbers;
}

std::unordered_set<std::string> Pkb::getModifiedBy(const int stmtNum) const {
  Table filterTable(getModifiesSTable());
  filterTable.filterColumn(0, { stmtNum });
  std::unordered_set<std::string> variablesModified;
  for (const Row& row : filterTable.getData()) {
    variablesModified.insert(intToEntityMapper.at(row[1]));
  }
  return variablesModified;
}

std::string Pkb::getProcNameFromCallStmt(const int stmtNum) const {
  const bool isValidCallStmt = callTable.contains({ stmtNum });
  if (isValidCallStmt) {
    return callProcMapper.at(stmtNum);
  } else {
    return "";
  }
}

std::string Pkb::getVarNameFromReadStmt(const int stmtNum) const {
  const bool isValidReadStmt = readTable.contains({ stmtNum });
  if (isValidReadStmt) {
    return readVarMapper.at(stmtNum);
  } else {
    return "";
  }
}

std::string Pkb::getVarNameFromPrintStmt(const int stmtNum) const {
  const bool isValidPrintStmt = printTable.contains({ stmtNum });
  if (isValidPrintStmt) {
    return printVarMapper.at(stmtNum);
  } else {
    return "";
  }
}

std::vector<int> Pkb::getNextStmtsFromCfg(const int stmtNum) const {
  return cfg.getNeighbours(stmtNum);
}

std::vector<Cfg::BipNode> Pkb::getNextStmtsFromCfgBip(const int stmtNum) const {
  return cfgBip.getNeighbours(stmtNum);
}

int Pkb::getStartStmtFromProc(const std::string& proc) const {
  return procStartMapper.at(proc);
}

std::vector<int> Pkb::getEndStmtsFromProc(const std::string& proc) const {
  return procEndMapper.at(proc);
}

std::string Pkb::getProcFromStmt(const int stmt) const {
  if (stmtProcMapper.count(stmt) == 1) {
    return stmtProcMapper.at(stmt);
  }
  return "";
}
