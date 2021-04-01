#pragma once

#include <unordered_map>
#include <vector>

namespace Cfg {
  enum class NodeType {
    // default
    NORMAL,

    // dummy
    DUMMY,

    // call stmt
    BRANCH_IN,
    BRANCH_BACK
  };

  struct BipNode {
    int node, label;
    NodeType type;
  };

  class CfgBip {
  private:
    std::unordered_map<int, std::vector<BipNode>> adjLst;

  public:
    /**
     * Adds a directed edge into the CFGBip.
     *
     * @param from From node to be inserted.
     * @param to To node to be inserted.
     * @param label Label of the edge.
     * @param link Link type of the edge.
     */
    void addBipEdge(const int from, const int to, const int label, NodeType type);

    /**
     * Finds the list of neighbouring nodes of the given node in the CFGBip.
     * If there are no neighbouring nodes, an empty list is returned.
     *
     * @param node Node of interest.
     * @return List of neighbouring nodes.
     */
    std::vector<BipNode> getNeighbours(const int node) const;
  };
}
