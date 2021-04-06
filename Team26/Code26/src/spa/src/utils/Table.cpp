#include "Table.h"

#include <assert.h>

#include <string>
#include <stack>
#include <unordered_set>
#include <stdexcept>
#include <unordered_map>
#include <utility>

TableException::TableException(const std::string& msg)
  : std::exception(msg.c_str()) {}

Table::Table() {
  header.emplace_back("");
}

Table::Table(int n) {
  if (n < 1) {
    throw TableException("Cannot initialise Table with non-positive number of columns");
  }
  header.reserve(n); // Optimization to avoid resizing
  for (int i = 0; i < n; i++) {
    header.push_back("");
  }
}

Table::Table(const Header& newHeader)
  : header(newHeader) {
}

void Table::setHeader(const Header& newHeader) {
  if (newHeader.size() != header.size()) {
    throw TableException("Given header size does not match number of columns");
  }

  std::unordered_set<std::string> prevNames;
  for (std::string name : newHeader) {
    const bool isDuplicate = (name != "") && prevNames.count(name);
    if (isDuplicate) {
      throw TableException("Non-empty column name could not be duplicated");
    } else {
      prevNames.emplace(name);
    }
  }

  header = newHeader;
}

void Table::insertRow(Row row) {
  if (row.size() != header.size()) {
    throw TableException("Row should be of length " + std::to_string(header.size()));
  }
  data.emplace(row);
}

Header Table::getHeader() const {
  return header;
}

RowSet Table::getData() const {
  return data;
}

int Table::getColumnIndex(const std::string& headerTitle) const {
  for (uint32_t i = 0; i < header.size(); i++) {
    if (header[i] == headerTitle) {
      return i;
    }
  }
  return -1;
}

bool Table::dropColumn(const int index) {
  if (index < 0 || index >= header.size()) {
    return false;
  }
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
  return true;
}

bool Table::dropColumn(const std::string& headerTitle) {
  const int index = getColumnIndex(headerTitle);
  if (index == -1) {
    return false;
  }
  dropColumn(index);
  return true;
}

void Table::filterColumn(const int index, const std::unordered_set<int>& values) {
  assert(index >= 0 && index < header.size());
  for (auto it = data.begin(); it != data.end(); ) {
    if (!values.count((*it)[index])) {
      it = data.erase(it);
    } else {
      it++;
    }
  }
}

void Table::concatenate(Table& otherTable) {
  if (header.size() != otherTable.header.size()) {
    throw TableException("Concatenation requires table with the same number of columns");
  }
  data.insert(otherTable.data.begin(), otherTable.data.end());
}

int Table::size() const {
  return data.size();
}

bool Table::contains(const Row& row) const {
  return data.count(row) == 1;
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

void Table::crossJoin(const Table& otherTable) {
  RowSet newData;
  newData.reserve(size() * otherTable.size()); // Optimization to avoid resizing
  const Header& otherHeader = otherTable.header;
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
  const Header& otherHeader = otherTable.header;
  std::vector<int> droppedSecondIndexes;
  std::vector<int> droppedFirstIndexes;

  for (const std::pair<int, int>& pair : indexPairs) {
    droppedFirstIndexes.emplace_back(pair.first);
    droppedSecondIndexes.emplace_back(pair.second);
  }

  for (uint32_t i = 0; i < otherHeader.size(); i++) {
    const bool isInToDrop = std::find(droppedSecondIndexes.begin(), droppedSecondIndexes.end(), i) != droppedSecondIndexes.end();
    if (!isInToDrop) {
      header.emplace_back(otherHeader[i]);
    }
  }

  std::unordered_map<int, std::vector<Row>> hashmap;

  const bool isOtherTableSmaller = otherTable.size() < size();
  const RowSet& lhsTableData = isOtherTableSmaller ? otherTable.data : data;
  const RowSet& rhsTableData = isOtherTableSmaller ? data : otherTable.data;
  const std::vector<int>& lhsTableDroppedIndexes = isOtherTableSmaller ? droppedSecondIndexes : droppedFirstIndexes;
  const std::vector<int>& rhsTableDroppedIndexes = isOtherTableSmaller ? droppedFirstIndexes : droppedSecondIndexes;

  // Build phase: construct hash table mapping from common attributes to rows
  for (const Row& lhsTableRow : lhsTableData) {

    std::uint32_t key = lhsTableDroppedIndexes.size();
    for (const uint32_t& i : lhsTableDroppedIndexes) {
      key ^= lhsTableRow[i] + 0x9e3779b9 + (key << 6) + (key >> 2);
    }

    const bool isInMap = hashmap.count(key) == 1;;
    if (!isInMap) {
      hashmap[key] = { lhsTableRow };
    } else {
      hashmap.at(key).emplace_back(lhsTableRow);
    }
  }

  // Probe phase: find relevant rows in hash table
  RowSet newData;
  for (const Row& rhsTableRow : rhsTableData) {
    std::uint32_t key = rhsTableDroppedIndexes.size();
    for (const uint32_t& i : rhsTableDroppedIndexes) {
      key ^= rhsTableRow[i] + 0x9e3779b9 + (key << 6) + (key >> 2);
    }

    const bool isInMap = hashmap.count(key) == 1;;
    if (isInMap) {
      const std::vector<Row> lhsTableRows = hashmap.at(key);
      for (const Row& lhsTableRow : lhsTableRows) { // do cross product on relevant rows
        Row newRow = isOtherTableSmaller ? rhsTableRow : lhsTableRow;
        for (uint32_t i = 0; i < otherHeader.size(); i++) {
          bool isInToDrop = std::find(droppedSecondIndexes.begin(), droppedSecondIndexes.end(), i) != droppedSecondIndexes.end();
          if (!isInToDrop) {
            newRow.emplace_back(isOtherTableSmaller ? lhsTableRow[i] : rhsTableRow[i]);
          }
        }
        newData.emplace(newRow);
      }
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, int thisTableIndex, int otherTableIndex) {
  innerJoin(otherTable, std::vector<std::pair<int, int>>{ { thisTableIndex, otherTableIndex } });
}

void Table::innerJoin(const Table& otherTable, const std::string& commonHeader) {
  const int firstTableIndex = getColumnIndex(commonHeader);
  const int secondTableIndex = otherTable.getColumnIndex(commonHeader);
  innerJoin(otherTable, firstTableIndex, secondTableIndex);
}

bool Table::deleteRow(const Row& row) {
  const int numRowErased = data.erase(row);
  return numRowErased == 1;
}

std::vector<std::pair<int, int>> Table::getColumnIndexPairs(const Table& otherTable) const {
  std::vector<std::pair<int, int>> indexPairs;
  const Header& otherHeader = otherTable.getHeader();

  for (uint32_t i = 0; i < header.size(); ++i) {
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