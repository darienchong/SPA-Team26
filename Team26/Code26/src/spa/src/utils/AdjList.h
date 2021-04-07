#pragma once

#include <map>
#include <list>
#include <unordered_set>

class AdjList {
private:
  // The internal representation of the adjacency list.
  std::map<int, std::unordered_set<int>> internalRepresentation;

  // Number of nodes in the graph.
  int size;

  /**
   * Helper method for implementation of Warshall's algorithm.
   * Performs row-wise a_i := a_i OR a_j.
   *
   * @param i The number to use for index i.
   * @param j The number to use for index j.
   * @returns none
   */
  void warshallRowOperation(int i, int j);

  const std::string INDEX_GREATER_THAN_SIZE_ERROR_MSG = "One of i, j is"
    " greater than the size allocated to this adjacency list.";
public:
  AdjList(int size) : size(size) {}
  ~AdjList() {}

  /**
   * Records a directed edge into the adjacency list i.e.
   * adds E(i,j) into the adjacency list.
   * Guarantees worst case O(size) performance.
   *
   * @param i
   * @param j
   * @returns none
   */
  void insert(int i, int j);

  /**
   * Retrives the value of a given entry.
   * Guarantees worst-case O(size) performance.
   *
   * @param i
   * @param j
   * @returns `true` if E(i, j) holds, `false` otherwise.
   */
  bool get(int i, int j);

  /**
   * Applies Warshall's algorithm
   * (see https://www.dartmouth.edu/~matc/DiscreteMath/V.6.pdf) to the
   * current adjacency list. Complexity: O(size * size)
   * 
   * @returns
   */
  void applyWarshallAlgorithm();

  /**
   * Returns an ordered list of nodes in topological order.
   *
   * @returns Nodes in topological order.
   */
  std::list<int> topologicalOrder();

  /**
   * Returns an ordered list of nodes in topological order 
   * where ties are broken by taking the node with the smallest
   * index.
   * 
   * @returns Nodes in topological order, where ties are broken by
   *   taking the node with the smallest index.
   */
  std::list<int> stableTopologicalOrder();

  /**
   * Returns a list of connected components.
   * 
   * @returns
   */
  std::list<std::list<int>> getAllConnectedComponents();

  /**
   * Returns a string representation of the graph
   * as adjacency matrix.
   *
   * @returns String representation of graph as adjacency list.
   */
  std::string toString();
};
