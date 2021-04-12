#include "Table.h"

#include <assert.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {
  bool areAllHeadersUnique(const Header& newHeader) {
    std::unordered_set<std::string> prevNames;
    for (const std::string& name : newHeader) {
      bool isUnique = (name == "" || prevNames.count(name) == 0);
      if (!isUnique) {
        false;
      }
      prevNames.emplace(name);
    }
    return true;
  }
}

Table::Table() {
  header.emplace_back("");
}

Table::Table(size_t n) : header(std::vector<std::string>(n, "")) {
  assert(n > 0);
}

Table::Table(const Header& newHeader)
  : header(newHeader) {
}

void Table::setHeader(const Header& newHeader) {
  assert(newHeader.size() == header.size());
  assert(areAllHeadersUnique(newHeader));
  header = newHeader;
}

void Table::insertRow(const Row& row) {
  assert(row.size() == header.size());
  data.emplace(row);
}

Header Table::getHeader() const {
  return header;
}

RowSet Table::getData() const {
  return data;
}

size_t Table::getColumnIndex(const std::string& headerTitle) const {
  for (uint32_t i = 0; i < header.size(); i++) {
    if (header[i] == headerTitle) {
      return i;
    }
  }
  return -1;
}

bool Table::dropColumn(const size_t index) {
  const size_t numCols = header.size();
  if (index < 0 || index >= numCols) {
    return false;
  }

  // Cannot remove one column
  if (numCols == 1) {
    return false;
  }

  header.erase(header.begin() + index);
  RowSet newData;
  newData.reserve(size());
  for (Row row : data) {
    row.erase(row.begin() + index);
    newData.emplace(row);
  }
  data = std::move(newData);

  return true;
}

bool Table::dropColumn(const std::string& headerTitle) {
  const size_t index = getColumnIndex(headerTitle);
  if (index == -1) {
    return false;
  }
  return dropColumn(index);
}

void Table::filterColumn(const size_t index, const std::unordered_set<int>& values) {
  const size_t numCols = header.size();
  assert(index >= 0 && index < numCols);
  // Filter table
  for (RowSet::iterator it = data.begin(); it != data.end(); ) {
    if (values.count((*it)[index]) == 0) {
      it = data.erase(it);
    } else {
      it++;
    }
  }
}

void Table::concatenate(Table& otherTable) {
  assert(header.size() == otherTable.header.size());
  data.reserve(size() + otherTable.size());
  data.insert(otherTable.data.begin(), otherTable.data.end());
}

size_t Table::size() const {
  return data.size();
}

bool Table::contains(const Row& row) const {
  return data.count(row) == 1;
}

bool Table::empty() const {
  return data.size() == 0;
}

void Table::naturalJoin(const Table& otherTable) {
  // Get pairs of columns with the same name
  const std::vector<std::pair<size_t, size_t>>& indexPairs = getColumnIndexPairs(otherTable);

  const bool hasCommonHeaders = !indexPairs.empty();
  // If current table and otherTable have common column names, do natural naturalJoin
  // Else, cross join
  if (hasCommonHeaders) {
    innerJoin(otherTable, indexPairs);
  } else {
    crossJoin(otherTable);
  }
}

void Table::crossJoin(const Table& otherTable) {
  RowSet newData;
  newData.reserve(size() * otherTable.size());
  const Header& otherHeader = otherTable.header;
  const size_t newColNum = header.size() + otherHeader.size();
  header.reserve(newColNum);
  header.insert(header.end(), otherHeader.begin(), otherHeader.end());
  for (const Row& row : data) {
    for (const Row& otherRow : otherTable.getData()) {
      Row newRow;
      newRow.reserve(newColNum);
      newRow.insert(newRow.end(), row.begin(), row.end());
      newRow.insert(newRow.end(), otherRow.begin(), otherRow.end());
      newData.emplace(std::move(newRow));
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, 
  const std::vector<std::pair<size_t, size_t>>& indexPairs) {

  const Header& otherHeader = otherTable.header;
  const size_t otherHeaderSize = otherHeader.size();
  const size_t commonColNum = indexPairs.size();

  std::vector<size_t> thisTableCommonColIdxs;
  thisTableCommonColIdxs.reserve(commonColNum);
  std::unordered_set<size_t> thisTableCommonColIdxSet;
  thisTableCommonColIdxSet.reserve(commonColNum);
  std::vector<size_t> otherTableCommonColIdxs;
  otherTableCommonColIdxs.reserve(commonColNum);
  std::unordered_set<size_t> otherTableCommonColIdxSet;
  otherTableCommonColIdxSet.reserve(commonColNum);
  std::vector<size_t> otherTableIdxsToJoin;
  otherTableIdxsToJoin.reserve(otherHeaderSize - commonColNum);

  for (const std::pair<size_t, size_t>& pair : indexPairs) {
    thisTableCommonColIdxs.emplace_back(pair.first);
    thisTableCommonColIdxSet.emplace(pair.first);
    otherTableCommonColIdxs.emplace_back(pair.second);
    otherTableCommonColIdxSet.emplace(pair.second);
  }

  // Add other headers not found in common header
  for (size_t i = 0; i < otherHeaderSize; i++) {
    if (otherTableCommonColIdxSet.count(i) == 0) {
      header.emplace_back(otherHeader[i]);
      otherTableIdxsToJoin.emplace_back(i);
    }
  }

  const size_t newColNum = header.size();
  const bool isOtherTableSmaller = otherTable.size() < size();

  // Set smaller table references to LHS and larger table references to RHS
  const RowSet& lhsTableData = isOtherTableSmaller ? otherTable.data : data;
  const RowSet& rhsTableData = isOtherTableSmaller ? data : otherTable.data;
  const std::vector<size_t>& lhsTableCommonColIdxs = isOtherTableSmaller
    ? otherTableCommonColIdxs
    : thisTableCommonColIdxs;
  const std::unordered_set<size_t>& lhsTableCommonColIdxSet = isOtherTableSmaller
    ? otherTableCommonColIdxSet
    : thisTableCommonColIdxSet;
  const std::vector<size_t>& rhsTableCommonColIdxs = isOtherTableSmaller
    ? thisTableCommonColIdxs
    : otherTableCommonColIdxs;
  const std::unordered_set<size_t>& rhsTableCommonColIdxSet = isOtherTableSmaller
    ? thisTableCommonColIdxSet
    : otherTableCommonColIdxSet;
  const size_t smallerTableSize = lhsTableData.size();


  // Build phase: construct hash table mapping from common attributes to rows
  std::unordered_map<Row, std::vector<Row>, RowHash> hashmap;
  hashmap.reserve(smallerTableSize);
  for (const Row& lhsTableRow : lhsTableData) {
    Row key;
    key.reserve(commonColNum);

    for (const size_t i : lhsTableCommonColIdxs) {
      key.emplace_back(lhsTableRow[i]);
    }
    hashmap[key].emplace_back(lhsTableRow);
  }

  // Probe phase: find relevant rows in hash table
  RowSet newData;
  newData.reserve(smallerTableSize);

  for (const Row& rhsTableRow : rhsTableData) {
    Row key;
    key.reserve(commonColNum);
    for (const size_t rhsIdx : rhsTableCommonColIdxs) {
      key.emplace_back(rhsTableRow[rhsIdx]);
    }

    // Do not join if key doesnt match
    if (hashmap.count(key) == 0) {
      continue;
    }

    // Do cross product on relevant rows
    const std::vector<Row> lhsTableRows = hashmap.at(key);
    for (const Row& lhsTableRow : lhsTableRows) {
      const Row& thisTableRow = isOtherTableSmaller ? rhsTableRow : lhsTableRow;
      const Row& otherTableRow = isOtherTableSmaller ? lhsTableRow : rhsTableRow;

      Row newRow;
      newRow.reserve(newColNum);
      newRow.insert(newRow.end(), thisTableRow.begin(), thisTableRow.end());

      for (size_t otherTableIdx : otherTableIdxsToJoin) {
        newRow.emplace_back(otherTableRow[otherTableIdx]);
      }
      newData.emplace(std::move(newRow));
    }
  }
  data = std::move(newData);
}

void Table::innerJoin(const Table& otherTable, size_t thisTableIndex, size_t otherTableIndex) {
  innerJoin(otherTable, std::vector<std::pair<size_t, size_t>>{ { thisTableIndex, otherTableIndex } });
}

void Table::innerJoin(const Table& otherTable, const std::string& commonHeader) {
  const size_t firstTableIndex = getColumnIndex(commonHeader);
  const size_t secondTableIndex = otherTable.getColumnIndex(commonHeader);
  assert(firstTableIndex != -1 && secondTableIndex != -1);
  innerJoin(otherTable, firstTableIndex, secondTableIndex);
}

bool Table::deleteRow(const Row& row) {
  const size_t numRowErased = data.erase(row);
  return numRowErased == 1;
}

std::vector<std::pair<size_t, size_t>> Table::getColumnIndexPairs(const Table& otherTable) const {
  std::vector<std::pair<size_t, size_t>> indexPairs;

  // Create mapping of header to index for other header
  const Header& otherHeader = otherTable.getHeader();
  const uint32_t otherHeaderSize = otherHeader.size();
  std::unordered_map<std::string, size_t> otherHeaderToOtherIdx;
  otherHeaderToOtherIdx.reserve(otherHeaderSize);
  for (uint32_t i = 0; i < otherHeaderSize; i++) {
    otherHeaderToOtherIdx[otherHeader[i]] = i;
  }

  // Iterate through this header and form index pairs for non-empty headers
  for (uint32_t i = 0; i < header.size(); ++i) {
    const bool isInOtherHeader =
      header[i] != "" &&
      otherHeaderToOtherIdx.count(header[i]) == 1;
    if (isInOtherHeader) {
      indexPairs.emplace_back(i, otherHeaderToOtherIdx.at(header[i]));
    }
  }
  return indexPairs;
}