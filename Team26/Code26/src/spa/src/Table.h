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
  std::set<Row> getData() const;
  std::set<std::string> getColumn(std::string headerTitle) const;
  std::set<Row> getColumns(Row headerTitles) const;
  int getColumnIndex(std::string headerTitle) const;

  // util functions
  void dropColumn(std::string headerTitle);
  void filterColumn(std::string columnName, std::set<std::string> values);
  void concatenate(Table& otherTable);
  void join(const Table& otherTable);
  void innerJoin(const Table& otherTable, int firstTableIndex, int secondTableIndex);
  void innerJoin(const Table& otherTable, std::string commonHeader);

  int size() const;
  bool contains(const Row& row) const;
  bool empty() const;

  // only called by transitive tables
  void fillTransitiveTable(Table table);
  void fillIndirectRelation(Table parentTable);
};
