#pragma once

#include <unordered_set>
#include <unordered_map>

class Cfg {
private:
  std::unordered_map<int, std::unordered_set<int>> adjLst;

public:
  /**
   * Adds a directed edge into the CFG.
   *
   * @param from From node to be inserted.
   * @param to To node to be inserted.
   */
  void addEdge(const int from, const int to);

  /**
   * Finds the set of neighbouring nodes of the given node.
   * If there are no neighbouring nodes, an empty set is returned.
   *
   * @param node Node of interest.
   * @return Set of neighbouring nodes.
   */
  std::unordered_set<int> getNeighbours(const int node) const;
};