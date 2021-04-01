#pragma once

#include <unordered_map>
#include <vector>

namespace Cfg {
  class Cfg {
  private:
    std::unordered_map<int, std::vector<int>> adjLst;

  public:
    /**
     * Adds a directed edge into the CFG.
     *
     * @param from From node to be inserted.
     * @param to To node to be inserted.
     */
    void addEdge(const int from, const int to);

    /**
     * Finds the list of neighbouring nodes of the given node.
     * If there are no neighbouring nodes, an empty list is returned.
     *
     * @param node Node of interest.
     * @return List of neighbouring nodes.
     */
    std::vector<int> getNeighbours(const int node) const;
  };
}
