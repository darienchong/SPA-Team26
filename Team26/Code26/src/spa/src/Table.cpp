#include "Table.h"

#include <string>
#include <stack>
#include <set>
#include <stdexcept>
#include <unordered_map>

Table::Table() {
  header.emplace_back("0");
}

Table::Table(int n) {
  for (int i = 0; i < n; i++) {
    header.push_back(std::to_string(i));
  }
}

Table::Table(Row h) {
  header = h;
}

void Table::setHeader(Row h) {
  if (h.size() != header.size()) {
    throw "Header size does not match";
  }
  header = h;
}

void Table::insertRow(Row row) {
  if (row.size() != header.size()) {
    throw "Row should be of length " + std::to_string(header.size());
  }
  data.emplace(row);
}

Table::Row Table::getHeader() const{
  return header;
}

std::set<Table::Row> Table::getData() {
  return data;
}

std::set<std::string> Table::getColumn(std::string headerTitle) {
  int index = getColumnIndex(headerTitle);
  std::set<std::string> column;
  for (Row row : data) {
    column.emplace(row[index]);
  }
  return column;
}

std::set<Table::Row> Table::getColumns(Row headerTitles) {
  std::vector<int> indexList;
  for (auto headerTitle : headerTitles) {
    indexList.push_back(getColumnIndex(headerTitle));
  }

  std::set<Row> result;
  for (Row row : data) {
    Row curr;
    for (auto i : indexList) {
      curr.push_back(row[i]);
    }
    result.insert(curr);
  }
  return result;
}

int Table::getColumnIndex(std::string headerTitle) {
  for (size_t i = 0; i < header.size(); i++) {
    if (header[i] == headerTitle) {
      return i;
    }
  }
  throw "Column with name:" + headerTitle + " not found";
}

void Table::dropColumn(std::string headerTitle) {
  int index = getColumnIndex(headerTitle);

  // Clear data if there is only one column, otherwise erase column
  if (header.size() == 1) {
    data.clear();
  } else {
    header.erase(header.begin() + index);
    std::set<Row> newData;
    for (auto row : data) {
      row.erase(row.begin() + index);
      newData.emplace(row);
    }
    data = std::move(newData);
  }
}

void Table::filterColumn(std::string columnName, std::set<std::string> values) {
  int index = getColumnIndex(columnName);

  for (auto it = data.begin(); it != data.end(); ) {
    if (!values.count(it->at(index))) {
      it = data.erase(it);
    } else {
      it++;
    }
  }

}

void Table::concatenate(Table& otherTable) {
  if (header.size() != otherTable.getHeader().size()) {
    throw "Concatenation requires table with the same number of columns";
  }
  for (Row row : otherTable.getData()) {
    data.emplace(row);
  }
}

// Used for transitive relationships Follows* and Parent*
void Table::fillTransitiveTable(Table table) {
  std::unordered_map<std::string, std::set<std::string>> adjList;

  // generate adjacency list
  for (Row row : table.getData()) {
    if (adjList.count(row[0]) == 0) {
      adjList.insert({ row[0], std::set<std::string>({ row[1] }) });
    } else {
      adjList.at(row[0]).insert(row[1]);
    }
  }

  for (std::pair<const std::string, std::set<std::string>>& entry : adjList) {
    // generate transitives
    std::set<std::string> transitives; // keep track of explored nodes
    std::stack<std::string> stack;
    std::string curr;
    stack.push(entry.first);
    while (!stack.empty()) {
      curr = stack.top();
      stack.pop();
      if (transitives.count(curr) == 0) {
        transitives.insert(curr);
        if (adjList.count(curr) == 1) { // push neighbors
          for (std::string value : adjList.at(curr)) {
            stack.push(value);
          }
        }
      }
    }
    // insert transitives
    std::set<std::string>::iterator it;
    for (it = transitives.begin(); it != transitives.end(); it++) {
      data.insert(Row({ entry.first, *it }));
    }
  }
}

bool Table::contains(const Row& row) {
  return data.count(row) == 1;
}

int Table::size() {
  return data.size();
}

bool Table::empty() {
  return data.size() == 0;
}

void Table::fillIndirectRelation(Table parentTTable) {
  if (header.size() != 2 || parentTTable.header.size() != 2) {
    throw "Tables must have 2 columns.";
  }
  if (header[0] == parentTTable.getHeader()[0] || header[1] == parentTTable.getHeader()[0]) {
    throw "Column name should not be the same as Parent column name.";
  }

  std::string joinedColumnName = header[0];
  Table newParentTTable = parentTTable;
  newParentTTable.setHeader({"Parent", joinedColumnName});
  newParentTTable.join(*this);
  newParentTTable.dropColumn(joinedColumnName);
  this->concatenate(newParentTTable);
}

// If current table and otherTable have common column names, do natural join
// else, cross product
void Table::join(const Table& otherTable) {
  std::vector<std::pair<int, int>> indexPairs; // pairs of columns with the same name
  Row otherHeader = otherTable.getHeader();

  for (int i = 0; i < header.size(); ++i) {
    auto otherIndex = find(otherHeader.begin(), otherHeader.end(), header[i]);
    if (otherIndex != otherHeader.end()) {
      indexPairs.emplace_back(i, otherIndex - otherHeader.begin());
    }
  }

  std::set<Row> newData;
  if (indexPairs.empty()) { // cross-product
    header.insert(header.end(), otherHeader.begin(), otherHeader.end());
    for (auto row : data) {
      for (auto otherRow : otherTable.data) {
        auto newRow = row;
        newRow.insert(newRow.end(), otherRow.begin(), otherRow.end());
        newData.emplace(newRow);
      }
    }

  } else { // natural join -> change to hash join ?
    std::set<int> droppedIndexes;
    for (auto pair : indexPairs) {
      droppedIndexes.emplace(pair.second);
    }
    for (size_t i = 0; i < otherHeader.size(); i++) {
      if (!droppedIndexes.count(i)) {
        header.emplace_back(otherHeader[i]);
      }
    }

    for (auto row : data) {
      for (auto otherRow : otherTable.data) {
        bool keep = true;
        for (auto pair : indexPairs) {
          if (row[pair.first] != otherRow[pair.second]) {
            keep = false;
            break;
          }
        }
        if (keep) {
          Row newRow = row;
          for (size_t i = 0; i < otherRow.size(); i++) {
            if (!droppedIndexes.count(i)) {
              newRow.emplace_back(otherRow[i]);
            }
          }
          newData.emplace(newRow);
        }
      }
    }
  }
  data = std::move(newData);
}
