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
    if (!callTable.contains({ getIntRefFromStmtNum(from) })) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::DUMMY);
    }
  } else {
    cfg.addEdge(from, to);
    addNext(from, to);
    if (!callTable.contains({ getIntRefFromStmtNum(from) })) {
      cfgBip.addBipEdge(from, to, 0, Cfg::NodeType::NORMAL);
      addNextBip(from, to);
    }
  }
}

void Pkb::addPatternIf(const int stmtNum, const std::string& var) {
  patternIfTable.insertRow({ addEntity(std::to_string(stmtNum)), addEntity(var) });
}

void Pkb::addPatternWhile(const int stmtNum, const std::string& var) {
  patternWhileTable.insertRow({ addEntity(std::to_string(stmtNum)), addEntity(var) });
}

void Pkb::addCalls(const std::string& caller, const std::string& called) {
  callsTable.insertRow({ addEntity(caller), addEntity(called) });
}

void Pkb::addCallsT(const std::string& caller, const std::string& called) {
  callsTTable.insertRow({ addEntity(caller), addEntity(called) });
}

void Pkb::addNext(const int prev, const int next) {
  nextTable.insertRow({ addEntity(std::to_string(prev)), addEntity(std::to_string(next)) });
}

void Pkb::addNextT(const int prev, const int next) {
  nextTTable.insertRow({ addEntity(std::to_string(prev)), addEntity(std::to_string(next)) });
}
void Pkb::addAffects(const int affecter, const int affected) {
  affectsTable.insertRow({ addEntity(std::to_string(affecter)), addEntity(std::to_string(affected)) });
}
void Pkb::addAffectsT(const int affecter, const int affected) {
  affectsTTable.insertRow({ addEntity(std::to_string(affecter)), addEntity(std::to_string(affected)) });
}

void Pkb::addNextBip(const int prev, const int next) {
  nextBipTable.insertRow({ addEntity(std::to_string(prev)), addEntity(std::to_string(next)) });
}

void Pkb::addNextBipT(const int prev, const int next) {
  nextBipTTable.insertRow({ addEntity(std::to_string(prev)), addEntity(std::to_string(next)) });
}
void Pkb::addAffectsBip(const int affecter, const int affected) {
  affectsBipTable.insertRow({ addEntity(std::to_string(affecter)), addEntity(std::to_string(affected)) });
}
void Pkb::addAffectsBipT(const int affecter, const int affected) {
  affectsBipTTable.insertRow({ addEntity(std::to_string(affecter)), addEntity(std::to_string(affected)) });
}

void Pkb::addCallProc(const int stmtNum, const std::string& proc) {
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  callIntRefToProcMapper[stmtNumIntRef] = proc;
  callProcTable.insertRow({ stmtNumIntRef, addEntity(proc) });
}

void Pkb::addReadVar(const int stmtNum, const std::string& var) {
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  readIntRefToVarMapper[stmtNumIntRef] = var;
  readVarTable.insertRow({ stmtNumIntRef, addEntity(var) });
}

void Pkb::addPrintVar(const int stmtNum, const std::string& var) {
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  printIntRefToVarMapper[stmtNumIntRef] = var;
  printVarTable.insertRow({ stmtNumIntRef, addEntity(var) });
}

void Pkb::addVar(const std::string& var) {
  const int varIntRef = addEntity(var);
  varTable.insertRow({ varIntRef });
  varIntRefs.emplace(varIntRef);
}

void Pkb::addStmt(const int stmtNum) {
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  stmtTable.insertRow({ stmtNumIntRef });
  stmtIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addProc(const std::string& proc) {
  const int procIntRef = addEntity(proc);
  procTable.insertRow({ procIntRef });
  procIntRefs.emplace(procIntRef);
}

void Pkb::addConst(const std::string& constValue) {
  const int constIntRef = addEntity(constValue);
  constTable.insertRow({ constIntRef });
  constIntRefs.emplace(constIntRef);
}

void Pkb::addIf(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  ifTable.insertRow({ stmtNumIntRef });
  ifIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addWhile(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  whileTable.insertRow({ stmtNumIntRef });
  whileIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addRead(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  readTable.insertRow({ stmtNumIntRef });
  readIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addPrint(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  printTable.insertRow({ stmtNumIntRef });
  printIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addAssign(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  assignTable.insertRow({ stmtNumIntRef });
  assignIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addCall(const int stmtNum) {
  addStmt(stmtNum);
  const int stmtNumIntRef = addEntity(std::to_string(stmtNum));
  callTable.insertRow({ stmtNumIntRef });
  callIntRefs.emplace(stmtNumIntRef);
}

void Pkb::addFollows(const int followed, const int follower) {
  assert(followed < follower);
  followsTable.insertRow({ addEntity(std::to_string(followed)), addEntity(std::to_string(follower)) });
}

void Pkb::addFollowsT(const int followed, const int follower) {
  assert(followed < follower);
  followsTTable.insertRow({ addEntity(std::to_string(followed)), addEntity(std::to_string(follower)) });
}

void Pkb::addParent(const int parent, const int child) {
  assert(parent < child);
  parentTable.insertRow({ addEntity(std::to_string(parent)), addEntity(std::to_string(child)) });
}

void Pkb::addParentT(const int parent, const int child) {
  assert(parent < child);
  parentTTable.insertRow({ addEntity(std::to_string(parent)), addEntity(std::to_string(child)) });
}

void Pkb::addUsesS(const int stmtNum, const std::string& var) {
  usesSTable.insertRow({ addEntity(std::to_string(stmtNum)), addEntity(var) });
}

void Pkb::addModifiesS(const int stmtNum, const std::string& var) {
  modifiesSTable.insertRow({ addEntity(std::to_string(stmtNum)), addEntity(var) });
}

void Pkb::addModifiesP(const std::string& proc, const std::string& var) {
  modifiesPTable.insertRow({ addEntity(proc), addEntity(var) });
}

void Pkb::addUsesP(const std::string& proc, const std::string& var) {
  usesPTable.insertRow({ addEntity(proc), addEntity(var) });
}

void Pkb::addPatternAssign(const int stmtNum, const std::string& lhs, const std::string& rhs) {
  patternAssignTable.insertRow({ addEntity(std::to_string(stmtNum)), addEntity(lhs), addEntity(rhs) });
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

std::unordered_set<int> Pkb::getVarIntRefs() const { return varIntRefs; }
std::unordered_set<int> Pkb::getStmtIntRefs() const{ return stmtIntRefs; }
std::unordered_set<int> Pkb::getProcIntRefs() const{ return procIntRefs; }
std::unordered_set<int> Pkb::getConstIntRefs() const{ return constIntRefs; }
std::unordered_set<int> Pkb::getIfIntRefs() const{ return ifIntRefs; }
std::unordered_set<int> Pkb::getWhileIntRefs() const{ return whileIntRefs; }
std::unordered_set<int> Pkb::getReadIntRefs() const{ return readIntRefs; }
std::unordered_set<int> Pkb::getPrintIntRefs() const{ return printIntRefs; }
std::unordered_set<int> Pkb::getAssignIntRefs() const{ return assignIntRefs; }
std::unordered_set<int> Pkb::getCallIntRefs() const { return callIntRefs; }

int Pkb::getIntRefFromEntity(const std::string& entity) const {
  if (entityToIntRefMapper.count(entity) == 0) {
    return -1;
  }
  return entityToIntRefMapper.at(entity);
}

std::string Pkb::getEntityFromIntRef(const int intRef) const {
  assert(intRefToEntityMapper.count(intRef) == 1);
  return intRefToEntityMapper.at(intRef);
}

int Pkb::getIntRefFromStmtNum(const int stmtNum) const {
  return getIntRefFromEntity(std::to_string(stmtNum));
}

int Pkb::getStmtNumFromIntRef(const int intRef) const {
  return std::stoi(getEntityFromIntRef(intRef));
}

std::unordered_set<int> Pkb::getAssignUses(const std::string& varName) const {
  Table filterTable = usesSTable;
  filterTable.filterColumn(1, { getIntRefFromEntity(varName) });
  filterTable.innerJoin(assignTable, 0, 0);
  std::unordered_set<int> stmtNumbers;
  for (Row row : filterTable.getData()) {
    stmtNumbers.insert(getStmtNumFromIntRef(row[0]));
  }
  return stmtNumbers;
}

std::unordered_set<std::string> Pkb::getModifiedBy(const int stmtNum) const {
  Table filterTable(getModifiesSTable());
  filterTable.filterColumn(0, { getIntRefFromStmtNum(stmtNum) });
  std::unordered_set<std::string> variablesModified;
  for (const Row& row : filterTable.getData()) {
    variablesModified.insert(getEntityFromIntRef(row[1]));
  }
  return variablesModified;
}

std::string Pkb::getProcNameFromCallStmt(const int stmtNum) const {
  return getProcNameFromCallStmtIntRef(getIntRefFromStmtNum(stmtNum));
}

std::string Pkb::getProcNameFromCallStmtIntRef(const int intRef) const {
  assert(callTable.contains({ intRef }));
  return callIntRefToProcMapper.at(intRef);
}

std::string Pkb::getVarNameFromReadStmt(const int stmtNum) const {
  return getProcNameFromCallStmtIntRef(getIntRefFromStmtNum(stmtNum));
}

std::string Pkb::getVarNameFromReadStmtIntRef(const int intRef) const {
  assert(readTable.contains({ intRef }));
  return readIntRefToVarMapper.at(intRef);
}

std::string Pkb::getVarNameFromPrintStmt(const int stmtNum) const {
  return getVarNameFromPrintStmtIntRef(getIntRefFromStmtNum(stmtNum));
}

std::string Pkb::getVarNameFromPrintStmtIntRef(const int intRef) const {
  assert(printTable.contains({ intRef }));
  return printIntRefToVarMapper.at(intRef);
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

int Pkb::addEntity(const std::string& entity) {
  if (entityToIntRefMapper.count(entity) == 0) {
    const int intRef = entityToIntRefMapper.size();
    intRefToEntityMapper.emplace(intRef, entity);
    entityToIntRefMapper.emplace(entity, intRef);
  }
  return entityToIntRefMapper[entity];
}