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
  /**
   * Constructor for Table.
   */
  Table();

  /**
   * Constructor for Table with specified number of columns.
   *
   * @param n The number of columns.
   */
  explicit Table(int n);

  /**
   * Constructor for Table with specified headers.
   *
   * @param newHeader A vector of strings corresponding to the header titles.
   */
  explicit Table(Row newHeader);

  /**
   * Replaces the current headers with new headers.
   *
   * @param newHeader A vector of strings corresponding to the header titles.
   */
  void setHeader(const Row& newHeader);

  /**
   * Inserts a new row into Table.
   *
   * @param row The new row of data to be inserted.
   */
  void insertRow(Row row);

  /**
   * @return The headers of the Table.
   */
  Row getHeader() const;

  /**
   * @return The data of the table.
   */
  std::set<Row> getData() const;

  /**
   * Returns a column of the Table under the specified header.
   *
   * @param headerTitle The specified header.
   * @return The column data under the header.
   */
  std::set<std::string> getColumn(const std::string& headerTitle) const;

  /**
   * Returns a column of the Table under the specified header.
   *
   * @param headerTitle The specified header.
   * @return The column data under the header.
   */
  std::set<Row> getColumns(const Row& headerTitles) const;

  /**
   * Returns the column index of the Table under the specified header.
   *
   * @param headerTitle The specified header.
   * @return The index of the column under the header.
   */
  int getColumnIndex(const std::string& headerTitle) const;

  /**
   * Returns a list of pairs which have the same header titles.
   * The pair contains two integers. The first integer refers to the column index
   * of the first table. The second integer refers to that of the second table.
   *
   * @param otherTable The second table to be compared with the first table.
   * @return A list of integer pairs corresponding to the column index of the tables.
   */
  std::vector<std::pair<int, int>> getColumnIndexPairs(const Table& otherTable) const;

  /**
   * Deletes a column from the Table at the specified index.
   * This method also deletes the header, reducing the number of columns of Table by 1.
   *
   * @param index The column index of the Table.
   */
  void dropColumn(int index);

  /**
   * Deletes a column from the Table at the index corresponding to the specified
   * header title.
   * This method also deletes the header, reducing the number of columns of Table by 1.
   *
   * @param headerTitle The specified column header.
   */
  void dropColumn(const std::string& headerTitle);

  /**
   * Filter the table rows based on the values for a particular column.
   * If the values from the column at the specified index in the table matches any of the
   * specified values, that particular row would be preserved.
   * Otherwise, if there is no match, that row would be deleted.
   *
   * @param index The column index to be based on for filtering the Table.
   * @param values A set of values to be checked upon when filtering the Table.
   */
  void filterColumn(int index, const std::set<std::string>& values);

  /**
   * Filter the table rows based on the values for a particular column.
   * If the values from the column at the specified header in the table matches any of the
   * specified values, that particular row would be preserved.
   * Otherwise, if there is no match, that row would be deleted.
   *
   * @param columnName The header to be based on for filtering the Table.
   * @param values A set of values to be checked upon when filtering the Table.
   */
  void filterColumn(const std::string& columnName, const std::set<std::string>& values);

  /**
   * Concatenates two tables with the same header size.
   * The data from the other table is appended into the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   */
  void concatenate(Table& otherTable);

  /**
   * Joins two tables using natural join.
   * 
   * The data from the other table would be added to the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   */
  void naturalJoin(const Table& otherTable);

  /**
   * Joins two tables using cross join.
   * 
   * The data from the other table would be added to the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   */
  void crossJoin(const Table& otherTable);

  /**
   * Joins two tables using inner join based on the specified index pairs.
   * The index pairs must contain the index of the first table and then the second table.
   * 
   * The data from the other table would be added to the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   * @param indexPairs The index pairs representing the column index for each table.
   */
  void innerJoin(const Table& otherTable, std::vector<std::pair<int, int>>& indexPairs);

  /**
   * Joins two tables using inner join based on the specified indices.
   *
   * The data from the other table would be added to the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   * @param firstTableIndex The index of the original table.
   * @param secondTableIndex The index of the other table.
   */
  void innerJoin(const Table& otherTable, int firstTableIndex, int secondTableIndex);

  /**
   * Joins two tables using inner join based on the specified common header.
   * If the specified header is not common to the two Tables, an error is thrown.
   *
   * The data from the other table would be added to the original table.
   * The other table remains unaltered.
   *
   * @param otherTable The other table.
   * @param commonHeader The common header for both Tables.
   */
  void innerJoin(const Table& otherTable, const std::string& commonHeader);

  /**
   * Deletes a row from the Table.
   * 
   * @param row The specified row.
   */
  void deleteRow(const Row& row);

  /**
   * @return Returns the number of rows of the Table data.
   */
  int size() const;

  /**
   * @param row The specified row.
   * @return Returns true if the Table data contains the specified row. Otherwise, returns false.
   */
  bool contains(const Row& row) const;

  /**
   * @return Returns true if the Table data is empty. Otherwise, returns false.
   */
  bool empty() const;

  /**
   * Updates the transitive table based on the non-transitive table.
   * 
   * The caller Table should match its type. For example, if the caller is FollowsTTable,
   * the argument should be FollowsTable.
   *
   * @param table The non-transitive table.
   */
  void fillTransitiveTable(const Table& table);
};
