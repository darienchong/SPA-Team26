#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <functional>

#include "Table.h"

namespace Pql {
  static const int JOINED_INDEX = -1;
  static const int NULL_INDEX = -2;

  struct ClauseNode {
    ClauseNode();
    ClauseNode(const std::vector<std::string>& header, const int index, const int size);
    ClauseNode(const ClauseNode& left, const ClauseNode& right);

    static std::unordered_set<std::string> mergeHeaders(const std::unordered_set<std::string>& leftHeader, const std::unordered_set<std::string>& rightHeader);
    std::unordered_set<std::string> header;
    int index;
    int size;
    long long cost;

    int left;
    int right;
  };

  class Optimizer {
  private:
    std::vector<Table> clauseTables;

  public:
    Optimizer(const std::vector<Table>& clauseTables);
    std::vector<int> getOptimizedOrder();
  };
}
