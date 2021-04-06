#include "PqlOptimizer.h"

#include <assert.h>

#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <functional>
#include <cmath>

namespace Pql {
  ClauseNode::ClauseNode()
    : index(NULL_INDEX), size(0), cost(LLONG_MAX), left(NULL_INDEX), right(NULL_INDEX) {
  };

  ClauseNode::ClauseNode(const std::vector<std::string>& header, const int index, const int size)
    : index(index), size(size), cost(0), left(NULL_INDEX), right(NULL_INDEX) {
    this->header.insert(header.begin(), header.end());
  };

  ClauseNode::ClauseNode(const ClauseNode& left, const ClauseNode& right)
    : index(JOINED_INDEX), left(left.index), right(right.index) {
    header = mergeHeaders(left.header, right.header);
    if (header.size() == left.header.size() + right.header.size()) {
      // cross product
      size = left.size * right.size;
      cost = (long long)size * (long long)header.size();
    } else {
      // inner join
      size = std::min(left.size, right.size);
      cost = (long long)left.size * (long long)left.header.size() + (long long)right.size * (long long)right.header.size();
    }
  }

  std::unordered_set<std::string> ClauseNode::mergeHeaders(const std::unordered_set<std::string>& leftHeader, const std::unordered_set<std::string>& rightHeader) {
    std::unordered_set<std::string> headerSet(leftHeader.begin(), leftHeader.end());
    headerSet.insert(rightHeader.begin(), rightHeader.end());
    return headerSet;
  }

  Optimizer::Optimizer(const std::vector<Table>& clauseTables) : clauseTables(clauseTables) {
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

    std::unordered_set<int> nonJoinedIndices;
    for (int i = 0; i < n; i++) {
      nonJoinedIndices.emplace(i);
    }

    ClauseNode globalMinCostNode = ClauseNode();
    // Initialise  base case for joining of 2 tables
    for (int i = 0; i < n - 1; i++) {
      for (int j = i + 1; j < n; j++) {
        ClauseNode joinedNode(clauseNodes[i], clauseNodes[j]);
        if (joinedNode.cost < globalMinCostNode.cost) {
          globalMinCostNode = joinedNode;
        }
      }
    }
    // Remove joined clauses from nonJoinedIndices
    nonJoinedIndices.erase(globalMinCostNode.left);
    nonJoinedIndices.erase(globalMinCostNode.right);

    // To store final result
    std::vector<int> resultOrder{ globalMinCostNode.left, globalMinCostNode.right };

    // Other cases
    for (int i = 2; i < n; i++) {
      ClauseNode localMinCostNode = globalMinCostNode;
      localMinCostNode.cost = LLONG_MAX;
      for (size_t j = 0; j < nonJoinedIndices.size(); j++) {
        std::unordered_set<int>::iterator it = nonJoinedIndices.begin();
        std::advance(it, j);
        ClauseNode joinedNode(globalMinCostNode, clauseNodes[*it]);
        if (joinedNode.cost < localMinCostNode.cost) {
          localMinCostNode = joinedNode;
        }
      }
      globalMinCostNode = localMinCostNode;
      nonJoinedIndices.erase(globalMinCostNode.right);
      resultOrder.emplace_back(globalMinCostNode.right);
    }

    assert(resultOrder.size() == n);
    return resultOrder;
  }
}