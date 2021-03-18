#pragma once

#include "Pkb.h"

namespace SourceProcessor {
  class DesignExtractor {
  private:
    Pkb& pkb;
  public:
    DesignExtractor(Pkb& pkb) : pkb(pkb) {}
    ~DesignExtractor() {}

    void extractDesignAbstractions();
  };
}