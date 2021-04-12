#include "catch.hpp"

#include <vector>

#include "Cfg.h"

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
