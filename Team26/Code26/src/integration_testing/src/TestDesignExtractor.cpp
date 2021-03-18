#include "catch.hpp"

#include "Table.h"
#include "DesignExtractor.h"

TEST_CASE("[TestDesignExtractor] ParentT extraction") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  REQUIRE(callsTTable.contains({ "p1", "p2" }));
  REQUIRE(callsTTable.contains({ "p1", "p3" }));
  REQUIRE(callsTTable.contains({ "p1", "p4" }));
  REQUIRE(callsTTable.contains({ "p2", "p3" }));
  REQUIRE(callsTTable.contains({ "p2", "p4" }));
  REQUIRE(callsTTable.contains({ "p3", "p4" }));
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
  for (int i = 1; i <= 12; i++) {
    pkb.addStmt(i);
  }
  pkb.addProc("Third");

  pkb.addParent(3, 4);
  pkb.addParent(3, 5);
  pkb.addCallProc(5, "Third");
  pkb.addParent(3, 6);
  pkb.addParent(7, 8);
  pkb.addParent(7, 9);

  // Have to manually add in all Next relations
  // (usually done by Parser/PKB)
  pkb.addNext(1, 2);
  pkb.addNext(2, 3);
  pkb.addNext(3, 4);
  pkb.addNext(4, 5);
  pkb.addNext(5, 6);
  pkb.addNext(6, 3);
  pkb.addNext(3, 7);
  pkb.addNext(7, 8);
  pkb.addNext(7, 9);
  pkb.addNext(8, 10);
  pkb.addNext(10, 11);
  pkb.addNext(11, 12);
  pkb.addNext(9, 10);

  designExtractor.extractDesignAbstractions();

  Table nextTTable = pkb.getNextTTable();

  REQUIRE(nextTTable.contains({ "1", "2" }));
  REQUIRE(nextTTable.contains({ "1", "3" }));
  REQUIRE(nextTTable.contains({ "2", "5" }));
  REQUIRE(nextTTable.contains({ "4", "3" }));
  REQUIRE(!(nextTTable.contains({ "5", "2" })));
  REQUIRE(nextTTable.contains({ "5", "5" }));
  REQUIRE(nextTTable.contains({ "5", "8" }));
  REQUIRE(nextTTable.contains({ "5", "12" }));
  REQUIRE(!(nextTTable.contains({ "8", "9" })));
}

TEST_CASE("[TestDesignExtractor] Uses(s != c, v) in container => Uses(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  REQUIRE(usesPTable.contains({ "p1", "usedByP2" }));
  REQUIRE(usesPTable.contains({ "p1", "usedByP3" }));

  REQUIRE(usesPTable.contains({ "p2", "usedByP2" }));
  REQUIRE(usesPTable.contains({ "p2", "usedByP3" }));

  REQUIRE(usesPTable.contains({ "p3", "usedByP3" }));
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

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
  pkb.addStmt(4);
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
  REQUIRE(usesSTable.contains({ "1", "x" }));
  REQUIRE(usesSTable.contains({ "2", "x" }));
  REQUIRE(usesSTable.contains({ "4", "x" }));
  REQUIRE(usesPTable.contains({ "p1", "x" }));
  REQUIRE(usesPTable.contains({ "p2", "x" }));
}

TEST_CASE("[TestDesignExtractor] Modifies(s != c, v) in container => Modifies(ifs/w, v)") {
  Pkb pkb;
  SourceProcessor::DesignExtractor designExtractor(pkb);

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

  REQUIRE(usesPTable.contains({ "p1", "modifiedByP2" }));
  REQUIRE(usesPTable.contains({ "p1", "modifiedByP3" }));

  REQUIRE(usesPTable.contains({ "p2", "modifiedByP2" }));
  REQUIRE(usesPTable.contains({ "p2", "modifiedByP3" }));

  REQUIRE(usesPTable.contains({ "p3", "modifiedByP3" }));
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

  pkb.addStmt(1);
  pkb.addStmt(2);
  pkb.addStmt(3);
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

  REQUIRE(modifiesSTable.contains({ "1", "x" }));
  REQUIRE(modifiesSTable.contains({ "2", "x" }));
  REQUIRE(modifiesSTable.contains({ "3", "x" }));
  REQUIRE(modifiesPTable.contains({ "p1", "x" }));
  REQUIRE(modifiesPTable.contains({ "p2", "x" }));
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

  pkb.addCallProc(5, "Third");

  designExtractor.extractDesignAbstractions();

  Table affectsTTable = pkb.getAffectsTTable();

  REQUIRE(affectsTTable.contains({ "1", "4" }));
  REQUIRE(affectsTTable.contains({ "1", "10" }));
  REQUIRE(affectsTTable.contains({ "1", "11" }));
  REQUIRE(affectsTTable.contains({ "9", "12" }));
}
