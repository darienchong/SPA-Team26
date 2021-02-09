#pragma once

#include <string>

class PkbStub {
public:
  PkbStub();
  ~PkbStub();

  // entities
  void addProc(std::string procName) {};
  void addAssign(int stmt);
  void addPrint(int stmt);
  void addRead(int stmt);
  void addIf(int stmt);
  void addWhile(int stmt);

  // relations
  void addParent(int parent, int child);
  void addFollowsT(int before, int after);
  void addUses(int stmt, std::string var);
  void addModifies(int stmt, std::string var);
};