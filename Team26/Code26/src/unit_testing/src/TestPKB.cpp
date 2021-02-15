#include <catch.hpp>

#include "PKB.h"

TEST_CASE("[TestPkb] varTable Insertion") {
	PKB pkb;
	pkb.addVar("x");
	auto dataCopy = pkb.getVarTable().getData();
	REQUIRE(dataCopy.find({ "x" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] stmtTable Insertion") {
	PKB pkb;
	pkb.addStmt(2);
	auto dataCopy = pkb.getStmtTable().getData();
	REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] procTable Insertion") {
	PKB pkb;
	pkb.addProc("main");
	auto dataCopy = pkb.getProcTable().getData();
	REQUIRE(dataCopy.find({ "main" }) != dataCopy.end());
}

TEST_CASE("[TestPkb] constTable Insertion") {
  PKB pkb;
  pkb.addConst(std::to_string(2));
  auto dataCopy = pkb.getConstTable().getData();
  REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}


TEST_CASE("[TestPkb] ifTable Insertion") {
	PKB pkb;
	pkb.addIf(12);
	auto dataCopy = pkb.getIfTable().getData();
	REQUIRE(dataCopy.find({ "12" }) != dataCopy.end());
	auto stmtDataCopy = pkb.getStmtTable().getData();
	REQUIRE(stmtDataCopy.count({"12"}));
}

TEST_CASE("[TestPkb] whileTable Insertion") {
	PKB pkb;
	pkb.addWhile(15);
	auto dataCopy = pkb.getWhileTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"15"}));
}

TEST_CASE("[TestPkb] readTable Insertion") {
	PKB pkb;
	pkb.addRead(15);
	auto dataCopy = pkb.getReadTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"15"}));
}

TEST_CASE("[TestPkb] printTable Insertion") {
	PKB pkb;
	pkb.addPrint(526);
	auto dataCopy = pkb.getPrintTable().getData();
	REQUIRE(dataCopy.find({ "526" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"526"}));
}

TEST_CASE("[TestPkb] assignTable Insertion") {
	PKB pkb;
	pkb.addAssign(32);
	auto dataCopy = pkb.getAssignTable().getData();
	REQUIRE(dataCopy.find({ "32" }) != dataCopy.end());
  auto stmtDataCopy = pkb.getStmtTable().getData();
  REQUIRE(stmtDataCopy.count({"32"}));
}

TEST_CASE("[TestPkb] addFollows") {
	PKB pkb;

	SECTION("Check valid insertion") {
		pkb.addFollows(5, 6);
		auto dataCopy = pkb.getFollowsTable().getData();
		REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	}

	SECTION("Check invalid insertion") {
		REQUIRE_THROWS(pkb.addFollows(5, 4));
	}
}

TEST_CASE("[TestPkb] addFollowsT") {
	PKB pkb;
	pkb.addFollows(5, 6);
	pkb.addFollows(6, 7);
	pkb.addFollowsT();
	auto dataCopy = pkb.getFollowsTTable().getData();
	REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	REQUIRE(dataCopy.count({ "6", "7" }) == 1);
	REQUIRE(dataCopy.count({ "5", "7" }) == 1);
}

TEST_CASE("[TestPkb] addParent") {
	PKB pkb;

	SECTION("Check valid insertion") {
		pkb.addParent(5, 6);
		auto dataCopy = pkb.getParentTable().getData();
		REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	}

	SECTION("Check invalid insertion") {
		REQUIRE_THROWS(pkb.addParent(5, 4));
	}
}

TEST_CASE("[TestPkb] addParentT") {
	PKB pkb;
	pkb.addParent(5, 6);
	pkb.addParent(6, 7);
	pkb.addParentT();
	auto dataCopy = pkb.getParentTTable().getData();
	REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	REQUIRE(dataCopy.count({ "6", "7" }) == 1);
	REQUIRE(dataCopy.count({ "5", "7" }) == 1);
}

TEST_CASE("[TestPkb] addUses") {
	PKB pkb;

	SECTION("Check valid insertion") {
		pkb.addUses(5, "x");
		pkb.addUses("main", "y");
		auto dataCopy = pkb.getUsesTable().getData();
		REQUIRE(dataCopy.count({ "5", "x" }) == 1);
		REQUIRE(dataCopy.count({ "main", "y" }) == 1);
	}
}

TEST_CASE("[TestPkb] addModifies") {
	PKB pkb;

	SECTION("Check valid insertion") {
		pkb.addModifies(5, "x");
		pkb.addModifies("main", "y");
		auto dataCopy = pkb.getModifiesTable().getData();
		REQUIRE(dataCopy.count({ "5", "x" }) == 1);
		REQUIRE(dataCopy.count({ "main", "y" }) == 1);
	}
}

TEST_CASE("[TestPkb] addPatternAssign") {
	PKB pkb;

	SECTION("Check valid insertion") {
		pkb.addPatternAssign(5, "x", "x y *");
		pkb.addPatternAssign(7, "_", "b c * a +");
		auto dataCopy = pkb.getPatternAssignTable().getData();
		REQUIRE(dataCopy.count({ "5", "x", "x y *" }) == 1);
		REQUIRE(dataCopy.count({ "7", "_", "b c * a +" }) == 1);
	}
}
