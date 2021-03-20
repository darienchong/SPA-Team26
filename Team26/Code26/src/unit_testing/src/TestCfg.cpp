#include "catch.hpp"

#include <unordered_set>

#include "Cfg.h"


TEST_CASE("Adding edges and getting neighbours", "[CFG]") {
  Cfg cfg;
  cfg.addEdge(1, 2);
  cfg.addEdge(1, 3);
  cfg.addEdge(1, 4);
  cfg.addEdge(4, 5);

  std::unordered_set<int> neighbours1 = cfg.getNeighbours(1);
  std::unordered_set<int> neighbours4 = cfg.getNeighbours(4);
  REQUIRE(neighbours1.count(2) == 1);
  REQUIRE(neighbours1.count(3) == 1);
  REQUIRE(neighbours1.count(4) == 1);
  REQUIRE(neighbours1.size() == 3);
  REQUIRE(cfg.getNeighbours(2).size() == 0);
  REQUIRE(cfg.getNeighbours(3).size() == 0);
  REQUIRE(neighbours4.count(5) == 1);
  REQUIRE(neighbours4.size() == 1);
  REQUIRE(cfg.getNeighbours(5).size() == 0);
  REQUIRE(cfg.getNeighbours(6).size() == 0);
}