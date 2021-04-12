#include "catch.hpp"

#include "DesignExtractor.h"
#include "Table.h"

TEST_CASE("[TestDesignExtractor] ParentT extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  pkb.addWhile(1);
  pkb.addWhile(2);
  pkb.addWhile(3);
  pkb.addAssign(4);

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addParent(3, 4);

  designExtractor.extractDesignAbstractions();
  Table parentTTable = pkb.getParentTTable();

  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));

  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));

  REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] FollowsT extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  pkb.addAssign(1);
  pkb.addAssign(2);
  pkb.addAssign(3);
  pkb.addAssign(4);

  pkb.addFollows(1, 2);
  pkb.addFollows(2, 3);
  pkb.addFollows(3, 4);

  designExtractor.extractDesignAbstractions();
  Table followsTTable = pkb.getFollowsTTable();

  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));

  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));

  REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(followsTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] CallsT extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  pkb.addProc("p1");
  pkb.addProc("p2");
  pkb.addProc("p3");
  pkb.addProc("p4");

  pkb.addCalls("p1", "p2");
  pkb.addCalls("p2", "p3");
  pkb.addCalls("p3", "p4");

  designExtractor.extractDesignAbstractions();
  Table callsTTable = pkb.getCallsTTable();

  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("p2") }));
  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("p3") }));
  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("p4") }));
  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("p3") }));
  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("p4") }));
  REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("p3"), pkb.getIntRefFromEntity("p4") }));
  REQUIRE(callsTTable.size() == 6);
}

TEST_CASE("[TestDesignExtractor] NextT extraction") {
  // We shall use the examples given in the slides
  // (CS3203-ADVANCED_SPA Slide 20) to test this.

  // The sample program is as follows:
  //     procedure Second {
  // 01.   x = 0;
  // 02.   i = 5;
  // 03.   while (i!=0) {
  // 04.     x = x + 2 * y;
  // 05.     call Third;
  // 06.     i = i - 1; 
  //       }
  // 07.   if (x == 1) then {
  // 08.     x = x + 1; 
  //       } else {
  // 09.     z = 1; 
  //       }
  // 10.   z = z + x + i;
  // 11.   y = z + 2;
  // 12.   x = x * y + z; 
  //     }

  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  pkb.addProc("Second");
  pkb.addProc("Third");

  pkb.addParent(3, 4);
  pkb.addParent(3, 5);
  pkb.addCallProc(5, "Third");
  pkb.addParent(3, 6);
  pkb.addParent(7, 8);
  pkb.addParent(7, 9);

  // Have to manually add in all Next relations
  // (usually done by Parser/PKB)
  pkb.addCfgEdge(1, 2);
  pkb.addCfgEdge(2, 3);
  pkb.addCfgEdge(3, 4);
  pkb.addCfgEdge(4, 5);
  pkb.addCfgEdge(5, 6);
  pkb.addCfgEdge(6, 3);
  pkb.addCfgEdge(3, 7);
  pkb.addCfgEdge(7, 8);
  pkb.addCfgEdge(7, 9);
  pkb.addCfgEdge(8, 10);
  pkb.addCfgEdge(10, 11);
  pkb.addCfgEdge(11, 12);
  pkb.addCfgEdge(9, 10);

  designExtractor.extractDesignAbstractions();

  Table nextTTable = pkb.getNextTTable();

  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(!(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) })));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(!(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(9) })));
}

TEST_CASE("[TestDesignExtractor] Uses(s != c, v) in container => Uses(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  pkb.addWhile(1);
  pkb.addWhile(2);
  pkb.addPrint(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addUsesS(3, "z");

  designExtractor.extractDesignAbstractions();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("z") }));
  REQUIRE(usesSTable.size() == 3);
}

TEST_CASE("[TestDesignExtractor] Indirect UsesP extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("usedByP2") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("usedByP3") }));

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("usedByP2") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("usedByP3") }));

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p3"), pkb.getIntRefFromEntity("usedByP3") }));
  REQUIRE(usesPTable.size() == 5);
}

TEST_CASE("[TestDesignExtractor] Uses(c, v) in container => Uses(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  // Program structure for the test:
  //     procedure p1 {
  // 01:   while (true) {
  // 02:     call p2; }}
  //     procedure p2 {
  // 03:   x = 5;
  // 04:   x = x + 1; }
  // We aim to test that UsesS(1, "x") and Uses(2, "x") are extracted by the DE.

  pkb.addWhile(1);
  pkb.addCall(2);
  pkb.addAssign(3);
  pkb.addAssign(4);
  pkb.addVar("x");
  pkb.addConst("5");
  pkb.addProc("p1");
  pkb.addProc("p2");

  // Would normally be added by the parser, etc.
  pkb.addParent(1, 2);
  pkb.addCallProc(2, "p2");
  pkb.addCalls("p1", "p2");
  pkb.addUsesP("p2", "x");
  pkb.addUsesS(4, "x");

  designExtractor.extractDesignAbstractions();

  Table usesSTable = pkb.getUsesSTable();
  Table usesPTable = pkb.getUsesPTable();
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("x") }));
}

TEST_CASE("[TestDesignExtractor] Modifies(s != c, v) in container => Modifies(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  pkb.addWhile(1);
  pkb.addWhile(2);
  pkb.addRead(3);
  pkb.addVar("z");

  pkb.addParent(1, 2);
  pkb.addParent(2, 3);
  pkb.addModifiesS(3, "z");

  designExtractor.extractDesignAbstractions();
  Table modifiesSTable = pkb.getModifiesSTable();

  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("z") }));
  REQUIRE(modifiesSTable.size() == 3);
}

TEST_CASE("[TestDesignExtractor] Indirect ModifiesP extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("modifiedByP2") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("modifiedByP3") }));

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("modifiedByP2") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("modifiedByP3") }));

  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("p3"), pkb.getIntRefFromEntity("modifiedByP3") }));
  REQUIRE(usesPTable.size() == 5);
}

TEST_CASE("[TestDesignExtractor] Modifies(c, v) in container => Modifies(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  // Program structure for the test:
  //     procedure p1 {
  // 01:   while (true) {
  // 02:     call p2; }}
  //     procedure p2 {
  // 03:   read x }
  // We aim to test that ModifiesS(1, "x"), ModifiesS(2, "x") are extracted by the DE.

  pkb.addWhile(1);
  pkb.addCall(2);
  pkb.addRead(3);
  pkb.addVar("x");
  pkb.addProc("p1");
  pkb.addProc("p2");

  // Would normally be added by the parser, etc.
  pkb.addParent(1, 2);
  pkb.addCallProc(2, "p2");
  pkb.addCalls("p1", "p2");
  pkb.addModifiesP("p2", "x");
  pkb.addModifiesS(3, "x");

  designExtractor.extractDesignAbstractions();

  Table modifiesSTable = pkb.getModifiesSTable();
  Table modifiesPTable = pkb.getModifiesPTable();

  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("p1"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("p2"), pkb.getIntRefFromEntity("x") }));
}

TEST_CASE("[TestDesignExtractor] Affects extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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
  pkb.addCfgEdge(1, 2);
  pkb.addCfgEdge(2, 3);
  pkb.addCfgEdge(3, 4);
  pkb.addCfgEdge(4, 5);
  pkb.addCfgEdge(5, 6);
  pkb.addCfgEdge(6, 3);
  pkb.addCfgEdge(3, 7);
  pkb.addCfgEdge(7, 8);
  pkb.addCfgEdge(7, 9);
  pkb.addCfgEdge(8, 10);
  pkb.addCfgEdge(9, 10);
  pkb.addCfgEdge(10, 11);
  pkb.addCfgEdge(11, 12);
  pkb.addCfgEdge(13, 14);

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

  // adding other relevant information
  pkb.addCallProc(5, "third");

  designExtractor.extractDesignAbstractions();

  Table affectsTable = pkb.getAffectsTable();

  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(13), pkb.getIntRefFromStmtNum(14) }));
  REQUIRE(affectsTable.size() == 19);
}

TEST_CASE("[TestDesignExtractor] AffectsT extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

  // Example Code 5 in Advanced SPA requirements
  pkb.addProc("Second");
  pkb.addProc("Third");

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
  pkb.addCfgEdge(1, 2);
  pkb.addCfgEdge(2, 3);
  pkb.addCfgEdge(3, 4);
  pkb.addCfgEdge(4, 5);
  pkb.addCfgEdge(5, 6);
  pkb.addCfgEdge(6, 3);
  pkb.addCfgEdge(3, 7);
  pkb.addCfgEdge(7, 8);
  pkb.addCfgEdge(7, 9);
  pkb.addCfgEdge(8, 10);
  pkb.addCfgEdge(9, 10);
  pkb.addCfgEdge(10, 11);
  pkb.addCfgEdge(11, 12);
  pkb.addCfgEdge(13, 14);

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

  designExtractor.extractDesignAbstractions();

  Table affectsTTable = pkb.getAffectsTTable();

  REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(12) }));
}
