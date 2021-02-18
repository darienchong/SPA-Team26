#pragma once

#include "Pkb.h"

class DesignExtractor {
private:
public:
  DesignExtractor();
  ~DesignExtractor();

  void extractDesignAbstractions(Pkb& pkb);
};
