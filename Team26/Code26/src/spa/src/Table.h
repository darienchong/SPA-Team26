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
  explicit Table(Row newHeader);

  // setters
  void setHeader(const Row& newHeader);

  // data insertions
  void insertRow(Row row);

  // getters
  Row getHeader() const;
  std::set<Row> getData() const;
  std::set<std::string> getColumn(const std::string& headerTitle) const;
  std::set<Row> getColumns(const Row& headerTitles) const;
  int getColumnIndex(const std::string& headerTitle) const;

  // util functions
  void dropColumn(const std::string& headerTitle);
  void filterColumn(const std::string& columnName, const std::set<std::string>& values);
  void concatenate(Table& otherTable);
  void join(const Table& otherTable);
  void innerJoin(const Table& otherTable, int firstTableIndex, int secondTableIndex);
  void innerJoin(const Table& otherTable, const std::string& commonHeader);
  void deleteRow(const Row& row);

  int size() const;
  bool contains(const Row& row) const;
  bool empty() const;

  // only called by transitive tables
  void fillTransitiveTable(const Table& table);
  void fillIndirectRelation(const Table& parentTable);
};
