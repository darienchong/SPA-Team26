#include "catch.hpp"

#include "Pkb.h"

TEST_CASE("[TestPkb] varTable Insertion") {
  Pkb pkb;
  pkb.addVar("x");
  auto dataCopy = pkb.getVarTable().getData();
  REQUIRE(dataCopy.find({ "x" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] stmtTable Insertion") {
  Pkb pkb;
  pkb.addStmt(2);
  auto dataCopy = pkb.getStmtTable().getData();
  REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] procTable Insertion") {
  Pkb pkb;
  pkb.addProc("main");
  auto dataCopy = pkb.getProcTable().getData();
  REQUIRE(dataCopy.find({ "main" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] constTable Insertion") {
  Pkb pkb;
  pkb.addConst(std::to_string(2));
  auto dataCopy = pkb.getConstTable().getData();
  REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}


TEST_CASE("[TestPkb] ifTable Insertion") {
  Pkb pkb;
  pkb.addIf(12);
  auto dataCopy = pkb.getIfTable().getData();
  REQUIRE(dataCopy.find({ "12" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({ "12" }));
}

TEST_CASE("[TestPkb] whileTable Insertion") {
  Pkb pkb;
  pkb.addWhile(15);
  auto dataCopy = pkb.getWhileTable().getData();
  REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({ "15" }));
}

TEST_CASE("[TestPkb] readTable Insertion") {
  Pkb pkb;
  pkb.addRead(15);
  auto dataCopy = pkb.getReadTable().getData();
  REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({ "15" }));
}

TEST_CASE("[TestPkb] printTable Insertion") {
  Pkb pkb;
  pkb.addPrint(526);
  auto dataCopy = pkb.getPrintTable().getData();
  REQUIRE(dataCopy.find({ "526" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({ "526" }));
}

TEST_CASE("[TestPkb] assignTable Insertion") {
  Pkb pkb;
  pkb.addAssign(32);
  auto dataCopy = pkb.getAssignTable().getData();
  REQUIRE(dataCopy.find({ "32" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({ "32" }));
}

TEST_CASE("[TestPkb] addFollows") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addFollows(5, 6);
    auto dataCopy = pkb.getFollowsTable().getData();
    REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  }

  SECTION("Check invalid insertion") {
    REQUIRE_THROWS(pkb.addFollows(5, 4));
  }
}

TEST_CASE("[TestPkb] addFollowsT(int, int)") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addFollowsT(5, 6);
    auto dataCopy = pkb.getFollowsTTable().getData();
    REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  }

  SECTION("Check invalid insertion") {
    REQUIRE_THROWS(pkb.addFollowsT(5, 4));
  }
}

TEST_CASE("[TestPkb] addParent") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addParent(5, 6);
    auto dataCopy = pkb.getParentTable().getData();
    REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  }

  SECTION("Check invalid insertion") {
    REQUIRE_THROWS(pkb.addParent(5, 4));
  }
}

TEST_CASE("[TestPkb] addParentT(int, int)") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addParentT(5, 6);
    auto dataCopy = pkb.getParentTTable().getData();
    REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  }

  SECTION("Check invalid insertion") {
    REQUIRE_THROWS(pkb.addParentT(5, 4));
  }
}

TEST_CASE("[TestPkb] addUsesS") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addUsesS(5, "x");
    pkb.addUsesS(7, "y");
    auto dataCopy = pkb.getUsesSTable().getData();
    REQUIRE(dataCopy.count({ "5", "x" }) == 1);
    REQUIRE(dataCopy.count({ "7", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addUsesP") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addUsesP("foo", "x");
    pkb.addUsesP("bar", "y");
    auto dataCopy = pkb.getUsesPTable().getData();
    REQUIRE(dataCopy.count({ "foo", "x" }) == 1);
    REQUIRE(dataCopy.count({ "bar", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addModifiesS") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addModifiesS(5, "x");
    pkb.addModifiesS(7, "y");
    auto dataCopy = pkb.getModifiesSTable().getData();
    REQUIRE(dataCopy.count({ "5", "x" }) == 1);
    REQUIRE(dataCopy.count({ "7", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addModifiesP") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addModifiesP("foo", "x");
    pkb.addModifiesP("bar", "y");
    auto dataCopy = pkb.getModifiesPTable().getData();
    REQUIRE(dataCopy.count({ "foo", "x" }) == 1);
    REQUIRE(dataCopy.count({ "bar", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addCalls") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addCalls("main", "foo");
    pkb.addCalls("foo", "bar");
    auto dataCopy = pkb.getCallsTable().getData();
    REQUIRE(dataCopy.count({ "main", "foo" }) == 1);
    REQUIRE(dataCopy.count({ "foo", "bar" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addCallsT") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addCallsT("main", "foo");
    pkb.addCallsT("foo", "bar");
    pkb.addCallsT("main", "bar");
    auto dataCopy = pkb.getCallsTTable().getData();
    REQUIRE(dataCopy.count({ "main", "foo" }) == 1);
    REQUIRE(dataCopy.count({ "foo", "bar" }) == 1);
    REQUIRE(dataCopy.count({ "main", "bar" }) == 1);
    REQUIRE(dataCopy.size() == 3);
  }
}

TEST_CASE("[TestPkb] addNext") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addNext(3, 4);
    pkb.addNext(4, 5);
    auto dataCopy = pkb.getNextTable().getData();
    REQUIRE(dataCopy.count({ "3", "4" }) == 1);
    REQUIRE(dataCopy.count({ "4", "5" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addNextT") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addNextT(3, 4);
    pkb.addNextT(4, 5);
    pkb.addNextT(3, 5);
    auto dataCopy = pkb.getNextTTable().getData();
    REQUIRE(dataCopy.count({ "3", "4" }) == 1);
    REQUIRE(dataCopy.count({ "4", "5" }) == 1);
    REQUIRE(dataCopy.count({ "3", "5" }) == 1);
    REQUIRE(dataCopy.size() == 3);
  }
}

TEST_CASE("[TestPkb] addAffects") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addAffects(3, 4);
    pkb.addAffects(4, 7);
    auto dataCopy = pkb.getAffectsTable().getData();
    REQUIRE(dataCopy.count({ "3", "4" }) == 1);
    REQUIRE(dataCopy.count({ "4", "7" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addAffectsT") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addAffectsT(3, 4);
    pkb.addAffectsT(4, 7);
    pkb.addAffectsT(3, 7);
    auto dataCopy = pkb.getAffectsTTable().getData();
    REQUIRE(dataCopy.count({ "3", "4" }) == 1);
    REQUIRE(dataCopy.count({ "4", "7" }) == 1);
    REQUIRE(dataCopy.count({ "3", "7" }) == 1);
    REQUIRE(dataCopy.size() == 3);
  }
}

TEST_CASE("[TestPkb] addPatternAssign") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addPatternAssign(5, "x", "x y *");
    pkb.addPatternAssign(7, "_", "b c * a +");
    auto dataCopy = pkb.getPatternAssignTable().getData();
    REQUIRE(dataCopy.count({ "5", "x", "x y *" }) == 1);
    REQUIRE(dataCopy.count({ "7", "_", "b c * a +" }) == 1);
  }
}

TEST_CASE("[TestPkb] addPatternIf") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addPatternIf(1, "count");
    pkb.addPatternIf(70, "a");
    auto dataCopy = pkb.getPatternIfTable().getData();
    REQUIRE(dataCopy.count({ "1", "count" }) == 1);
    REQUIRE(dataCopy.count({ "70", "a" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addPatternWhile") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addPatternWhile(4, "count");
    pkb.addPatternWhile(100, "i");
    auto dataCopy = pkb.getPatternWhileTable().getData();
    REQUIRE(dataCopy.count({ "4", "count" }) == 1);
    REQUIRE(dataCopy.count({ "100", "i" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addCallProc") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addCallProc(4, "proc1");
    pkb.addCallProc(20, "proc2");
    auto dataCopy = pkb.getCallProcTable().getData();
    REQUIRE(dataCopy.count({ "4", "proc1" }) == 1);
    REQUIRE(dataCopy.count({ "20", "proc2" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addReadVar") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addReadVar(1, "x");
    pkb.addReadVar(3, "y");
    auto dataCopy = pkb.getReadVarTable().getData();
    REQUIRE(dataCopy.count({ "1", "x" }) == 1);
    REQUIRE(dataCopy.count({ "3", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}

TEST_CASE("[TestPkb] addPrintVar") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addPrintVar(1, "x");
    pkb.addPrintVar(3, "y");
    auto dataCopy = pkb.getPrintVarTable().getData();
    REQUIRE(dataCopy.count({ "1", "x" }) == 1);
    REQUIRE(dataCopy.count({ "3", "y" }) == 1);
    REQUIRE(dataCopy.size() == 2);
  }
}



TEST_CASE("[TestPkb] getFollows") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  Table table = pkb.getFollower(2);
  REQUIRE(table.getData().count({ "5" }));
}

TEST_CASE("[TestPkb] getFollowedBy") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  Table table = pkb.getFollowedBy(2);
  REQUIRE(table.getData().count({ "1" }));
}

TEST_CASE("[TestPkb] getFollowerT") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(2, 5);
  pkb.addFollows(3, 4);

  pkb.addFollowsT(1, 2);
  pkb.addFollowsT(1, 5);
  pkb.addFollowsT(2, 5);
  pkb.addFollowsT(3, 4);

  Table table = pkb.getFollowerT(1);
  REQUIRE(table.getData().count({ "2" }));
  REQUIRE(table.getData().count({ "5" }));
}

TEST_CASE("[TestPkb] getFollowedByT") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(2, 5);
  pkb.addFollows(3, 4);

  pkb.addFollowsT(1, 2);
  pkb.addFollowsT(1, 5);
  pkb.addFollowsT(2, 5);
  pkb.addFollowsT(3, 4);

  Table table = pkb.getFollowedByT(5);
  REQUIRE(table.getData().count({ "1" }));
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getParent") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  Table table = pkb.getParent(5);
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getChild") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  Table table = pkb.getChild(1);
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getParentT") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(2, 5);
  pkb.addParent(3, 4);

  pkb.addParentT(1, 2);
  pkb.addParentT(1, 5);
  pkb.addParentT(2, 5);
  pkb.addParentT(3, 4);

  Table table = pkb.getParentT(5);
  REQUIRE(table.getData().count({ "1" }));
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getChildT") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(2, 5);
  pkb.addParent(3, 4);

  pkb.addParentT(1, 2);
  pkb.addParentT(1, 5);
  pkb.addParentT(2, 5);
  pkb.addParentT(3, 4);

  Table table = pkb.getChildT(1);
  REQUIRE(table.getData().count({ "2" }));
  REQUIRE(table.getData().count({ "5" }));
}

TEST_CASE("[TestPkb] getUses") {
  Pkb pkb;
  pkb.addUsesS(1, "x");
  pkb.addUsesS(2, "y");
  Table table = pkb.getUses("x");
  REQUIRE(table.getData().count({ "1" }));
  pkb.addUsesS(2, "x");
  table = pkb.getUses("x");
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getUsedBy") {
  Pkb pkb;
  pkb.addUsesS(1, "x");
  pkb.addUsesS(2, "y");
  pkb.addUsesS(2, "x");

  SECTION("used by statement") {
    Table table = pkb.getUsedBy(1);
    REQUIRE(table.getData().count({ "x" }));
    table = pkb.getUsedBy(2);
    REQUIRE(table.getData().count({ "x" }));
    REQUIRE(table.getData().count({ "y" }));
  }
}

TEST_CASE("[TestPkb] getModifies") {
  Pkb pkb;
  pkb.addModifiesS(1, "x");
  pkb.addModifiesS(2, "y");
  Table table = pkb.getModifies("x");
  REQUIRE(table.getData().count({ "1" }));
  pkb.addModifiesS(2, "x");
  table = pkb.getModifies("x");
  REQUIRE(table.getData().count({ "2" }));
}

TEST_CASE("[TestPkb] getModifiedBy") {
  Pkb pkb;
  pkb.addModifiesS(1, "x");
  pkb.addModifiesS(2, "y");
  pkb.addModifiesS(2, "x");

  SECTION("modified by statement") {
    Table table = pkb.getModifiedBy(1);
    REQUIRE(table.getData().count({ "x" }));
    table = pkb.getModifiedBy(2);
    REQUIRE(table.getData().count({ "x" }));
    REQUIRE(table.getData().count({ "y" }));
  }
}