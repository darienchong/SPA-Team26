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

TEST_CASE("[TestDesignExtractor] Affects extractor") {
  Pkb pkb;
  DesignExtractor designExtractor(pkb);

  // Example Code 5 in Advanced SPA requirements
  pkb.addProc("second");
  pkb.addProc("third");

  // adding statements
  pkb.addAssign(1);
  pkb.addAssign(2);
  pkb.addWhile(3);
  pkb.addAssign(4);
  pkb.addCall(5);
  pkb.addAssign(6);
  pkb.addIf(7);
  pkb.addAssign(8);
  pkb.addAssign(9);
  pkb.addAssign(10);
  pkb.addAssign(11);
  pkb.addAssign(12);
  pkb.addAssign(13);
  pkb.addAssign(14);

  // adding to cfg
  pkb.addCfgLink(1, 2);
  pkb.addCfgLink(2, 3);
  pkb.addCfgLink(3, 4);
  pkb.addCfgLink(4, 5);
  pkb.addCfgLink(5, 6);
  pkb.addCfgLink(6, 3);
  pkb.addCfgLink(3, 7);
  pkb.addCfgLink(7, 8);
  pkb.addCfgLink(7, 9);
  pkb.addCfgLink(8, 10);
  pkb.addCfgLink(9, 10);
  pkb.addCfgLink(10, 11);
  pkb.addCfgLink(11, 12);
  pkb.addCfgLink(13, 14);

  // adding usesS and modifiesS
  pkb.addUsesS(4, "x");
  pkb.addUsesS(4, "y");
  pkb.addUsesS(6, "i");
  pkb.addUsesS(8, "x");
  pkb.addUsesS(10, "i");
  pkb.addUsesS(10, "x");
  pkb.addUsesS(10, "z");
  pkb.addUsesS(11, "z");
  pkb.addUsesS(12, "x");
  pkb.addUsesS(12, "y");
  pkb.addUsesS(12, "z");
  pkb.addUsesS(14, "z");
  pkb.addModifiesS(1, "x");
  pkb.addModifiesS(2, "i");
  pkb.addModifiesS(4, "x");
  pkb.addModifiesS(6, "i");
  pkb.addModifiesS(8, "x");
  pkb.addModifiesS(9, "z");
  pkb.addModifiesS(10, "z");
  pkb.addModifiesS(11, "y");
  pkb.addModifiesS(12, "x");
  pkb.addModifiesS(13, "z");
  pkb.addModifiesS(14, "v");

  pkb.addCallProc(5, "third");

  pkb.addProcStmtRange(1, 12, "second");
  pkb.addProcStmtRange(13, 14, "third");

  designExtractor.extractDesignAbstractions();

  Table affectsTable = pkb.getAffectsTable();

  REQUIRE(affectsTable.contains({ "1", "4" }));
  REQUIRE(affectsTable.contains({ "1", "8" }));
  REQUIRE(affectsTable.contains({ "1", "10" }));
  REQUIRE(affectsTable.contains({ "1", "12" }));
  REQUIRE(affectsTable.contains({ "2", "10" }));
  REQUIRE(affectsTable.contains({ "2", "6" }));
  REQUIRE(affectsTable.contains({ "4", "4" }));
  REQUIRE(affectsTable.contains({ "4", "8" }));
  REQUIRE(affectsTable.contains({ "4", "10" }));
  REQUIRE(affectsTable.contains({ "4", "12" }));
  REQUIRE(affectsTable.contains({ "6", "6" }));
  REQUIRE(affectsTable.contains({ "6", "10" }));
  REQUIRE(affectsTable.contains({ "8", "10" }));
  REQUIRE(affectsTable.contains({ "8", "12" }));
  REQUIRE(affectsTable.contains({ "9", "10" }));
  REQUIRE(affectsTable.contains({ "10", "11" }));
  REQUIRE(affectsTable.contains({ "10", "12" }));
  REQUIRE(affectsTable.contains({ "11", "12" }));
  REQUIRE(affectsTable.contains({ "13", "14" }));
  REQUIRE(affectsTable.size() == 19);
}
