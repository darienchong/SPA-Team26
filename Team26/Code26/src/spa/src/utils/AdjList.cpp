#include <assert.h>

#include <map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <list>
#include <vector>
#include <deque>
#include <queue>

#include "AdjList.h"

namespace {
  /**
   * Helper function to return the neighbours of a given node.
   */
  std::unordered_set<int> getNeighboursOf(int node, const std::map<int, std::unordered_set<int>>& internalRepresentation) {
    bool nodeHasEntryInInternalRepresentation = internalRepresentation.count(node) > 0;
    if (nodeHasEntryInInternalRepresentation) {
      return internalRepresentation.at(node);
    } else {
      return std::unordered_set<int>();
    }
  }

  /**
   * Helper function for DFS topological sort.
   * See https://en.wikipedia.org/wiki/Topological_sorting
   */
  void visit(int node,
    std::vector<bool>& hasPermanentMark,
    std::vector<bool>& hasTemporaryMark,
    std::list<int>& resultList,
    const std::map<int, std::unordered_set<int>>& internalRepresentation) {
    if (hasPermanentMark[node]) {
      return;
    }

    assert(!hasTemporaryMark[node]); // Cycle detection

    hasTemporaryMark[node] = true;

    std::unordered_set<int> neighboursOfNode = getNeighboursOf(node, internalRepresentation);
    for (int neighbour : neighboursOfNode) {
      visit(neighbour, hasPermanentMark, hasTemporaryMark, resultList, internalRepresentation);
    }

    hasTemporaryMark[node] = false;
    hasPermanentMark[node] = true;
    resultList.push_front(node);
  }
}

void AdjList::insert(const int i, const int j) {
  if (i > size || j > size) {
    throw std::invalid_argument("[AdjList::insert] "
      + INDEX_GREATER_THAN_SIZE_ERROR_MSG);
  }

  const bool isFirstTimePlacingIntoAdjList = (internalRepresentation.count(i) == 0);
  if (isFirstTimePlacingIntoAdjList) {
    std::unordered_set<int> temp{ j };
    internalRepresentation.emplace(i, temp);
  } else {
    internalRepresentation.at(i).insert(j);
  }
}

bool AdjList::get(const int i, const int j) {
  const bool isKeyNotInAdjList = (internalRepresentation.count(i) == 0);
  if (isKeyNotInAdjList) {
    return false;
  }

  std::unordered_set<int> setOfPossibleValues = internalRepresentation.at(i);
  const bool isValueNotInSet = (setOfPossibleValues.count(j) == 0);
  if (isValueNotInSet) {
    return false;
  }

  return true;
}

void AdjList::warshallRowOperation(const int i, const int j) {
  // Handle the simple cases first.
  // If there is no row at j (i.e. every entry in the jth row is 0)
  // then we do nothing (since x OR 0 = x)
  // If there is no row at i (i.e. every entry in the ith row is 0)
  // then we copy over the jth row to the ith row.
  const bool isIthRowEmpty = (internalRepresentation.count(i) == 0);
  const bool isJthRowEmpty = (internalRepresentation.count(j) == 0);

  if (isJthRowEmpty) {
    return;
  }

  if (isIthRowEmpty) {
    std::unordered_set<int> copyOfJthRow = internalRepresentation.at(j);
    internalRepresentation.emplace(i, copyOfJthRow);
    return;
  }

  // Otherwise, both the ith and jth rows are non-empty.
  // We perform 'bitwise' operations (one entry at a time).
  std::unordered_set<int> newIthRow;
  for (int k = 1; k <= size; k++) {
    bool ithRowKthEntry = get(i, k);
    bool jthRowKthEntry = get(j, k);
    bool bitwiseOr = ithRowKthEntry || jthRowKthEntry;
    if (bitwiseOr) {
      newIthRow.insert(k);
    }
  }
  internalRepresentation.erase(i);
  internalRepresentation.emplace(i, newIthRow);
}

void AdjList::applyWarshallAlgorithm() {
  for (int j = 1; j <= size; j++) {
    for (int i = 1; i <= size; i++) {
      if (get(i, j)) {
        warshallRowOperation(i, j);
      }
    }
  }
}

std::list<int> AdjList::topologicalOrder() {
  std::vector<int> inDegree(size + 1, 0);
  std::list<int> toReturn;
  std::deque<int> tempQueue;

  // Compute the in-degrees of all the nodes
  for (int i = 1; i <= size; i++) {
    for (int j = 1; j <= size; j++) {
      bool edgeToIExists = this->get(j, i);
      if (edgeToIExists) {
        inDegree[i] = inDegree[i] + 1;
      }
    }
  }

  // Put all vertices with in-degree 0 into the queue.
  for (int i = 1; i <= size; i++) {
    if (inDegree[i] == 0) {
      tempQueue.push_back(i);
    }
  }

  while (tempQueue.size() > 0) {
    int node = tempQueue.front();
    tempQueue.pop_front();

    toReturn.push_back(node);

    // Recompute all the in-degrees of the neighbours of node
    for (int j = 1; j <= size; j++) {
      bool edgeFromNodeToJExists = this->get(node, j);
      if (edgeFromNodeToJExists) {
        inDegree[j] = inDegree[j] - 1;
      }
    }

    for (int k = 1; k <= size; k++) {
      bool isNeighbourOfNode = this->get(node, k);
      if (inDegree[k] == 0 && isNeighbourOfNode) {
        tempQueue.push_back(k);
      }
    }
  }

  return toReturn;
}

std::list<int> AdjList::stableTopologicalOrder() {
  std::list<int> toReturn;
  std::vector<bool> hasTemporaryMark(size + 1, false);
  std::vector<bool> hasPermanentMark(size + 1, false);
  std::priority_queue<int, std::vector<int>, std::greater<int>> tieBreaker;
  for (int i = 1; i <= size; i++) {
    tieBreaker.emplace(i);
  }

  while (!tieBreaker.empty()) {
    int selectedNode = tieBreaker.top();
    tieBreaker.pop();

    bool isUnmarked = !(hasTemporaryMark[selectedNode] || hasPermanentMark[selectedNode]);
    if (isUnmarked) {
      visit(selectedNode, hasPermanentMark, hasTemporaryMark, toReturn, internalRepresentation);
    }
  }

  return toReturn;
}

std::list<std::list<int>> AdjList::getAllConnectedComponents() {
  // Algorithm taken from http://math.hws.edu/eck/cs327_s04/chapter9.pdf
  std::list<std::list<int>> connectedComponents;
  std::vector<bool> isVisited(size + 1, false);

  for (int j = 1; j <= size; j++) {
    std::list<int> connectedComponent;
    if (!isVisited[j]) {
      std::deque<int> queue;
      queue.push_back(j);
      isVisited[j] = true;
      while (!(queue.size() == 0)) {
        int nodeInConnectedComponent = queue.front();
        queue.pop_front();

        connectedComponent.emplace_back(nodeInConnectedComponent);
        if (internalRepresentation.count(nodeInConnectedComponent) == 0) {
          continue;
        }
        std::unordered_set<int> neighboursOfNode = internalRepresentation.at(nodeInConnectedComponent);
        for (int neighbouringNode : neighboursOfNode) {
          if (!isVisited[neighbouringNode]) {
            isVisited[neighbouringNode] = true;
            queue.push_back(neighbouringNode);
          }
        }
      }
    }

    if (!connectedComponent.empty()) {
      connectedComponents.push_back(connectedComponent);
    }
  }

  return connectedComponents;
}

std::string AdjList::toString() {
  std::string toReturn = "";
  for (int i = 1; i <= size; i++) {
    for (int j = 1; j <= size; j++) {
      if (this->get(i, j)) {
        toReturn += " 1 ";
      } else {
        toReturn += " 0 ";
      }
    }
    toReturn += "\n";
  }

  return toReturn;
}
