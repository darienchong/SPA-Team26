#include "CfgBip.h"

#include <unordered_map>
#include <vector>

namespace Cfg {
  void CfgBip::addBipEdge(const int from, const int to, const int label, const NodeType type) {
    const BipNode node = { to, label, type };
    if (adjLst.count(from) == 0) {
      adjLst.emplace(from, std::vector<BipNode>{ node });
    }
    else {
      adjLst.at(from).emplace_back(node);
    }
  }

  std::vector<BipNode> CfgBip::getNeighbours(const int node) const {
    if (adjLst.count(node) == 0) {
      return {};
    }
    return adjLst.at(node);
  }
}
