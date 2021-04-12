#pragma once

#include <list>
#include <string>

#include "Pkb.h"

namespace SourceProcessor {
  class DesignExtractor {
  private:
    Pkb& pkb;
    std::list<std::string> topoSortedProcs;
    std::list<std::string> reverseTopoSortedProcs;

  public:
    DesignExtractor(Pkb& pkb);

    void extractDesignAbstractions();
    void extractIter3DesignAbstractions();
    void extractAllDesignAbstractions();
  };
}