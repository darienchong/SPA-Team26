#include "Cfg.h"

#include <unordered_set>
#include <unordered_map>

void Cfg::addEdge(const int from, const int to) {
  if (adjLst.count(from) == 0) {
    adjLst.emplace(from, std::unordered_set<int>{ to });
  } else {
    adjLst.at(from).insert(to);
  }
}

std::unordered_set<int> Cfg::getNeighbours(const int node) const {
  if (adjLst.count(node) == 0) {
    return {};
  }
  return adjLst.at(node);
}
