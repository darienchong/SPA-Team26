#include "DesignExtractor.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>

#include "Table.h"
#include "Pkb.h"

namespace {
  static const std::string TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG =
    "[DesignExtractor::generateTransitiveClosure]: Table must have two columns.";

  /**
   * Helper method to insert a key-value pair into the adjacency list.
   * Handles edge cases e.g. first insertion (creating the set).
   *
   * @param adjList The adjacency list to modify.
   * @param key The key in the key-value pair.
   * @param value The value in the key-value pair.
   * @returns
   */
  void insertIntoAdjList(std::map<std::string, std::set<std::string>>& adjList,
    std::string key, std::string value) {
    bool isFirstTimePlacingIntoAdjList = (adjList.count(key) == 0);
    if (isFirstTimePlacingIntoAdjList) {
      std::set<std::string> temp{ value };
      adjList.emplace(key, temp);
    } else {
      adjList.at(key).insert(value);
    }
  }

  /**
   * This construction is because our adjacency list is
   * used as a 2d matrix of boolean entries, but is structurally a map, hence
   * the need for these wrappers.
   * 
   * @param adjList The adjacency list to use.
   * @param key The key to use.
   * @param value The value to use.
   * @returns `true` if the given key-value pair is in the adjacency list,
   *     `false` otherwise.
   */
  bool getFromAdjList(std::map<std::string, std::set<std::string>>& adjList,
    std::string key, std::string value) {
    
    bool isKeyNotInAdjList = (adjList.count(key) == 0);
    if (isKeyNotInAdjList) {
      return false;
    }

    std::set<std::string> setOfPossibleValues = adjList.at(key);
    bool isValueNotInSet = (setOfPossibleValues.count(value) == 0);
    if (isValueNotInSet) {
      return false;
    }

    return true;
  }

  /**
   * Overload for ease of use.
   */
  bool getFromAdjList(std::map<std::string, std::set<std::string>>& adjList,
    int key, int value) {
    return getFromAdjList(adjList, std::to_string(key), std::to_string(value));
  }

  /**
   * Helper method for implementation of Warshall's algorithm.
   * Performs row-wise a_i := a_i OR a_j.
   * 
   * @param adjList The adjacency list to use.
   * @param i The number to use for index i.
   * @param j The number to use for index j.
   * @param n The maximum row length (table size).
   */
  void warshallRowOperation(std::map<std::string, std::set<std::string>>& adjList,
    int i, int j, int n) {
    std::string iAsString = std::to_string(i);
    std::string jAsString = std::to_string(j);

    // Handle the simple cases first.
    // If there is no row at j (i.e. every entry in the jth row is 0)
    // then we do nothing (since x OR 0 = x)
    // If there is no row at i (i.e. every entry in the ith row is 0)
    // then we copy over the jth row to the ith row.
    bool isIthRowEmpty = (adjList.count(iAsString) == 0);
    bool isJthRowEmpty = (adjList.count(jAsString) == 0);

    if (isJthRowEmpty) {
      return;
    }

    if (isIthRowEmpty) {
      std::set<std::string> copyOfJthRow = adjList.at(jAsString);
      adjList.emplace(iAsString, copyOfJthRow);
      return;
    }

    // Otherwise, both the ith and jth rows are non-empty.
    // We perform 'bitwise' operations (one entry at a time).
    std::set<std::string> newIthRow;
    for (int k = 1; k <= n; k++) {
      bool ithRowKthEntry = getFromAdjList(adjList, i, k);
      bool jthRowKthEntry = getFromAdjList(adjList, j, k);
      bool bitwiseOr = ithRowKthEntry || jthRowKthEntry;
      if (bitwiseOr) {
        newIthRow.insert(std::to_string(k));
      }
    }
    adjList.erase(iAsString);
    adjList.emplace(iAsString, newIthRow);
  }

  /**
   * Uses Warshall's Algorithm to populate the adjacency list given.
   * See https://www.dartmouth.edu/~matc/DiscreteMath/V.6.pdf for more
   * details.
   * 
   * @param adjList The adjacency list to use/modify.
   * @param n The total number of statements (vertices in graph).
   */
  void applyWarshallAlgorithm(std::map<std::string, std::set<std::string>>& adjList, int n) {
    for (int j = 1; j <= n; j++) {
      for (int i = 1; i <= n; i++) {
        if (getFromAdjList(adjList, i, j)) {
          warshallRowOperation(adjList, i, j, n);
        }
      }
    }
  }

  /**
   * Given a table, generates the transitive closure
   * of the table. We assume that the table tabulates
   * a binary relation (call it R) where
   * 
   *     <x, y> in table <=> R(x, y) holds.
   * 
   * This method generates a table that forms the 
   * transitive closure of the table i.e.
   * 
   *     for all x, y, z, if R(x, y) and R(y, z)
   *     then R(x, z)
   * 
   * @param table The table to use when generating
   *     the transitive closure.
   * @returns A Table with entries as detailed above.
   */
  Table generateTransitiveClosure(Table table) {
    Table newTable = table;
    if (table.getHeader().size() != 2) {
      throw std::invalid_argument(TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG);
    }

    // We insert the initial relations into the adjacency list
    // for use in the Warshall algorithm later.
    std::map<std::string, std::set<std::string>> adjList;
    for (std::vector<std::string> row : table.getData()) {
      insertIntoAdjList(adjList, row[0], row[1]);
    }

    // We don't know how many statements are numbered
    // in the table, but we do know that an upper bound
    // for the number of statements in the table
    // is the number of rows in the table, since 
    // in the upper bound each row enumerates a unique
    // statement. 
    int tableSize = table.getData().size();
    applyWarshallAlgorithm(adjList, tableSize);

    // For each true entry in the transitive
    // closure matrix, add a new row to the 
    // table to return.
    for (int i = 1; i <= tableSize; i++) {
      for (int j = 1; j <= tableSize; j++) {
        if (getFromAdjList(adjList, i, j)) {
          std::vector<std::string> newRow = { std::to_string(i), std::to_string(j) };
          newTable.insertRow(newRow);
        }
      }
    }

    return newTable;
  }

  /**
   * Given the Parent() relations between statements, writes in all the
   * transitive Parent*() relations.
   * 
   * Pre-condition: Requires that all the Parent() relations be populated
   * in the PKB first.
   * 
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillParentTTable(Pkb& pkb) {
    Table parentTTable = generateTransitiveClosure(pkb.getParentTable());
    for (std::vector<std::string> row : parentTTable.getData()) {
      int parent = std::stoi(row[0]);
      int child = std::stoi(row[1]);
      pkb.addParentT(parent, child);
    }
  }
  
  /**
   * Given the Follows() relations between statements, writes in all the
   * transitive Follows*() relations.
   *
   * Pre-condition: Requires that all the Follows() relations be populated
   * in the PKB first.
   *
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillFollowsTTable(Pkb& pkb) {
    Table followsTTable = generateTransitiveClosure(pkb.getFollowsTable());
    for (std::vector<std::string> row : followsTTable.getData()) {
      int followed = std::stoi(row[0]);
      int follower = std::stoi(row[1]);
      pkb.addFollowsT(followed, follower);
    }
  }

  /**
   * .
   */
  void fillUsesTable(Pkb& pkb) {

  }

  /**
   * .
   */
  void fillModifiesTable(Pkb& pkb) {

  }
}

DesignExtractor::DesignExtractor(Pkb pkb) : pkb(pkb) {
}

DesignExtractor::~DesignExtractor() {
}

void DesignExtractor::extractDesignAbstractions() {
  fillParentTTable(pkb);
  fillFollowsTTable(pkb);
}