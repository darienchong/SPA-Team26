#pragma once

#include <string>
#include <list>

#include "Pkb.h"

class Spa {
private:
  Pkb pkb;
public:
  Spa();
  void parseSourceFile(const std::string& filename);
  void evaluateQuery(const std::string& queryString, std::list<std::string>& results);
};