#pragma once

#include <string>
#include <set>
#include <vector>

class Table {
  typedef std::vector<std::string> Row;

private:
  Row header;
  std::set<Row> data;

public:
  // constructors
  Table();
  explicit Table(int n);
  explicit Table(Row header);

  // setters
  void setHeader(Row header);

  // data insertions
  void insertRow(Row row);

  // getters
  Row getHeader() const;
  std::set<Row> getData();
  std::set<std::string> getColumn(std::string headerTitle);
  std::set<Row> getColumns(Row headerTitles);
  int getColumnIndex(std::string headerTitle);

  // util functions
  void dropColumn(std::string headerTitle);
  void filterColumn(std::string columnName, std::set<std::string> values);
  void concatenate(Table& otherTable);
  void join(const Table& otherTable);

  int size();
  bool contains(const Row& row);
  bool empty();

  // only called by transitive tables
  void fillTransitiveTable(Table table);
  void fillIndirectRelation(Table parentTable);
};
