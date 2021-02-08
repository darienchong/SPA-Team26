#include <catch.hpp>

#include "PKB.h"
#include "Table.h"


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
