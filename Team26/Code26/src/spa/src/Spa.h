#pragma once

#include <string>
#include <list>

#include "Pkb.h"

class Spa {
/**
 * Static Program Analyzer (SPA) library API
*/
private:
  Pkb pkb;
public:
  Spa();
  void parseSourceFile(const std::string& filename);
  void evaluateQuery(const std::string& queryString, std::list<std::string>& results);
};