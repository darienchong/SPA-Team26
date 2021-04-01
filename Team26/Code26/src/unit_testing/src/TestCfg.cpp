#include "catch.hpp"

#include <vector>

#include "Cfg.h"
#include "CfgBip.h"

TEST_CASE("CFG", "[CFG]") {
  Cfg::Cfg cfg;
  cfg.addEdge(1, 2);
  cfg.addEdge(1, 3);
  cfg.addEdge(1, 4);
  cfg.addEdge(4, 5);

  std::vector<int> neighbours1 = cfg.getNeighbours(1);
  std::vector<int> neighbours4 = cfg.getNeighbours(4);
  REQUIRE(std::find(neighbours1.begin(), neighbours1.end(), 2) != neighbours1.end());
  REQUIRE(std::find(neighbours1.begin(), neighbours1.end(), 3) != neighbours1.end());
  REQUIRE(std::find(neighbours1.begin(), neighbours1.end(), 4) != neighbours1.end());
  REQUIRE(neighbours1.size() == 3);
  REQUIRE(cfg.getNeighbours(2).size() == 0);
  REQUIRE(cfg.getNeighbours(3).size() == 0);
  REQUIRE(std::find(neighbours4.begin(), neighbours4.end(), 5) != neighbours4.end());
  REQUIRE(neighbours4.size() == 1);
  REQUIRE(cfg.getNeighbours(5).size() == 0);
  REQUIRE(cfg.getNeighbours(6).size() == 0);
}

TEST_CASE("CFGBip", "[CFG]") {
  Cfg::CfgBip cfg;
  cfg.addBipEdge(1, 2, 0, Cfg::NodeType::NORMAL);
  cfg.addBipEdge(1, 3, 1, Cfg::NodeType::DUMMY);
  cfg.addBipEdge(1, 4, 7, Cfg::NodeType::BRANCH_IN);
  cfg.addBipEdge(4, 5, 0, Cfg::NodeType::BRANCH_BACK);

  std::vector<Cfg::BipNode> neighbours1 = cfg.getNeighbours(1);
  std::vector<Cfg::BipNode> neighbours4 = cfg.getNeighbours(4);
  Cfg::BipNode edge1 = neighbours1.at(0);
  Cfg::BipNode edge2 = neighbours1.at(1);
  Cfg::BipNode edge3 = neighbours1.at(2);
  REQUIRE(edge1.node == 2);
  REQUIRE(edge1.label == 0);
  REQUIRE(edge1.type == Cfg::NodeType::NORMAL);
  REQUIRE(edge2.node == 3);
  REQUIRE(edge2.label == 1);
  REQUIRE(edge2.type == Cfg::NodeType::DUMMY);
  REQUIRE(edge3.node == 4);
  REQUIRE(edge3.label == 7);
  REQUIRE(edge3.type == Cfg::NodeType::BRANCH_IN);
  REQUIRE(neighbours1.size() == 3);
  REQUIRE(cfg.getNeighbours(2).size() == 0);
  REQUIRE(cfg.getNeighbours(3).size() == 0);
  REQUIRE(neighbours4.at(0).node == 5);
  REQUIRE(neighbours4.at(0).label == 0);
  REQUIRE(neighbours4.size() == 1);
  REQUIRE(cfg.getNeighbours(5).size() == 0);
  REQUIRE(cfg.getNeighbours(6).size() == 0);
}
