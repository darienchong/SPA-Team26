#include "Table.h"
#include <algorithm>
#include <iterator>
#include <map>

  Table::Table() {
    headerRow.emplace_back(std::to_string(0));
  }

  Table::Table(Row header)  {
    headerRow = header;
  }


 int Table::getColumnIndex(std::string columnName) {
   for (int i = 0; i < headerRow.size(); ++i) {
     if (headerRow[i] == columnName) {
       return i;
     }
   }
   throw std::logic_error("Column with name:" + columnName + " not found");
 }

 Table::Row Table::getHeader() const {
     return headerRow;
 }

 void Table::setHeader(Row header) {
   headerRow = header;
 }


 void Table::insertRow(Row row) {
   data.emplace(row);
 }


 std::set<std::string> Table::getColumn(std::string columnName) {
   int index = getColumnIndex(columnName);
   std::set<std::string> column;

   for (auto &row : data) {
     column.emplace(row[index]);
   }
   return column;
 }


  std::set<Table::Row> Table::getData() const {
    return data;
  }

 int Table::size() const {
  return data.size();
  }

 bool Table::contains(Row row) const {
  return data.count(row) == 1;
}

 std::set<Table::Row> Table::getData(Row cols) const {
   std::vector<int> indexList;
   for (auto colName : cols) {
     auto it = std::find(headerRow.begin(), headerRow.end(), colName);
     if (it == headerRow.end()) {
       throw std::logic_error("Column: " + colName + " does not exist");
     }
     indexList.emplace_back(std::distance(headerRow.begin(), it));
   }

   std::set<Row> result;
   for (auto row : data) {
     Row curr;
     for (auto i : indexList) {
       curr.emplace_back(row[i]);
     }
     result.insert(curr);
   }

   return result;
 }

  bool Table::empty() const {
    return data.empty();
  }


  void Table::dropColumn(std::string toDrop) {
    int index = getColumnIndex(toDrop);
    headerRow.erase(headerRow.begin() + index);

    std::set<Row> newData;
    for (auto &row : data) {
      Row curr;
      curr.reserve(row.size() - 1);
      auto it = row.begin();
      std::advance(it, index);
      curr.assign(row.begin(), it++);
      curr.insert(curr.end(), it, row.end());
      newData.emplace_hint(newData.end(), std::move(curr));
    }
    data = std::move(newData);
  }

  void Table::selfJoin() {
    auto it = data.begin();
    while (it != data.end()) {
      if (it->at(0) != it->at(1)) {
        it = data.erase(it);
      } else {
        ++it;
      }
    }
    dropColumn(headerRow[1]);
  }