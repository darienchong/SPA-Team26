#pragma once

#include "PKB.h"

class DesignExtractor {
private:
  PKB pkb;
public:
  DesignExtractor(PKB pkb);
  ~DesignExtractor();

  void extractDesignAbstractions();
};
