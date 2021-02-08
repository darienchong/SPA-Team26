#include <catch.hpp>
#include "Table.h"

TEST_CASE("[TestTable] New Table") {

  SECTION( "new empty table" ) {
    Table table;
    REQUIRE(table.getHeader() == (std::vector<std::string>){"0"});
    REQUIRE(table.empty());
  }

  SECTION( "new table with header") {
    std::vector<std::string> header = {"0", "1"};
    Table tableWithHeader(header);
    REQUIRE(tableWithHeader.getHeader() == header);
    REQUIRE(tableWithHeader.getColumnIndex("1") == 1);
    REQUIRE_THROWS_WITH(tableWithHeader.getColumnIndex("a"), Catch::Contains("a"));
  }

  SECTION( "new table with given header length") {
    Table tableWithHeader(2);
    REQUIRE(tableWithHeader.getHeader().size() == 2);
    REQUIRE(tableWithHeader.getHeader() == (std::vector<std::string>){"0", "1"});
    REQUIRE(tableWithHeader.getColumnIndex("1") == 1);
  }

  SECTION( "set header " ) {
    Table table;
    std::vector<std::string> header = {"0", "1"};
    table.setHeader(header);
    REQUIRE(table.getHeader() == header);
  }
}

TEST_CASE("[TestTable] Insert Data") {

  SECTION ("valid insertion") {
    Table table;
    table.insertRow({"1"});
    table.insertRow({"2"});
    table.insertRow({"3"});
    REQUIRE(!table.empty());
    REQUIRE(table.size() == 3);
    REQUIRE(table.contains({"2"}));
  }

  SECTION ("invalid insertion") {
    Table table;
    REQUIRE_THROWS(table.insertRow({"1", "2"}));
  }

}

TEST_CASE("[TestTable] Get Data") {

  SECTION ("one column") {
    Table table;
    table.insertRow({"1"});
    table.insertRow({"2"});
    table.insertRow({"3"});
    REQUIRE(table.getData().size() == 3);
    REQUIRE(table.getData().count({"1"}));
  }

  SECTION ("two columns") {
    Table table{2};
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    table.insertRow({"3", "33"});
    REQUIRE(table.getData().count({"1", "11"}));
    REQUIRE(table.getDataWithColumns({"1"}).count({"33"}));
    REQUIRE(!table.getDataWithColumns({"1"}).count({"3"}));
    REQUIRE(table.getColumn("0") == (std::set<std::string>){"1", "2", "3"});
  }

  SECTION ("invalid column name") {
    Table table(2);
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    REQUIRE_THROWS_WITH(table.getDataWithColumns({"4"}), Catch::Contains("4"));
  }

}

TEST_CASE("[TestTable] Drop Column") {
  Table table({"a", "b"});
  table.insertRow({"1", "11"});
  table.insertRow({"2", "22"});
  table.dropColumn("a");
  REQUIRE(table.getHeader() == (std::vector<std::string>){"b"});
  table.dropColumn("b");
  REQUIRE(table.empty());
}

TEST_CASE("[TestTable] Concatenate") {
  SECTION ("valid concatenation") {
    Table table1(2);
    table1.insertRow({"1", "11"});
    table1.insertRow({"2", "22"});
    Table table2(2);
    table2.insertRow({"3", "33"});
    table1.concatenate(table2);
    REQUIRE(table1.contains({"3", "33"}));
  }

  SECTION ("invalid concatenation") {
    Table table1(2);
    table1.insertRow({"1", "11"});
    table1.insertRow({"2", "22"});
    Table table2(3);
    table2.insertRow({"3", "33", "333"});
    REQUIRE_THROWS(table1.concatenate(table2));
  }


}
