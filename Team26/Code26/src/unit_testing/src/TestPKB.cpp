#include <catch.hpp>

#include "PKB.h"

TEST_CASE("[PKB.cpp] 1st Test") { 
	REQUIRE(1 == 1);
}

TEST_CASE("[PKB.cpp] varTable Insertion") {
	PKB pkb;
	pkb.addVar("x");
	auto dataCopy = pkb.getVarTable().getData();
	REQUIRE(dataCopy.find({ "x" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] stmtTable Insertion") {
	PKB pkb;
	pkb.addStmt(2);
	auto dataCopy = pkb.getStmtTable().getData();
	REQUIRE(dataCopy.find({ "2" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] procTable Insertion") {
	PKB pkb;
	pkb.addProc("main");
	auto dataCopy = pkb.getProcTable().getData();
	REQUIRE(dataCopy.find({ "main" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] ifTable Insertion") {
	PKB pkb;
	pkb.addIf(12);
	auto dataCopy = pkb.getIfTable().getData();
	REQUIRE(dataCopy.find({ "12" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] whileTable Insertion") {
	PKB pkb;
	pkb.addWhile(15);
	auto dataCopy = pkb.getWhileTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] readTable Insertion") {
	PKB pkb;
	pkb.addRead(15);
	auto dataCopy = pkb.getReadTable().getData();
	REQUIRE(dataCopy.find({ "15" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] printTable Insertion") {
	PKB pkb;
	pkb.addPrint(526);
	auto dataCopy = pkb.getPrintTable().getData();
	REQUIRE(dataCopy.find({ "526" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] assignTable Insertion") {
	PKB pkb;
	pkb.addAssign(32);
	auto dataCopy = pkb.getAssignTable().getData();
	REQUIRE(dataCopy.find({ "32" }) != dataCopy.end());
}

TEST_CASE("[PKB.cpp] followsAll Check") {
	PKB pkb;
	pkb.addFollows(5, 6);
	pkb.addFollows(6, 7);
	pkb.addFollows(7, 8);
}