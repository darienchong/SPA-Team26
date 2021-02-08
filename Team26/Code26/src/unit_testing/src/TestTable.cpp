#include <catch.hpp>

#include "PKB.h"
#include "Table.h"

using Row = typename std::vector<std::string>;

TEST_CASE("[Table.cpp] 1st Test") {
  REQUIRE(1 == 1);
}

TEST_CASE("[TestTable] New Table") {

  SECTION( "new empty table" ) {
    Table table;
    REQUIRE( table.empty() );
  }

  SECTION( "new table with header") {
    std::vector<std::string> header = {"0", "1"};
    Table tableWithHeader(header);
    REQUIRE(tableWithHeader.getHeader() == header);
  }

  SECTION( "set header " ) {
    Table table;
    std::vector<std::string> header = {"0", "1"};
    table.setHeader(header);
    REQUIRE(table.getHeader() == header);
  }
}

TEST_CASE("[TestTable] Insert Data") {

  SECTION ("add data") {
    Table table;
    table.insertRow({"1"});
    table.insertRow({"2"});
    table.insertRow({"3"});
    REQUIRE(!table.empty());
    REQUIRE(table.size() == 3);
    REQUIRE(table.contains({"2"}));
  }

}

TEST_CASE("[Table.cpp] Transitive closure") {
  Table table(2);
  table.insertRow({ "1", "2" });
  table.insertRow({ "2", "3" });
  table.insertRow({ "3", "4" });
  table.insertRow({ "5", "6" });

  Table tTable(2);
  tTable.fillTransitiveTable(table);

  SECTION("Check for existence") {
    REQUIRE(tTable.getData().count({ "1", "2" }) == 1);
    REQUIRE(tTable.getData().count({ "1", "3" }) == 1);
    REQUIRE(tTable.getData().count({ "1", "4" }) == 1);
    REQUIRE(tTable.getData().count({ "2", "3" }) == 1);
    REQUIRE(tTable.getData().count({ "2", "4" }) == 1);
    REQUIRE(tTable.getData().count({ "3", "4" }) == 1);
    REQUIRE(tTable.getData().count({ "5", "6" }) == 1);
  }

  SECTION("Check for non-existence") {
    REQUIRE(tTable.getData().count({ "1", "5" }) == 0);
    REQUIRE(tTable.getData().count({ "3", "2" }) == 0);
    REQUIRE(tTable.getData().count({ "4", "4" }) == 0);
  }
}