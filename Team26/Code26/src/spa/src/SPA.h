#pragma once
#include <string>
#include <list>
#include "PKB.h"

class SPA {
private:
  PKB pkb;
public:
  SPA();
  void parseSourceFile(const std::string& filename);
  void evaluateQuery(const std::string& query, std::list<std::string>& results);
};