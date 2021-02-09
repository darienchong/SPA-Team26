#pragma once

#include <set>
#include <string>
#include <vector>

class Table {
  typedef std::vector<std::string> Row;

private:
  Row headerRow;
  std::set<Row> data;

public:
  Table();
  explicit Table(Row header);
  explicit Table(int n);
  void setHeader(Row header);
  Row getHeader();
  void insertRow(Row row);
  int getColumnIndex(std::string columnName);
  std::set<std::string> getColumn(std::string header);
  int size();
  bool contains(Row row);
  bool empty();
  std::set<Row> getData();
  std::set<Row> getDataWithColumns(Row columnNames);
  void dropColumn(std::string toDrop);
  void concatenate(Table otherTable);

  // technically only followsTTable and parentTTable are allowed to call this
  void fillTransitiveTable(Table table);
};
