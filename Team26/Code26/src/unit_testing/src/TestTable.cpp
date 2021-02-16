#include "catch.hpp"

#include "Table.h"

TEST_CASE("[TestTable] New Table") {

  SECTION( "new empty table" ) {
    Table table;
    REQUIRE(table.getHeader() == std::vector<std::string>{"0"});
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
    REQUIRE(tableWithHeader.getHeader() == std::vector<std::string> {"0", "1"});
    REQUIRE(tableWithHeader.getColumnIndex("1") == 1);
  }

}

TEST_CASE("[TestTable] Set Header") {
  SECTION( "valid header" ) {
    Table table(2);
    std::vector<std::string> h{"0", "1"};
    table.setHeader(h);
    REQUIRE(table.getHeader() == h);
  }

  SECTION( "valid header with empty string" ) {
    Table table(2);
    std::vector<std::string> h{"", "a"};
    table.setHeader(h);
    REQUIRE(table.getHeader() == h);
  }

  SECTION( "valid header with duplicated empty string" ) {
    Table table(2);
    std::vector<std::string> h{"", ""};
    table.setHeader(h);
    REQUIRE(table.getHeader() == h);
  }

  SECTION( "invalid header" ) {
    Table table(2);
    std::vector<std::string> h{"a", "a"};
    REQUIRE_THROWS(table.setHeader(h));
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
    REQUIRE(table.getData().count({"1"}) == 1);
  }

  SECTION ("two columns") {
    Table table{2};
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    table.insertRow({"3", "33"});
    REQUIRE(table.getData().count({"1", "11"}) == 1);
    REQUIRE(table.getColumns({"1"}).count({"33"}) == 1);
    REQUIRE(!table.getColumns({"1"}).count({"3"}));
    REQUIRE(table.getColumn("0") == std::set<std::string> { "1", "2", "3" });
  }

  SECTION ("invalid column name") {
    Table table(2);
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    REQUIRE_THROWS_WITH(table.getColumns({"4"}), Catch::Contains("4"));
  }

}

TEST_CASE("[TestTable] Drop Column") {
  Table table({"a", "b"});
  table.insertRow({"1", "11"});
  table.insertRow({"2", "22"});
  table.dropColumn("a");
  REQUIRE(table.getHeader() == std::vector<std::string>{"b"});
  REQUIRE(table.getData().count({ "1","11" }) == 0);
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
    REQUIRE(table1.size() == 3);
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

TEST_CASE("[TestTable] Filter Column") {
  SECTION ("valid filtration") {
    Table table({"a", "b"});
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    table.filterColumn("b", std::set<std::string>{"11"});
    REQUIRE(!table.contains({"2", "22"}));
    REQUIRE(table.contains({"1", "11"}));
  }

  SECTION ("valid filtration with empty filter values") {
    Table table({"a", "b"});
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    table.filterColumn("b", std::set<std::string>{});
    REQUIRE(table.empty());
  }

  SECTION ("valid filtration with non-existent filter values") {
    Table table({"a", "b"});
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    table.filterColumn("a", std::set<std::string>{"3"});
    REQUIRE(table.empty());
  }

  SECTION ("invalid filtration") {
    Table table({"a", "b"});
    table.insertRow({"1", "11"});
    table.insertRow({"2", "22"});
    REQUIRE_THROWS(table.filterColumn("c", std::set<std::string>{"11"}));
  }
}

TEST_CASE("[TestTable] join table") {
  SECTION ("cross product join") {
    Table table1({"a", "b"});
    table1.insertRow({"1", "11"});
    table1.insertRow({"2", "22"});
    Table table2({"c", "d"});
    table2.insertRow({"3", "33"});
    table2.insertRow({"4", "44"});
    table1.join(table2);
    REQUIRE(table1.size() == 4);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c", "d"});
    REQUIRE(table1.contains({ "1", "11", "3", "33" }));
    REQUIRE(table1.contains({ "1", "11", "4", "44" }));
    REQUIRE(table1.contains({ "2", "22", "3", "33" }));
    REQUIRE(table1.contains({ "2", "22", "4", "44" }));
  }

  SECTION("valid cross product join with empty tables") {
    Table table1({ "a", "b" });
    Table table2({ "c", "d" });
    table1.join(table2);
    REQUIRE(table1.size() == 0);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c", "d"});
  }

  SECTION ("natural join one overlapping column") {
    Table table1({"a", "b"});
    table1.insertRow({"1", "11"});
    table1.insertRow({"2", "22"});
    Table table2({"a", "c"});
    table2.insertRow({"1", "33"});
    table2.insertRow({"2", "44"});
    table1.join(table2);
    REQUIRE(table1.size() == 2);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c"});
    REQUIRE(table1.contains({"1", "11", "33"}));
    REQUIRE(table1.contains({"2", "22", "44"}));
  }

  SECTION ("natural join one overlapping empty string column name") {
    Table table1({"", "b"});
    table1.insertRow({"1", "11"});
    table1.insertRow({"2", "22"});
    Table table2({"", "c"});
    table2.insertRow({"1", "33"});
    table2.insertRow({"2", "44"});
    table1.join(table2);
    REQUIRE(table1.size() == 4);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"", "b", "", "c"});
    REQUIRE(table1.contains({"1", "11", "1", "33"}));
    REQUIRE(table1.contains({"2", "22", "2", "44"}));
  }

  SECTION ("natural join two overlapping column") {
    Table table1({"a", "b", "c"});
    table1.insertRow({"1", "11", "33"});
    table1.insertRow({"2", "22", "43"});
    Table table2({"a", "c"});
    table2.insertRow({"1", "33"});
    table2.insertRow({"2", "44"});
    table1.join(table2);
    REQUIRE(table1.size() == 1);
    REQUIRE(table1.getHeader() == std::vector<std::string>{"a", "b", "c"});
    REQUIRE(table1.contains({"1", "11", "33"}));
    REQUIRE(!table1.contains({"2", "22", "43"}));
  }

  SECTION("valid natural join with empty tables") {
    Table table1({ "a", "b" });
    Table table2({ "a", "c" });
    table1.join(table2);
    REQUIRE(table1.size() == 0);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c"});
  }
}

TEST_CASE("[TestTable] inner join with indexes") {
  SECTION("inner join with a common header specified") {
    Table table1({ "a", "b" });
    table1.insertRow({ "1", "11" });
    table1.insertRow({ "2", "22" });
    Table table2({ "a", "c" });
    table2.insertRow({ "1", "33" });
    table2.insertRow({ "2", "44" });
    table1.innerJoin(table2, 0, 0);
    REQUIRE(table1.size() == 2);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c"});
    REQUIRE(table1.contains({ "1", "11", "33" }));
    REQUIRE(table1.contains({ "2", "22", "44" }));
  }
}

TEST_CASE("[TestTable] inner join with column name") {
  SECTION("inner join with a common header specified") {
    Table table1({ "a", "b" });
    table1.insertRow({ "1", "11" });
    table1.insertRow({ "2", "22" });
    Table table2({ "a", "c" });
    table2.insertRow({ "1", "33" });
    table2.insertRow({ "2", "44" });
    table1.innerJoin(table2, "a");
    REQUIRE(table1.size() == 2);
    REQUIRE(table1.getHeader() == std::vector<std::string> {"a", "b", "c"});
    REQUIRE(table1.contains({ "1", "11", "33" }));
    REQUIRE(table1.contains({ "2", "22", "44" }));
  }
}

TEST_CASE("[TestTable] Fill Indirect Relation") {
  SECTION("Valid Input with one indirect relation") {
    Table usesTable({ "stmtRef", "entRef" });
    usesTable.insertRow({ "1", "x" });
    usesTable.insertRow({ "3", "y" });
    Table parentTTable({ "parent", "child" });
    parentTTable.insertRow({ "1", "3" });
    usesTable.fillIndirectRelation(parentTTable);
    REQUIRE(usesTable.size() == 3);
    REQUIRE(usesTable.getHeader() == std::vector<std::string>{"stmtRef", "entRef"});
    REQUIRE(usesTable.contains({ "1", "y" }));
  }

  SECTION("Valid Input with multiple indirect relations") {
    Table usesTable({ "stmtRef", "entRef" });
    usesTable.insertRow({ "3", "y" });
    usesTable.insertRow({ "3", "z" });
    usesTable.insertRow({ "5", "a" });
    Table parentTTable({ "parent", "child" });
    parentTTable.insertRow({ "1", "3" });
    parentTTable.insertRow({ "3", "5" });
    parentTTable.insertRow({ "1", "5" });
    usesTable.fillIndirectRelation(parentTTable);
    REQUIRE(usesTable.size() == 7);
    REQUIRE(usesTable.getHeader() == std::vector<std::string>{"stmtRef", "entRef"});
    REQUIRE(usesTable.contains({ "1", "y" }));
    REQUIRE(usesTable.contains({ "1", "z" }));
    REQUIRE(usesTable.contains({ "1", "a" }));
    REQUIRE(usesTable.contains({ "3", "a" }));
  }

  SECTION("Valid input with no indirect relations") {
    Table usesTable({ "stmtRef", "entRef" });
    usesTable.insertRow({ "3", "y" });
    usesTable.insertRow({ "3", "z" });
    Table parentTTable({ "parent", "child" });
    parentTTable.insertRow({ "1", "4" });
    usesTable.fillIndirectRelation(parentTTable);
    REQUIRE(usesTable.size() == 2);
    REQUIRE(usesTable.getHeader() == std::vector<std::string>{"stmtRef", "entRef"});
  }
}