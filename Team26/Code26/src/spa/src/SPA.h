#pragma once

#include <string>
#include <list>

#include "PKB.h"

/**
 * Static Program Analyzer (SPA) library API
*/
class SPA {
private:
  PKB pkb;
public:
  SPA();
  void parseSourceFile(const std::string& filename);
  void evaluateQuery(const std::string& queryString, std::list<std::string>& results);
};