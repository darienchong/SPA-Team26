#include "Table.h"

#include <string>
#include <stack>
#include <unordered_set>
#include <stdexcept>
#include <unordered_map>
#include <stdexcept>
#include <utility>

Table::Table() {
  header.emplace_back("0");
}

Table::Table(int n) {
  header.reserve(n); // Optimization to avoid resizing
  for (int i = 0; i < n; i++) {
    header.push_back(std::to_string(i));
  }
}

Table::Table(const Row& newHeader)
  : header(newHeader) {
}

void Table::setHeader(const Row& newHeader) {
  if (newHeader.size() != header.size()) {
    throw std::invalid_argument("Header size does not match");
  }

  std::unordered_set<std::string> prevNames;
  for (std::string name : newHeader) {
    bool isDuplicate = (name != "") && prevNames.count(name);
    if (isDuplicate) {
      throw std::invalid_argument("Non-empty column name could not be duplicated");
    } else {
      prevNames.emplace(name);
    }
  }


  header = newHeader;
}

void Table::insertRow(Row row) {
  if (row.size() != header.size()) {
    throw std::invalid_argument("Row should be of length " + std::to_string(header.size()));
  }
  data.emplace(row);
}

Row Table::getHeader() const {
  return header;
}

RowSet Table::getData() const {
  return data;
}

std::unordered_set<std::string> Table::getColumn(const std::string& headerTitle) const {
  int index = getColumnIndex(headerTitle);
  std::unordered_set<std::string> column;
  for (Row row : data) {
    column.emplace(row[index]);
  }
  return column;
}

RowSet Table::getColumns(const Row& headerTitles) const {
  std::vector<int> indexList;
  indexList.reserve(headerTitles.size()); // Optimization to avoid resizing
  for (const std::string& headerTitle : headerTitles) {
    indexList.push_back(getColumnIndex(headerTitle));
  }

  RowSet result;
  result.reserve(size()); // Optimization to avoid rehashing
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
  throw std::invalid_argument("Column with name:" + headerTitle + " not found");
}

void Table::dropColumn(int index) {
  // Clear data if there is only one column, otherwise erase column
  if (header.size() == 1) {
    data.clear();
  } else {
    header.erase(header.begin() + index);
    RowSet newData;
    newData.reserve(size()); // Optimization to avoid rehashing
    for (Row row : data) {
      row.erase(row.begin() + index);
      newData.emplace(row);
    }
    data = std::move(newData);
  }
}

void Table::dropColumn(const std::string& headerTitle) {
  const int index = getColumnIndex(headerTitle);
  dropColumn(index);
}

void Table::filterColumn(int index, const std::unordered_set<std::string>& values) {
  for (auto it = data.begin(); it != data.end(); ) {
    if (!values.count(it->at(index))) {
      it = data.erase(it);
    } else {
      it++;
    }
  }
}

void Table::filterColumn(const std::string& columnName, const std::unordered_set<std::string>& values) {
  const int index = getColumnIndex(columnName);
  filterColumn(index, values);
}

void Table::concatenate(Table& otherTable) {
  if (header.size() != otherTable.getHeader().size()) {
    throw std::invalid_argument("Concatenation requires table with the same number of columns");
  }
  for (const Row& row : otherTable.getData()) {
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
  const std::vector<std::pair<int, int>>& indexPairs = getColumnIndexPairs(otherTable);

  const bool hasCommonHeaders = !indexPairs.empty();
  if (hasCommonHeaders) {
    innerJoin(otherTable, indexPairs);
  } else {
    crossJoin(otherTable);
  }
}

std::vector<std::pair<int, int>> Table::getColumnIndexPairs(const Table& otherTable) const {
  std::vector<std::pair<int, int>> indexPairs;
  const Row& otherHeader = otherTable.getHeader();

  for (size_t i = 0; i < header.size(); ++i) {
    if (header[i] == "") {
      continue;
    }
    const bool isInOtherHeader = std::find(otherHeader.begin(), otherHeader.end(), header[i]) != otherHeader.end();
    if (isInOtherHeader) {
      indexPairs.emplace_back(i, otherTable.getColumnIndex(header[i]));
    }
  }
  return indexPairs;
}

void Table::crossJoin(const Table& otherTable) {
  RowSet newData;
  newData.reserve(size() * otherTable.size()); // Optimization to avoid resizing
  const Row& otherHeader = otherTable.getHeader();
  header.insert(header.end(), otherHeader.begin(), otherHeader.end());
  for (const Row& row : data) {
    for (const Row& otherRow : otherTable.getData()) {
      Row newRow = row;
      newRow.insert(newRow.end(), otherRow.begin(), otherRow.end());
      newData.emplace(newRow);
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, const std::vector<std::pair<int, int>>& indexPairs) {
  RowSet newData;
  const Row& otherHeader = otherTable.getHeader();
  std::vector<int> droppedSecondIndexes;
  std::vector<int> droppedFirstIndexes;

  for (std::pair<int, int> pair : indexPairs) {
    droppedFirstIndexes.emplace_back(pair.first);
    droppedSecondIndexes.emplace_back(pair.second);
  }

  for (size_t i = 0; i < otherHeader.size(); i++) {
    bool isInToDrop = std::find(droppedSecondIndexes.begin(), droppedSecondIndexes.end(), i) != droppedSecondIndexes.end();
    if (!isInToDrop) {
      header.emplace_back(otherHeader[i]);
    }
  }

  // Build phase: construct hash table mapping from common attributes to rows
  std::unordered_map<int, std::vector<Row>> hashmap;
  for (const Row& row : data) {
    std::string concatStr;
    for (int i : droppedFirstIndexes) {
      concatStr.append(row[i] + " ");
    }
    int key = std::hash<std::string>{}(concatStr);

    bool isInMap = hashmap.find(key) != hashmap.end();
    if (!isInMap) {
      std::vector<Row> vect{ row };
      hashmap[key] = vect;
    }
    else {
      hashmap.at(key).emplace_back(row);
    }
  }

  // Probe phase: find relevant rows in hash table
  for (const Row& otherRow : otherTable.getData()) {
    std::string concatStr;
    for (int i : droppedSecondIndexes) {
      concatStr.append(otherRow[i] + " ");
    }
    int key = std::hash<std::string>{}(concatStr);

    bool isInMap = hashmap.find(key) != hashmap.end();
    if (isInMap) {
      std::vector<Row> rows = hashmap.at(key);
      for (const Row& row : rows) { // do cross product on relevant rows
        Row newRow = row;
        for (size_t i = 0; i < otherHeader.size(); i++) {
          bool isInToDrop = std::find(droppedSecondIndexes.begin(), droppedSecondIndexes.end(), i) != droppedSecondIndexes.end();
          if (!isInToDrop) {
            newRow.emplace_back(otherRow[i]);
          }
        }
        newData.emplace(newRow);
      }
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, int thisTableIndex, int otherTableIndex) {
  std::vector<std::pair<int, int>> indexPairs{ {thisTableIndex, otherTableIndex} };
  innerJoin(otherTable, indexPairs);
}

void Table::innerJoin(const Table& otherTable, const std::string& commonHeader) {
  const int firstTableIndex = getColumnIndex(commonHeader);
  const int secondTableIndex = otherTable.getColumnIndex(commonHeader);
  innerJoin(otherTable, firstTableIndex, secondTableIndex);
}

bool Table::deleteRow(const Row& row) {
  int numRowErased = data.erase(row);
  return numRowErased == 1;
}
