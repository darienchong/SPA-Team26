#include <catch.hpp>

#include "Pkb.h"

TEST_CASE("[Pkb.cpp] varTable Insertion") {
	Pkb pkb;
	pkb.addVar("x");
	auto dataCopy = pkb.getVarTable().getData();
	REQUIRE(dataCopy.find({ "x" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] stmtTable Insertion") {
	Pkb pkb;
	pkb.addStmt(2);
	auto dataCopy = pkb.getStmtTable().getData();
	REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] procTable Insertion") {
	Pkb pkb;
	pkb.addProc("main");
	auto dataCopy = pkb.getProcTable().getData();
	REQUIRE(dataCopy.find({ "main" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] ifTable Insertion") {
	Pkb pkb;
	pkb.addIf(12);
	auto dataCopy = pkb.getIfTable().getData();
	REQUIRE(dataCopy.find({ "12" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] whileTable Insertion") {
	Pkb pkb;
	pkb.addWhile(15);
	auto dataCopy = pkb.getWhileTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] readTable Insertion") {
	Pkb pkb;
	pkb.addRead(15);
	auto dataCopy = pkb.getReadTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] printTable Insertion") {
	Pkb pkb;
	pkb.addPrint(526);
	auto dataCopy = pkb.getPrintTable().getData();
	REQUIRE(dataCopy.find({ "526" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] assignTable Insertion") {
	Pkb pkb;
	pkb.addAssign(32);
	auto dataCopy = pkb.getAssignTable().getData();
	REQUIRE(dataCopy.find({ "32" }) != dataCopy.end());
}

TEST_CASE("[Pkb.cpp] addFollows") {
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

TEST_CASE("[Pkb.cpp] addFollowsT") {
	Pkb pkb;
	pkb.addFollows(5, 6);
	pkb.addFollows(6, 7);
	pkb.addFollowsT();
	auto dataCopy = pkb.getFollowsTTable().getData();
	REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	REQUIRE(dataCopy.count({ "6", "7" }) == 1);
	REQUIRE(dataCopy.count({ "5", "7" }) == 1);
}

TEST_CASE("[Pkb.cpp] addParent") {
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

TEST_CASE("[Pkb.cpp] addParentT") {
	Pkb pkb;
	pkb.addParent(5, 6);
	pkb.addParent(6, 7);
	pkb.addParentT();
	auto dataCopy = pkb.getParentTTable().getData();
	REQUIRE(dataCopy.count({ "5", "6" }) == 1);
	REQUIRE(dataCopy.count({ "6", "7" }) == 1);
	REQUIRE(dataCopy.count({ "5", "7" }) == 1);
}

TEST_CASE("[Pkb.cpp] addUses") {
	Pkb pkb;

	SECTION("Check valid insertion") {
		pkb.addUses(5, "x");
		pkb.addUses("main", "y");
		auto dataCopy = pkb.getUsesTable().getData();
		REQUIRE(dataCopy.count({ "5", "x" }) == 1);
		REQUIRE(dataCopy.count({ "main", "y" }) == 1);
	}
}

TEST_CASE("[Pkb.cpp] addModifies") {
	Pkb pkb;

	SECTION("Check valid insertion") {
		pkb.addModifies(5, "x");
		pkb.addModifies("main", "y");
		auto dataCopy = pkb.getModifiesTable().getData();
		REQUIRE(dataCopy.count({ "5", "x" }) == 1);
		REQUIRE(dataCopy.count({ "main", "y" }) == 1);
	}
}


TEST_CASE("[Pkb.cpp] addPatternAssign") {
	Pkb pkb;

	SECTION("Check valid insertion") {
		pkb.addPatternAssign(5, "x", "x y *");
		pkb.addPatternAssign(7, "_", "b c * a +");
		auto dataCopy = pkb.getPatternAssignTable().getData();
		REQUIRE(dataCopy.count({ "5", "x", "x y *" }) == 1);
		REQUIRE(dataCopy.count({ "7", "_", "b c * a +" }) == 1);
	}
}