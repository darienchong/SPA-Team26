#pragma once

#include <unordered_set>
#include <unordered_map>

class Cfg {
private:
  std::unordered_map<int, std::unordered_set<int>> adjLst;

public:

  /**
   * Adds a parent and child node into the CFG.
   *
   * @param parent Parent node to be inserted
   * @param child Child node to be inserted
   */
  void addNext(const int parent, const int child);

  /**
   * Finds the set of nodes directly following the node of interest in the CFG.
   * If there are no nodes following, will return an empty set.
   *
   * @param node Node of interest.
   * @return Unordered set of nodes
   */
  std::unordered_set<int> getNext(const int node) const;

  Cfg();
};