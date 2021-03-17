#include "Cfg.h"

#include <unordered_set>
#include <unordered_map>

Cfg::Cfg() = default;

void Cfg::addNext(const int parent, const int child) {
  auto search = adjLst.find(parent);
  if (search == adjLst.end()) {
    adjLst.emplace(parent, std::unordered_set<int>{child});
  }
  else {
    search->second.insert(child);
  }
}

std::unordered_set<int> Cfg::getNext(const int node) const {
  auto search = adjLst.find(node);
  if (search == adjLst.end()) {
    return std::unordered_set<int>();
  }
  return search->second;
}
