#include "DesignExtractor.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>

#include "Table.h"
#include "Pkb.h"

namespace {
  static const std::string GENERATE_TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG =
    "[DesignExtractor::generateTransitiveClosure]: Table must have two columns.";

  static const std::string FILL_INDIRECT_RELATION_HEADER_SIZE_NEQ_2_ERROR_MSG =
    "[DesignExtractor::fillIndirectRelation]: Both tables must have two columns.";

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
   * @param The total number of statements.
   * @returns A Table with entries as detailed above.
   */
  Table generateTransitiveClosure(Table table, int numOfStmt) {
    Table newTable = table;
    if (table.getHeader().size() != 2) {
      throw std::invalid_argument(GENERATE_TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG);
    }

    // We insert the initial relations into the adjacency list
    // for use in the Warshall algorithm later.
    std::map<std::string, std::set<std::string>> adjList;
    for (std::vector<std::string> row : table.getData()) {
      insertIntoAdjList(adjList, row[0], row[1]);
    }

    applyWarshallAlgorithm(adjList, numOfStmt);

    // For each true entry in the transitive
    // closure matrix, add a new row to the 
    // table to return.
    for (int i = 1; i <= numOfStmt; i++) {
      for (int j = 1; j <= numOfStmt; j++) {
        if (getFromAdjList(adjList, i, j)) {
          std::vector<std::string> newRow = { std::to_string(i), std::to_string(j) };
          newTable.insertRow(newRow);
        }
      }
    }

    return newTable;
  }

  /**
   * Populates a given table with indirect relations.
   * We have a table with a number of direct relations.
   * We wish to implement a new indirect relation, defined so: where
   * 
   *   R(x, y) indicates an existing relation in the first table,
   *   P(x, y) indicates an existing relation in the second table,
   * 
   * If it is the case that
   * 
   *   1) P(x, y)
   *   2) R(y, z)
   * 
   * Then we have a new indirect relation to be put in the 
   * first table, R(x, z).
   * 
   * @param table The table to use/modify.
   * @param parentTTable The second table to refer to.
   * @returns The table with new indirect relation entries.
   */
  Table fillIndirectRelation(Table table, Table parentTTable) {
    bool isTableColumnCountNotTwo = (table.getHeader().size() != 2) || 
      (parentTTable.getHeader().size() != 2);

    if (isTableColumnCountNotTwo) {
      throw std::invalid_argument(FILL_INDIRECT_RELATION_HEADER_SIZE_NEQ_2_ERROR_MSG);
    }

    Table newParentTTable = parentTTable;
    newParentTTable.innerJoin(table, 1, 0);
    newParentTTable.dropColumn(1);
    table.concatenate(newParentTTable);

    return table;
  }

  /**
   * Given the Parent() relations between statements, writes in all the
   * transitive Parent*() relations.
   * 
   * Pre-conditions: 
   *   1) Requires that all the Parent() relations be populated
   *      in the PKB first.
   *   2) Requires that pkb.getStmtTable().size() returns the total
   *      number of statements in the PKB.
   * 
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillParentTTable(Pkb& pkb) {
    Table parentTTable = generateTransitiveClosure(pkb.getParentTable(),
      pkb.getStmtTable().size());
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
    Table followsTTable = generateTransitiveClosure(pkb.getFollowsTable(), 
      pkb.getStmtTable().size());
    for (std::vector<std::string> row : followsTTable.getData()) {
      int followed = std::stoi(row[0]);
      int follower = std::stoi(row[1]);
      pkb.addFollowsT(followed, follower);
    }
  }

  /**
   * Populates the PKB's Uses table with all indirect relations (i.e. Uses() relations
   * other than Uses(s, v) and Uses(if, v) where the variable appears in the conditional.
   * 
   * Requires that the ParentTTable be populated first.
   * 
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillUsesTable(Pkb& pkb) {
    Table newUsesTable = fillIndirectRelation(pkb.getUsesTable(), pkb.getParentTTable());
    for (std::vector<std::string> row : newUsesTable.getData()) {
      std::string proc = row[0];
      std::string var = row[1];
      pkb.addUses(proc, var);
    }
  }

  /**
   * Populates the PKB's Modifies table with all indirect relations (i.e. Modifies() relations
   * other than Modifies(s, v).
   *
   * Requires that the ParentTTable be populated first.
   * 
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillModifiesTable(Pkb& pkb) {
    Table newModifiesTable = fillIndirectRelation(pkb.getModifiesTable(), pkb.getParentTTable());
    for (std::vector<std::string> row : newModifiesTable.getData()) {
      std::string stmtNum = row[0];
      std::string var = row[1];
      pkb.addModifies(stmtNum, var);
    }
  }
}

DesignExtractor::DesignExtractor() {
}

DesignExtractor::~DesignExtractor() {
}

void DesignExtractor::extractDesignAbstractions(Pkb& pkb) {
  fillParentTTable(pkb);
  fillFollowsTTable(pkb);
  fillUsesTable(pkb);
  fillModifiesTable(pkb);
}