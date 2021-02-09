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
  Table(int n);
  Table(Row header);

  // setters
  void setHeader(Row header);

  // data insertions
  void insertRow(Row row);

  // getters
  Row getHeader();
  std::set<Row> getData();
  std::set<std::string> getColumn(std::string headerTitle);
  std::set<Row> getColumns(Row headerTitles);
  int getColumnIndex(std::string headerTitle);

  // util functions
  void dropColumn(std::string headerTitle);
  void concatenate(Table otherTable);

  int size();
  bool contains(Row row);
  bool empty();

  // only called by transitive tables
  void fillTransitiveTable(Table table);
};
