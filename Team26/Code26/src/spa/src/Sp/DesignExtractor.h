#pragma once

#include "Pkb.h"

class DesignExtractor {
private:
  Pkb& pkb;
public:
  DesignExtractor(Pkb& pkb) : pkb(pkb) {}
  ~DesignExtractor() {}

  void extractDesignAbstractions();
};
