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
  REQUIRE(stmtDataCopy.count({"12"}));
}

TEST_CASE("[TestPkb] whileTable Insertion") {
  Pkb pkb;
  pkb.addWhile(15);
  auto dataCopy = pkb.getWhileTable().getData();
  REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"15"}));
}

TEST_CASE("[TestPkb] readTable Insertion") {
  Pkb pkb;
  pkb.addRead(15);
  auto dataCopy = pkb.getReadTable().getData();
  REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"15"}));
}

TEST_CASE("[TestPkb] printTable Insertion") {
  Pkb pkb;
  pkb.addPrint(526);
  auto dataCopy = pkb.getPrintTable().getData();
  REQUIRE(dataCopy.find({ "526" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"526"}));
}

TEST_CASE("[TestPkb] assignTable Insertion") {
  Pkb pkb;
  pkb.addAssign(32);
  auto dataCopy = pkb.getAssignTable().getData();
  REQUIRE(dataCopy.find({ "32" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"32"}));
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

TEST_CASE("[TestPkb] addFollowsT()") {
  Pkb pkb;
  pkb.addFollows(5, 6);
  pkb.addFollows(6, 7);
  
  // TODO: Add transitive relations

  auto dataCopy = pkb.getFollowsTTable().getData();
  REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  REQUIRE(dataCopy.count({ "6", "7" }) == 1);
  REQUIRE(dataCopy.count({ "5", "7" }) == 1);
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

TEST_CASE("[TestPkb] addParentT()") {
  Pkb pkb;
  pkb.addParent(5, 6);
  pkb.addParent(6, 7);
  // TODO: pkb.addParentT();
  auto dataCopy = pkb.getParentTTable().getData();
  REQUIRE(dataCopy.count({ "5", "6" }) == 1);
  REQUIRE(dataCopy.count({ "6", "7" }) == 1);
  REQUIRE(dataCopy.count({ "5", "7" }) == 1);
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

TEST_CASE("[TestPkb] addUses") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addUses(5, "x");
    pkb.addUses("main", "y");
    auto dataCopy = pkb.getUsesTable().getData();
    REQUIRE(dataCopy.count({ "5", "x" }) == 1);
    REQUIRE(dataCopy.count({ "main", "y" }) == 1);
  }
}

TEST_CASE("[TestPkb] addModifies") {
  Pkb pkb;

  SECTION("Check valid insertion") {
    pkb.addModifies(5, "x");
    pkb.addModifies("main", "y");
    auto dataCopy = pkb.getModifiesTable().getData();
    REQUIRE(dataCopy.count({ "5", "x" }) == 1);
    REQUIRE(dataCopy.count({ "main", "y" }) == 1);
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

TEST_CASE("[TestPkb] addIndirectUses") {
  Pkb pkb;

  SECTION("with parentTTable already filled") {
    pkb.addUses(2, "x");
    pkb.addParent(1, 2);
    // TODO: pkb.addParentT();
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().getData().count({"1", "x"}) == 1);
  }

  SECTION("without parentTTable already filled") {
    pkb.addUses(2, "x");
    pkb.addParent(1, 2);
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().getData().count({"1", "x"}) == 1);
  }

  SECTION("3 transitive steps") {
    pkb.addUses(3, "x");
    pkb.addParent(1, 2);
    pkb.addParent(2, 3);
    // TODO: pkb.addParentT();
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().getData().count({"1", "x"}) == 1);
  }
}

TEST_CASE("[TestPkb] addIndirectModifies") {
  Pkb pkb;

  SECTION("with parentTTable already filled") {
    pkb.addModifies(2, "x");
    pkb.addParent(1, 2);
    // TODO: pkb.addParentT();
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().getData().count({"1", "x"}) == 1);
  }

  SECTION("without parentTTable already filled") {
    pkb.addModifies(2, "x");
    pkb.addParent(1, 2);
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().getData().count({"1", "x"}) == 1);
  }

  SECTION("3 transitive steps") {
    pkb.addModifies(3, "x");
    pkb.addParent(1, 2);
    pkb.addParent(2, 3);
    // TODO: pkb.addParentT();
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().getData().count({"1", "x"}) == 1);
  }
}

TEST_CASE("[TestPkb] getFollows") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  Table table = pkb.getFollower(2);
  REQUIRE(table.getData().count({"5"}));
}

TEST_CASE("[TestPkb] getFollowedBy") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  Table table = pkb.getFollowedBy(2);
  REQUIRE(table.getData().count({"1"}));
}

TEST_CASE("[TestPkb] getFollowerT") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  // TODO: pkb.addFollowsT();
  Table table = pkb.getFollowerT(1);
  REQUIRE(table.getData().count({"2"}));
  REQUIRE(table.getData().count({"5"}));
}

TEST_CASE("[TestPkb] getFollowedByT") {
  Pkb pkb;
  pkb.addFollows(1, 2);
  pkb.addFollows(3, 4);
  pkb.addFollows(2, 5);
  // TODO: pkb.addFollowsT();
  Table table = pkb.getFollowedByT(5);
  REQUIRE(table.getData().count({"1"}));
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getParent") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  Table table = pkb.getParent(5);
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getChild") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  Table table = pkb.getChild(1);
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getParentT") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  // TODO: pkb.addParentT();
  Table table = pkb.getParentT(5);
  REQUIRE(table.getData().count({"1"}));
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getChildT") {
  Pkb pkb;
  pkb.addParent(1, 2);
  pkb.addParent(3, 4);
  pkb.addParent(2, 5);
  // TODO: pkb.addParentT();
  Table table = pkb.getChildT(1);
  REQUIRE(table.getData().count({"2"}));
  REQUIRE(table.getData().count({"5"}));
}

TEST_CASE("[TestPkb] getUses") {
  Pkb pkb;
  pkb.addUses(1, "x");
  pkb.addUses(2, "y");
  Table table = pkb.getUses("x");
  REQUIRE(table.getData().count({"1"}));
  pkb.addUses(2, "x");
  table = pkb.getUses("x");
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getUsedBy") {
  Pkb pkb;
  pkb.addUses(1, "x");
  pkb.addUses(2, "y");
  pkb.addUses(2, "x");
  pkb.addUses("proc1", "x");

  SECTION("used by statement") {
    Table table = pkb.getUsedBy(1);
    REQUIRE(table.getData().count({"x"}));
    table = pkb.getUsedBy(2);
    REQUIRE(table.getData().count({"x"}));
    REQUIRE(table.getData().count({"y"}));
  }

  SECTION("used by procedure") {
    Table table = pkb.getUsedBy("proc1");
    REQUIRE(table.getData().count({"x"}));
  }
}

TEST_CASE("[TestPkb] getModifies") {
  Pkb pkb;
  pkb.addModifies(1, "x");
  pkb.addModifies(2, "y");
  Table table = pkb.getModifies("x");
  REQUIRE(table.getData().count({"1"}));
  pkb.addModifies(2, "x");
  table = pkb.getModifies("x");
  REQUIRE(table.getData().count({"2"}));
}

TEST_CASE("[TestPkb] getModifiedBy") {
  Pkb pkb;
  pkb.addModifies(1, "x");
  pkb.addModifies(2, "y");
  pkb.addModifies(2, "x");
  pkb.addModifies("proc1", "x");

  SECTION("modified by statement") {
    Table table = pkb.getModifiedBy(1);
    REQUIRE(table.getData().count({"x"}));
    table = pkb.getModifiedBy(2);
    REQUIRE(table.getData().count({"x"}));
    REQUIRE(table.getData().count({"y"}));
  }

  SECTION("modified by procedure") {
    Table table = pkb.getModifiedBy("proc1");
    REQUIRE(table.getData().count({"x"}));
  }
}

TEST_CASE("[TestPkb] Add Indirect Uses") {
  SECTION("Valid Input with one indirect relation") {
    Pkb pkb;
    pkb.addUses(1, "x");
    pkb.addUses(2, "y");
    pkb.addParentT(1, 2);
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().size() == 3);
    REQUIRE(pkb.getUsesTable().contains({ "1", "y" }));
  }

  SECTION("Valid Input with multiple indirect relations") {
    Pkb pkb;
    pkb.addUses(3, "y");
    pkb.addUses(3, "z");
    pkb.addUses(5, "a");
    pkb.addParentT(1, 3);
    pkb.addParentT(3, 5);
    pkb.addParentT(1, 5);
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().size() == 7);
    REQUIRE(pkb.getUsesTable().contains({ "1", "y" }));
    REQUIRE(pkb.getUsesTable().contains({ "1", "z" }));
    REQUIRE(pkb.getUsesTable().contains({ "1", "a" }));
    REQUIRE(pkb.getUsesTable().contains({ "3", "a" }));
  }

  SECTION("Valid input with no parentTTable") {
    Pkb pkb;
    pkb.addUses(3, "y");
    pkb.addUses(3, "z");
    pkb.addParent(1, 3);
    // TODO: pkb.addIndirectUses();
    REQUIRE(pkb.getUsesTable().size() == 4);
  }
}

TEST_CASE("[TestPkb] Add Indirect Modifies") {
  SECTION("Valid Input with one indirect relation") {
    Pkb pkb;
    pkb.addModifies(1, "x");
    pkb.addModifies(2, "y");
    pkb.addParentT(1, 2);
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().size() == 3);
    REQUIRE(pkb.getModifiesTable().contains({ "1", "y" }));
  }

  SECTION("Valid Input with multiple indirect relations") {
    Pkb pkb;
    pkb.addModifies(3, "y");
    pkb.addModifies(3, "z");
    pkb.addModifies(5, "a");
    pkb.addParentT(1, 3);
    pkb.addParentT(3, 5);
    pkb.addParentT(1, 5);
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().size() == 7);
    REQUIRE(pkb.getModifiesTable().contains({ "1", "y" }));
    REQUIRE(pkb.getModifiesTable().contains({ "1", "z" }));
    REQUIRE(pkb.getModifiesTable().contains({ "1", "a" }));
    REQUIRE(pkb.getModifiesTable().contains({ "3", "a" }));
  }

  SECTION("Valid input with no parentTTable") {
    Pkb pkb;
    pkb.addModifies(3, "y");
    pkb.addModifies(3, "z");
    pkb.addParent(1, 3);
    // TODO: pkb.addIndirectModifies();
    REQUIRE(pkb.getModifiesTable().size() == 4);
    REQUIRE(pkb.getParentTTable().size() == 1);
    REQUIRE(pkb.getModifiesTable().contains({ "3", "y" }));
    REQUIRE(pkb.getModifiesTable().contains({ "3", "z" }));
    REQUIRE(pkb.getModifiesTable().contains({ "1", "y" }));
    REQUIRE(pkb.getModifiesTable().contains({ "1", "z" }));
  }
}
