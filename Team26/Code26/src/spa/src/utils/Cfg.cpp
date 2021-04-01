#include "Cfg.h"

#include <unordered_map>
#include <vector>

namespace Cfg {
  void Cfg::addEdge(const int from, const int to) {
    if (adjLst.count(from) == 0) {
      adjLst.emplace(from, std::vector<int>{ to });
    }
    else {
      adjLst.at(from).emplace_back(to);
    }
  }

  std::vector<int> Cfg::getNeighbours(const int node) const {
    if (adjLst.count(node) == 0) {
      return {};
    }
    return adjLst.at(node);
  }
}
