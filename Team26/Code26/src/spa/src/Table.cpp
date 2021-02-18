#include "Table.h"

#include <string>
#include <stack>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>

Table::Table() {
  header.emplace_back("0");
}

Table::Table(int n) {
  for (int i = 0; i < n; i++) {
    header.push_back(std::to_string(i));
  }
}

Table::Table(Row newHeader) {
  header = std::move(newHeader);
}

void Table::setHeader(const Row& newHeader) {
  std::set<std::string> prevNames;
  for (std::string name : newHeader) {
    bool isDuplicate = (name != "") && prevNames.count(name);
    if (isDuplicate) {
      throw "Non-empty  column name could not be duplicated";
    } else {
      prevNames.emplace(name);
    }
  }

  if (newHeader.size() != header.size()) {
    throw "Header size does not match";
  }
  header = newHeader;
}

void Table::insertRow(Row row) {
  if (row.size() != header.size()) {
    throw "Row should be of length " + std::to_string(header.size());
  }
  data.emplace(row);
}

Table::Row Table::getHeader() const {
  return header;
}

std::set<Table::Row> Table::getData() const {
  return data;
}

std::set<std::string> Table::getColumn(const std::string& headerTitle) const {
  int index = getColumnIndex(headerTitle);
  std::set<std::string> column;
  for (Row row : data) {
    column.emplace(row[index]);
  }
  return column;
}

std::set<Table::Row> Table::getColumns(const Row& headerTitles) const {
  std::vector<int> indexList;
  for (const std::string& headerTitle : headerTitles) {
    indexList.push_back(getColumnIndex(headerTitle));
  }

  std::set<Row> result;
  for (Row row : data) {
    Row curr;
    for (int i : indexList) {
      curr.push_back(row[i]);
    }
    result.insert(curr);
  }
  return result;
}

int Table::getColumnIndex(const std::string& headerTitle) const {
  for (size_t i = 0; i < header.size(); i++) {
    if (header[i] == headerTitle) {
      return i;
    }
  }
  throw "Column with name:" + headerTitle + " not found";
}

void Table::dropColumn(int index) {
  // Clear data if there is only one column, otherwise erase column
  if (header.size() == 1) {
    data.clear();
  } else {
    header.erase(header.begin() + index);
    std::set<Row> newData;
    for (Row row : data) {
      row.erase(row.begin() + index);
      newData.emplace(row);
    }
    data = std::move(newData);
  }
}

void Table::dropColumn(const std::string& headerTitle) {
  int index = getColumnIndex(headerTitle);
  dropColumn(index);
}

void Table::filterColumn(int index, const std::set<std::string>& values) {
  for (auto it = data.begin(); it != data.end(); ) {
    if (!values.count(it->at(index))) {
      it = data.erase(it);
    } else {
      it++;
    }
  }
}

void Table::filterColumn(const std::string& columnName, const std::set<std::string>& values) {
  int index = getColumnIndex(columnName);
  filterColumn(index, values);
}

void Table::concatenate(Table& otherTable) {
  if (header.size() != otherTable.getHeader().size()) {
    throw "Concatenation requires table with the same number of columns";
  }
  for (Row row : otherTable.getData()) {
    data.emplace(row);
  }
}

bool Table::contains(const Row& row) const {
  return data.count(row) == 1;
}

int Table::size() const {
  return data.size();
}

bool Table::empty() const {
  return data.size() == 0;
}


// If current table and otherTable have common column names, do natural naturalJoin
// else, cross product
void Table::naturalJoin(const Table& otherTable) {
  // get pairs of columns with the same name
  std::vector<std::pair<int, int>> indexPairs = getColumnIndexPairs(otherTable);

  if (indexPairs.empty()) { // cross-product if no common headers
    crossJoin(otherTable);
  } else { // natural join -> change to hash naturalJoin ?
    innerJoin(otherTable, indexPairs);
  }
}

std::vector<std::pair<int, int>> Table::getColumnIndexPairs(const Table& otherTable) const {
  std::vector<std::pair<int, int>> indexPairs;
  Row otherHeader = otherTable.getHeader();

  for (size_t i = 0; i < header.size(); ++i) {
    if (header[i] == "") {
      continue;
    }
    bool isInOtherHeader = std::find(otherHeader.begin(), otherHeader.end(), header[i]) != otherHeader.end();
    if (isInOtherHeader) {
      indexPairs.emplace_back(i, otherTable.getColumnIndex(header[i]));
    }
  }
  return indexPairs;
}

void Table::crossJoin(const Table& otherTable) {
  std::set<Row> newData;
  Table::Row otherHeader = otherTable.getHeader();
  header.insert(header.end(), otherHeader.begin(), otherHeader.end());
  for (const Row& row : data) {
    for (Row otherRow : otherTable.data) {
      Row newRow = row;
      newRow.insert(newRow.end(), otherRow.begin(), otherRow.end());
      newData.emplace(newRow);
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, std::vector<std::pair<int, int>>& indexPairs) {
  std::set<Row> newData;
  const Table::Row otherHeader = otherTable.getHeader();
  std::set<int> droppedIndexes;

  for (std::pair<int, int> pair : indexPairs) {
    droppedIndexes.emplace(pair.second);
  }

  for (size_t i = 0; i < otherHeader.size(); i++) {
    if (!droppedIndexes.count(i)) {
      header.emplace_back(otherHeader[i]);
    }
  }

  for (Row row : data) {
    for (Row otherRow : otherTable.data) {
      bool keepRow = true;
      for (std::pair<int, int> pair : indexPairs) {
        bool isMatchingPair = row[pair.first] == otherRow[pair.second];
        if (!isMatchingPair) {
          keepRow = false;
          break;
        }
      }
      if (keepRow) {
        Row newRow = row;
        for (size_t i = 0; i < otherRow.size(); i++) {
          bool isToDrop = droppedIndexes.count(i);
          if (!isToDrop) {
            newRow.emplace_back(otherRow[i]);
          }
        }
        newData.emplace(newRow);
      }
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, int firstTableIndex, int secondTableIndex) {
  std::vector<std::pair<int, int>> indexPairs;
  indexPairs.emplace_back(firstTableIndex, secondTableIndex);
  innerJoin(otherTable, indexPairs);
}

void Table::innerJoin(const Table& otherTable, const std::string& commonHeader) {
  int firstTableIndex = getColumnIndex(commonHeader);
  int secondTableIndex = otherTable.getColumnIndex(commonHeader);
  innerJoin(otherTable, firstTableIndex, secondTableIndex);
}

void Table::deleteRow(const Row& row) {
  auto index = find(data.begin(), data.end(), row);
  if (index != data.end()) {
    data.erase(row);
  } else {
    throw "Row not found when trying to delete row.";
  }
}