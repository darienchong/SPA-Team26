#include "catch.hpp"

#include "Table.h"
#include "DesignExtractor.h"

TEST_CASE("[TestDesignExtractor] Sanity check") {
  REQUIRE(1 == 1);
}

TEST_CASE("[TestDesignExtractor] ParentT extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

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

TEST_CASE("[TestDesignExtractor] Uses extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addParent(1, 2);
  pkb.addUses(2, "z");

  designExtractor.extractDesignAbstractions(pkb);
  Table usesTable = pkb.getUsesTable();

  REQUIRE(usesTable.contains({ 1, "z" });
  REQUIRE(usesTable.size() == 2);

}

TEST_CASE("[TestDesignExtractor] Modifies extraction") {
  Pkb pkb;
  DesignExtractor designExtractor;

  pkb.addParent(1, 2);
  pkb.addModifies(2, "z");

  designExtractor.extractDesignAbstractions(pkb);
  Table modifiesTable = pkb.getModifiesTable();

  REQUIRE(modifiesTable.contains({ 1, "z" });
  REQUIRE(modifiesTable.size() == 2);
}

