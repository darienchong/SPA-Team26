#include <stack>
#include <stdexcept>
#include <unordered_map>

#include "Table.h"

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
    throw std::logic_error("Header size does not match");
  }
  header = h;
}

void Table::insertRow(Row row) {
  if (row.size() != header.size()) {
    throw std::logic_error("Row should be of length " + std::to_string(header.size()));
  }
  data.emplace(row);
}

Table::Row Table::getHeader() {
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
  for (int i = 0; i < header.size(); i++) {
    if (header[i] == headerTitle) {
      return i;
    }
  }
  throw std::logic_error("Column with name:" + headerTitle + " not found");
}

void Table::dropColumn(std::string headerTitle) {
  int index = getColumnIndex(headerTitle);

  // Clear data if there is only one column, otherwise erase column
  if (header.size() == 1) {
    data.clear();
  } else {
    header.erase(header.begin() + index);
    for (Row row : data) {
      row.erase(row.begin() + index);
    }
  }
}

void Table::concatenate(Table table) {
  if (header.size() != table.getHeader().size()) {
    throw std::logic_error("Concatenation requires table with the same number of columns");
  }
  for (Row row : table.getData()) {
    data.emplace(row);
  }
}


// Used for transitive relationships Follows* and Parent*
void Table::fillTransitiveTable(Table table) {
  std::unordered_map<std::string, std::set<std::string>> adjList;
  std::unordered_map<std::string, std::set<std::string>> tMap;

  for (Row row : table.getData()) {
    if (!(adjList.find(row[0]) != adjList.end())) {
      adjList.insert({ row[0], std::set<std::string>({ row[1] }) });
    } else {
      adjList.at(row[0]).insert(row[1]);
    }
  }

  for (auto entry : adjList) {
    std::set<std::string> visited;
    std::stack<std::string> stack;
    std::string curr;

    stack.push(entry.first);
    while (!stack.empty()) {
      curr = stack.top();
      stack.pop();
      if (!(visited.find(curr) != visited.end())) {
        visited.insert(curr);
        if (adjList.find(curr) != adjList.end()) {
          for (auto value : adjList.at(curr)) {
            stack.push(value);
          }
        }
      }
    }
    tMap.insert({ entry.first, std::move(visited) });
  }

  for (auto entry : tMap) {
    for (auto value : entry.second) {
      data.insert(Row({ entry.first, value }));
    }
  }
}

bool Table::contains(Row row) {
  return data.count(row) == 1;
}

int Table::size() {
  return data.size();
}


bool Table::empty() {
  return data.size() == 0;
}