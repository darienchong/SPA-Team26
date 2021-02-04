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
  int getColumnIndex(std::string columnName);
  Row getHeader() const;
  void setHeader(Row header);
  void insertRow(Row row);
  std::set<std::string> getColumn(std::string header);
  std::set<Row> getData(Row cols) const;
  std::set<Row> getData() const;
  int size() const;
  bool contains(Row row) const;
  bool empty() const;
  void dropColumn(std::string toDrop);
  void selfJoin();
};
