#include "catch.hpp"

#include "Table.h"
#include "DesignExtractor.h"

TEST_CASE("[TestDesignExtractor] ParentT extraction") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addStmt(4);

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addParent(3, 4);

  designExtractor.extractDesignAbstractions();
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
  DesignExtractor designExtractor(pkb);

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addStmt(4);

  pkb.addFollows(1, 2);
  pkb.addFollows(2, 3);
  pkb.addFollows(3, 4);

  designExtractor.extractDesignAbstractions();
  Table followsTTable = pkb.getFollowsTTable();

  REQUIRE(followsTTable.contains({ "1", "2" }));
  REQUIRE(followsTTable.contains({ "1", "3" }));
  REQUIRE(followsTTable.contains({ "1", "4" }));

  REQUIRE(followsTTable.contains({ "2", "3" }));
  REQUIRE(followsTTable.contains({ "2", "4" }));

  REQUIRE(followsTTable.contains({ "3", "4" }));
  REQUIRE(followsTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] CallsT extraction") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addProc("p1");
  pkb.addProc("p2");
  pkb.addProc("p3");
  pkb.addProc("p4");

  pkb.addCalls("p1", "p2");
  pkb.addCalls("p2", "p3");
  pkb.addCalls("p3", "p4");

  designExtractor.extractDesignAbstractions();
  Table callsTTable = pkb.getCallsTTable();

  REQUIRE(callsTTable.contains({ "p1", "p2" }));
  REQUIRE(callsTTable.contains({ "p1", "p3" }));
  REQUIRE(callsTTable.contains({ "p1", "p4" }));
  REQUIRE(callsTTable.contains({ "p2", "p3" }));
  REQUIRE(callsTTable.contains({ "p2", "p4" }));
  REQUIRE(callsTTable.contains({ "p3", "p4" }));
  REQUIRE(callsTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] Indirect Uses extraction") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addUsesS(3, "z");

  designExtractor.extractDesignAbstractions();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(usesSTable.contains({ "1", "z" }));
  REQUIRE(usesSTable.size() == 3);
}

TEST_CASE("[TestDesignExtractor] Indirect Modifies extraction") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addModifiesS(3, "z");

  designExtractor.extractDesignAbstractions();
  Table modifiesSTable = pkb.getModifiesSTable();

  REQUIRE(modifiesSTable.contains({ "1", "z" }));
  REQUIRE(modifiesSTable.size() == 3);
}

TEST_CASE("[TestDesignExtractor] Indirect UsesP extraction") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addProc("p1");
  pkb.addProc("p2");
  pkb.addProc("p3");

  pkb.addVar("usedByP2");
  pkb.addVar("usedByP3");

  pkb.addCalls("p1", "p2");
  pkb.addCalls("p2", "p3");

  pkb.addUsesP("p2", "usedByP2");
  pkb.addUsesP("p3", "usedByP3");

  designExtractor.extractDesignAbstractions();
  Table usesPTable = pkb.getUsesPTable();

  REQUIRE(usesPTable.contains({ "p1", "usedByP2" }));
  REQUIRE(usesPTable.contains({ "p1", "usedByP3" }));

  REQUIRE(usesPTable.contains({ "p2", "usedByP2" }));
  REQUIRE(usesPTable.contains({ "p2", "usedByP3" }));

  REQUIRE(usesPTable.contains({ "p3", "usedByP3" }));
  REQUIRE(usesPTable.size() == 5);
}

TEST_CASE("[TestDesignExtractor] Indirect ModifiesP extractor") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  pkb.addProc("p1");
  pkb.addProc("p2");
  pkb.addProc("p3");

  pkb.addVar("modifiedByP2");
  pkb.addVar("modifiedByP3");

  pkb.addCalls("p1", "p2");
  pkb.addCalls("p2", "p3");

  pkb.addModifiesP("p2", "modifiedByP2");
  pkb.addModifiesP("p3", "modifiedByP3");

  designExtractor.extractDesignAbstractions();
  Table usesPTable = pkb.getModifiesPTable();

  REQUIRE(usesPTable.contains({ "p1", "modifiedByP2" }));
  REQUIRE(usesPTable.contains({ "p1", "modifiedByP3" }));

  REQUIRE(usesPTable.contains({ "p2", "modifiedByP2" }));
  REQUIRE(usesPTable.contains({ "p2", "modifiedByP3" }));

  REQUIRE(usesPTable.contains({ "p3", "modifiedByP3" }));
  REQUIRE(usesPTable.size() == 5);
}
