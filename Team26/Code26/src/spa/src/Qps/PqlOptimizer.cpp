#include "PqlOptimizer.h"

#include <assert.h>

#include <string>
#include <unordered_set>
#include <vector>

namespace Pql {
  ClauseNode::ClauseNode()
    : index(NULL_INDEX), size(0), cost(ULLONG_MAX), left(NULL_INDEX), right(NULL_INDEX) {
  };

  ClauseNode::ClauseNode(const std::vector<std::string>& header, const int index, const size_t size)
    : index(index), size(size), cost(0), left(NULL_INDEX), right(NULL_INDEX) {
    this->header.insert(header.begin(), header.end());
  };

  ClauseNode::ClauseNode(const ClauseNode& left, const ClauseNode& right)
    : index(JOINED_INDEX), left(left.index), right(right.index) {
    header = mergeHeaders(left.header, right.header);
    if (header.size() == left.header.size() + right.header.size()) {
      // cross product
      size = left.size * right.size;
      cost = (unsigned long long)size * (unsigned long long)header.size();
    } else {
      // inner join
      size = left.size + right.size;
      cost = (unsigned long long)left.size * (unsigned long long)left.header.size()
        + (unsigned long long)right.size * (unsigned long long)right.header.size();
    }
  }

  std::unordered_set<std::string> ClauseNode::mergeHeaders(const std::unordered_set<std::string>& leftHeader, const std::unordered_set<std::string>& rightHeader) {
    std::unordered_set<std::string> headerSet(leftHeader.begin(), leftHeader.end());
    headerSet.insert(rightHeader.begin(), rightHeader.end());
    return headerSet;
  }

  Optimizer::Optimizer(std::vector<Table>& clauseTables) : clauseTables(clauseTables) {
    assert(!clauseTables.empty());
  }

  std::vector<int> Optimizer::getOptimizedOrder() {
    const int n = clauseTables.size();
    if (n == 1) {
      return { 0 };
    }

    // Constructs and add clause nodes to a vector
    std::vector<ClauseNode> clauseNodes;
    clauseNodes.reserve(n);
    for (int i = 0; i < n; i++) {
      clauseNodes.emplace_back(ClauseNode(clauseTables[i].getHeader(), i, clauseTables[i].size()));
    }

    std::unordered_set<int> nonJoinedIndexes;
    for (int i = 0; i < n; i++) {
      nonJoinedIndexes.emplace(i);
    }

    ClauseNode globalMinCostNode;
    // Initialise  base case for joining of 2 tables
    for (int i = 0; i < n - 1; i++) {
      for (int j = i + 1; j < n; j++) {
        ClauseNode joinedNode(clauseNodes[i], clauseNodes[j]);
        if (joinedNode.cost < globalMinCostNode.cost) {
          globalMinCostNode = joinedNode;
        }
      }
    }
    // Remove joined clauses from nonJoinedIndexes
    nonJoinedIndexes.erase(globalMinCostNode.left);
    nonJoinedIndexes.erase(globalMinCostNode.right);

    // To store final result
    std::vector<int> resultOrder;
    resultOrder.reserve(n);
    resultOrder.emplace_back(globalMinCostNode.left);
    resultOrder.emplace_back(globalMinCostNode.right);

    // Other cases
    for (int i = 2; i < n; i++) {
      ClauseNode localMinCostNode = globalMinCostNode;
      localMinCostNode.cost = ULLONG_MAX;
      for (const int nonJoinedIndex : nonJoinedIndexes) {
        ClauseNode joinedNode(globalMinCostNode, clauseNodes[nonJoinedIndex]);
        if (joinedNode.cost < localMinCostNode.cost) {
          localMinCostNode = joinedNode;
        }
      }
      globalMinCostNode = localMinCostNode;
      nonJoinedIndexes.erase(globalMinCostNode.right);
      resultOrder.emplace_back(globalMinCostNode.right);
    }

    assert(resultOrder.size() == n);
    return resultOrder;
  }
}