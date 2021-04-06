#include <map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <list>
#include <vector>
#include <deque>

#include "AdjList.h"

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

std::list<std::list<int>> AdjList::getAllConnectedComponents() {
  // Algorithm taken from http://math.hws.edu/eck/cs327_s04/chapter9.pdf
  std::list<std::list<int>> connectedComponents;
  std::vector<bool> isVisited(size + 1);
  for (int i = 1; i <= size; i++) {
    isVisited[i] = false;
  }

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
        std::unordered_set<int> neighboursOfNode = internalRepresentation.at(nodeInConnectedComponent);
        for (int neighbouringNode : neighboursOfNode) {
          if (!isVisited[neighbouringNode]) {
            isVisited[neighbouringNode] = true;
            queue.push_back(neighbouringNode);
          }
        }
      }
    }

    connectedComponents.push_back(connectedComponent);
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
