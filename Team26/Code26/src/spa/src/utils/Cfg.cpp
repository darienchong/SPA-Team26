#include "Cfg.h"

#include <assert.h>

#include <memory>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Cfg {
  void Cfg::addEdge(const int from, const int to) {
    if (adjLst.count(from) == 0) {
      adjLst.emplace(from, std::vector<int>{ to });
    } else {
      adjLst.at(from).emplace_back(to);
    }
  }

  std::vector<int> Cfg::getNeighbours(const int node) const {
    if (adjLst.count(node) == 0) {
      return {};
    }
    return adjLst.at(node);
  }

  void Cfg::initialiseCfgBip(
    const std::list<std::string>& topoProc,
    const std::unordered_map<std::string, int>& procStartMapper,
    const std::unordered_map<std::string, std::vector<int>>& procEndMapper,
    const std::unordered_map<int, std::string>& callStmtToProcMapper) {

    // For each procedure, generate the graph and store the procStartNode
    std::unordered_set<std::string> visitedProcs;

    for (const std::string& procName : topoProc) {
      if (visitedProcs.count(procName) == 1) {
        continue;
      }
      visitedProcs.emplace(procName);

      CfgBipStartEnd& procStartEnd = createProcedureCfg(procName, procStartMapper);
      std::shared_ptr<BipNode>& startPtr = procStartEnd.start;
      startNodes.emplace_back(startPtr); // store the procStartNodes

      std::unordered_set<std::shared_ptr<BipNode>> visited;
      std::stack<std::shared_ptr<BipNode>> stack;
      stack.push(startPtr); // push startNode to stack
      visited.emplace(startPtr); // mark startNode as visited

      while (!stack.empty()) {
        std::shared_ptr<BipNode>& currPtr = stack.top();
        const int node = currPtr->node;
        stack.pop();

        // If currNode is a call statement - generate procedure CFG for the called proc
        if (callStmtToProcMapper.count(node) == 1) {
          const std::string calledProc = callStmtToProcMapper.at(node);
          CfgBipStartEnd& calledProcStartEnd = createProcedureCfg(calledProc, procStartMapper);
          visitedProcs.emplace(calledProc);

          // Add the ptrs from the called proc's end to currNode's nextNodes
          assert(currPtr->nexts.size() == 1);
          std::shared_ptr<BipNode>& nextNode = currPtr->nexts[0];
          calledProcStartEnd.end->nexts.emplace_back(nextNode);

          // Add the ptrs from curr to called proc's start
          currPtr->nexts.clear();
          currPtr->nexts.emplace_back(calledProcStartEnd.start);
        }

        // Add nextNodes to the stack under certain conditions
        for (std::shared_ptr<BipNode>& nextPtr : currPtr->nexts) {
          // Add nextNode to stack only if not visited
          if (visited.count(nextPtr) == 0) {
            stack.push(nextPtr);
            visited.emplace(nextPtr); // mark nextNode as visited
          }
        }
      }
    }
  }

  CfgBipStartEnd Cfg::createProcedureCfg(const std::string& procName, const std::unordered_map<std::string, int>& procStartMapper) {
    std::unordered_map<int, std::shared_ptr<BipNode>> valueToNode;
    std::unordered_set<int> visited;
    std::stack<int> stack;

    const int startValue = procStartMapper.at(procName);
    std::shared_ptr<BipNode> startNode(new BipNode{ startValue });
    valueToNode.emplace(startValue, startNode);
    stack.push(startValue);
    visited.emplace(startValue);
    while (!stack.empty()) {
      const int currentValue = stack.top();
      std::shared_ptr<BipNode> currentNode = valueToNode.at(currentValue);
      stack.pop();


      // Check if there are neighbours
      if (adjLst.count(currentValue) == 0) {
        continue;
      }

      // Push all unvisited neighbours into stack
      const std::vector<int>& neighbours = adjLst.at(currentValue);
      for (const int neighbourValue : neighbours) {
        if (valueToNode.count(neighbourValue) == 0) { // Node not yet created
          valueToNode.emplace(neighbourValue, new BipNode{ neighbourValue });
        }

        // Add to currentNode.nexts
        const std::shared_ptr<BipNode>& neighbourNode = valueToNode.at(neighbourValue);
        currentNode->nexts.emplace_back(neighbourNode);

        if (visited.count(neighbourValue) == 0) {
          stack.push(neighbourValue);
          visited.emplace(neighbourValue);
        }

      }
    }

    const std::shared_ptr<BipNode>& endNode = valueToNode.at(startValue * -1);
    return CfgBipStartEnd{ startNode, endNode };
  }

  std::vector<std::shared_ptr<BipNode>> Cfg::getStartBipNodes() const {
    return startNodes;
  }
}
