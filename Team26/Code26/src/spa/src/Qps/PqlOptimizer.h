#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Table.h"

namespace Pql {
  static const int JOINED_INDEX = -1;
  static const int NULL_INDEX = -2;

  /**
   * Clause table representation as a Tree node.
   */
  struct ClauseNode {
    ClauseNode();
    ClauseNode(const std::vector<std::string>& header, const int index, const size_t size);
    ClauseNode(const ClauseNode& left, const ClauseNode& right);

    /**
     * Calculates and returns the merged header of the given two headers.
     * 
     * @param leftHeader Left header.
     * @param rightHeader Right header.
     * @return Resulting merged tables.
    */
    static std::unordered_set<std::string> mergeHeaders(const std::unordered_set<std::string>& leftHeader, const std::unordered_set<std::string>& rightHeader);
    std::unordered_set<std::string> header;
    int index;
    size_t size;
    unsigned long long cost;

    int left;
    int right;
  };

  /**
   * PQL Optimizer that calculates the best plan to join the clause tables.
   */
  class Optimizer {
  private:
    std::vector<Table>& clauseTables;

  public:
    /**
     * Constructs an Optimizer with the given clause tables.
     * 
     * @param clauseTables Clause tables.
     */
    Optimizer(std::vector<Table>& clauseTables);

    /**
     * Calculates the best order to join the clause tables an returns the index order of the clause tables.
     * 
     * @return Indices of the clause tables sorted in order to for joining.
     */
    std::vector<int> getOptimizedOrder();
  };
}
