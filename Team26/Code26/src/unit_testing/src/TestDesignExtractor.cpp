#include "catch.hpp"

#include "Table.h"
#include "DesignExtractor.h"

TEST_CASE("[TestDesignExtractor] ParentT extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addStmt(4);

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addParent(3, 4);

  designExtractor.extractDesignAbstractions(pkb);
  Table parentTTable = pkb.getParentTTable();

  REQUIRE(parentTTable.contains({ "1", "2" }));
  REQUIRE(parentTTable.contains({ "1", "3" }));
  REQUIRE(parentTTable.contains({ "1", "4" }));

  REQUIRE(parentTTable.contains({ "2", "3" }));
  REQUIRE(parentTTable.contains({ "2", "4" }));

  REQUIRE(parentTTable.contains({ "3", "4" }));
  REQUIRE(parentTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] FollowsT extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addStmt(4);

  pkb.addFollows(1, 2);
  pkb.addFollows(2, 3);
  pkb.addFollows(3, 4);

  designExtractor.extractDesignAbstractions(pkb);
  Table followsTTable = pkb.getFollowsTTable();

  REQUIRE(followsTTable.contains({ "1", "2" }));
  REQUIRE(followsTTable.contains({ "1", "3" }));
  REQUIRE(followsTTable.contains({ "1", "4" }));

  REQUIRE(followsTTable.contains({ "2", "3" }));
  REQUIRE(followsTTable.contains({ "2", "4" }));

  REQUIRE(followsTTable.contains({ "3", "4" }));
  REQUIRE(followsTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] Indirect Uses extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addUsesS(3, "z");

  designExtractor.extractDesignAbstractions(pkb);
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(usesSTable.contains({ "1", "z" }));
  REQUIRE(usesSTable.size() == 3);
}

TEST_CASE("[TestDesignExtractor] Indirect Modifies extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addModifiesS(3, "z");

  designExtractor.extractDesignAbstractions(pkb);
  Table modifiesSTable = pkb.getModifiesSTable();

  REQUIRE(modifiesSTable.contains({ "1", "z" }));
  REQUIRE(modifiesSTable.size() == 3);
}

