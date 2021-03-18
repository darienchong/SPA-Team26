#include "Cfg.h"

#include <unordered_set>
#include <unordered_map>

Cfg::Cfg() = default;

void Cfg::addNext(const int parent, const int child) {
  if (adjLst.count(parent) == 0) {
    adjLst.emplace(parent, std::unordered_set<int>{ child });
  } else {
    adjLst.at(parent).insert(child);
  }
}

std::unordered_set<int> Cfg::getNext(const int node) const {
  if (adjLst.count(node) == 0) {
    return {};
  }
  return adjLst.at(node);
}
