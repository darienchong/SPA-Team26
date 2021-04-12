#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Cfg {
  struct BipNode {
    int node;
    std::vector<std::shared_ptr<BipNode>> nexts;
    std::vector<std::shared_ptr<BipNode>> affectsNexts;
  };

  struct CfgBipStartEnd {
    std::shared_ptr<BipNode> start;
    std::shared_ptr<BipNode> end;
  };

  class Cfg {
  private:
    std::unordered_map<int, std::vector<int>> adjLst;
    std::vector<std::shared_ptr<BipNode>> startNodes;

    /**
     * Creates a CFG of a target procedure.
     *
     * @param procName Procedure of interest.
     * @param procStartMapper Mapping of procedure to their respective start statements.
     */
    CfgBipStartEnd createProcedureCfg(const std::string& procName, const std::unordered_map<std::string, int>& procStartMapper);

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

    /**
     * Initialises and generates the CFGBip of all procedures.
     * 
     * @param procStartMapper Mapping of procedures to their respective start statements.
     * @param procEndMapper Mapping of procedures to their respective end statements.
     * @param callStmtToProcMapper Mapping of call statements to their respective called procedures.
     */
    void initialiseCfgBip(
      const std::list<std::string>& topoProc,
      const std::unordered_map<std::string, int>& procStartMapper,
      const std::unordered_map<std::string, std::vector<int>>& procEndMapper,
      const std::unordered_map<int, std::string>& callStmtToProcMapper);

    /**
     * Gets the list of pointers to the startNodes of relevant topo-sorted procedures in the CFGBip.
     *
     * @return std::vector<std::shared_ptr<Cfg::BipNode>> List of pointers to the startNodes of relevant topo-sorted procedures in the CFGBip.
     */
    std::vector<std::shared_ptr<BipNode>> getStartBipNodes() const;
  };
}
