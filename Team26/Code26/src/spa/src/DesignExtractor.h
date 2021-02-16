#pragma once

#include "Pkb.h"

class DesignExtractor {
private:
  Pkb pkb;
public:
  DesignExtractor(Pkb pkb);
  ~DesignExtractor();

  void extractDesignAbstractions();
};
