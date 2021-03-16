#include "DesignExtractor.h"

#include <assert.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <unordered_set>

#include "Table.h"
#include "Pkb.h"
#include "AdjList.h"

namespace {
  static const std::string GENERATE_TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG =
    "[DesignExtractor::generateTransitiveClosure]: Table must have two columns.";

  static const std::string FILL_INDIRECT_RELATION_HEADER_SIZE_NEQ_2_ERROR_MSG =
    "[DesignExtractor::fillIndirectRelation]: Both tables must have two columns.";

  static const std::string CYCLE_EXISTS_ERROR_MSG =
    "[DesignExtractor::verifyNoCyclicCalls]: Cycle detected.";

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
   * @param listOfEntities The list of all entities in the table. 
   * @returns A Table with entries as detailed above. 
   */
  Table generateTransitiveClosure(const Table& table, const std::list<std::string>& listOfEntities) {
    int numEntities = listOfEntities.size();
    Table newTable = table;
    if (table.getHeader().size() != 2) {
      throw std::invalid_argument(GENERATE_TRANSITIVE_CLOSURE_HEADER_SIZE_NEQ_2_ERROR_MSG);
    }

    // Tables may sometimes have non-numerical entries.
    // We use this to perform conversion to and from.
    std::map<std::string, int> nameToNum;
    std::map<int, std::string> numToName;
    int counter = 1;
    for (const std::string name : listOfEntities) {
      nameToNum.emplace(name, counter);
      numToName.emplace(counter, name);
      counter++;
    }

    // We insert the initial relations into the adjacency list
    // for use in the Warshall algorithm later.
    AdjList adjList(numEntities);
    for (const Row row : table.getData()) {
      adjList.insert(nameToNum.at(row[0]), nameToNum.at(row[1]));
    }

    adjList.applyWarshallAlgorithm();

    // For each true entry in the transitive
    // closure matrix, add a new row to the 
    // table to return.
    for (int i = 1; i <= numEntities; i++) {
      for (int j = 1; j <= numEntities; j++) {
        if (adjList.get(i, j)) {
          Row newRow = { numToName.at(i), numToName.at(j) };
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
  Table fillIndirectRelation(Table table, const Table& parentTTable) {
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
    std::list<std::string> procList;
    for (const Row row : pkb.getStmtTable().getData()) {
      procList.push_back(row[0]);
    }

    Table parentTTable = generateTransitiveClosure(pkb.getParentTable(),
      procList);
    for (const Row row : parentTTable.getData()) {
      int parent = std::stoi(row[0]);
      int child = std::stoi(row[1]);
      pkb.addParentT(parent, child);
    }
  }

  /**
   * Given the Follows() relations between statements, writes in all the
   * transitive Follows*() relations.
   *
   * Pre-conditions:
   *   1) Requires that all the Follows() relations be populated
   *      in the PKB first.
   *   2) Requires that pkb.getStmtTable().size() returns the total
   *      number of statements in the PKB.
   *
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillFollowsTTable(Pkb& pkb) {
    std::list<std::string> stmtList;
    for (const Row row : pkb.getStmtTable().getData()) {
      stmtList.push_back(row[0]);
    }

    Table followsTTable = generateTransitiveClosure(pkb.getFollowsTable(),
      stmtList);
    for (const Row row : followsTTable.getData()) {
      int followed = std::stoi(row[0]);
      int follower = std::stoi(row[1]);
      pkb.addFollowsT(followed, follower);
    }
  }

  /**
   * Given the Calls() relations between procedures, writes in all the 
   * transitive Calls*() relations.
   * 
   * Pre-conditions:
   *   1) Requires that all the Calls() relations be populated 
   *      in the PKB first.
   *   2) Requires that pkb.getProcTable() returns a table
   *      containing all the procedures in the program.
   * 
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillCallsTTable(Pkb& pkb) {
    std::list<std::string> procList;
    for (const Row row : pkb.getProcTable().getData()) {
      procList.push_back(row[0]);
    }

    Table callsTTable = generateTransitiveClosure(pkb.getCallsTable(),
      procList);
    for (const Row row : callsTTable.getData()) {
      pkb.addCallsT(row[0], row[1]);
    }
  }

  /**
   * Populates the PKB's UsesS table with all Uses(s, v) relations between
   * container statements s and variables v.
   * 
   * Requires that the Uses() relations between assignments/print statements and 
   * variables, as well as the ParentTTable be populated first.
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillUsesSTable(Pkb& pkb) {
    Table newUsesSTable = fillIndirectRelation(pkb.getUsesSTable(), pkb.getParentTTable());
    for (const Row row : newUsesSTable.getData()) {
      int stmtNum = std::stoi(row[0]);
      std::string var = row[1];
      pkb.addUsesS(stmtNum, var);
    }
  }

  /**
   * Populates the PKB's ModifiesS table with all Modifies(s, v) relations between
   * container statements s and variables v.
   *
   * Requires that the Modifies() relations between assignments/read statements and
   * variables, as well as the ParentTTable be populated first.
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillModifiesSTable(Pkb& pkb) {
    Table newModifiesSTable = fillIndirectRelation(pkb.getModifiesSTable(), pkb.getParentTTable());
    for (const Row row : newModifiesSTable.getData()) {
      int stmtNum = std::stoi(row[0]);
      std::string var = row[1];
      pkb.addModifiesS(stmtNum, var);
    }
  }

  /**
   * Helper method to get all the procedures directly called by procName. 
   * Requires that the callsTable be populated.
   * 
   * @param pkb The PKB to refer to.
   * @param procName The procedure to check against.
   */
  std::unordered_set<std::string> getProceduresCalledBy(Pkb &pkb, std::string procName) {
    Table callsTable = pkb.getCallsTable();
    std::unordered_set<std::string> toReturn;

    for (const Row row : callsTable.getData()) {
      bool isProcNameCaller = (row[0] == procName);
      if (isProcNameCaller) {
        toReturn.insert(row[1]);
      }
    }

    return toReturn;
  }

  /**
   * Populates the UsesP table with all indirect Uses(p, v) relations between
   * procedures p and variables v. Direct relations are when Uses(p, v) holds because
   * there exists some statement s in p such that Uses(s, v) holds.
   * 
   * Requires that the following be computed first:
   * 1) All other types of Uses() relations
   * 2) callsTable
   * 
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillUsesPTable(Pkb& pkb) {
    // Key idea:
    // 1) Construct a graph where an E(p1, p2) holds if p2 calls p1
    //    i.e. an inversion of the usual procedure call graph.
    // 2) Get a topological ordering of this graph.
    // 3) Compute the Uses(p, v) relations in this order.
    //    The idea is that if p2 calls p1, then p2 depends
    //    on p1 (we need to compute the Uses(p, v) for p1
    //    before we compute Uses(p, v) for p2). Since we inverted
    //    the edge directions, the topological order guarantees that
    //    E(p1, p2) => p1 comes before p2 in the topological ordering.
    // Same logic should work for ModifiesP as well.

    Table callsTable = pkb.getCallsTable();
    AdjList invertedProcGraph(pkb.getProcTable().size());

    // We need to build a bijective mapping between
    // node numbers and procedure names since our 
    // adjacency list only allows numeric identifiers
    // for the graph vertices.
 
    // Maps procedure names to a vertex number.
    std::map<std::string, int> procNameToNum;
    // Maps vertex numbers back to a procedure name.
    std::map<int, std::string> numToProcName;

    int counter = 1;
    for (const Row row : pkb.getProcTable().getData()) {
      std::string procName = row[0];
      procNameToNum.emplace(procName, counter);
      numToProcName.emplace(counter, procName);
      counter++;
    }

    assert(counter == (pkb.getProcTable().size() + 1));

    // Populate the graph
    for (const Row row : callsTable.getData()) {
      std::string caller = row[0];
      std::string callee = row[1];
      invertedProcGraph.insert(procNameToNum.at(callee), procNameToNum.at(caller));
    }

    std::list<int> topoOrder = invertedProcGraph.topologicalOrder();

    // Now we compute the Uses(p, v) relations in topological order.
    for (int nodeNum : topoOrder) {
      std::string procName = numToProcName.at(nodeNum);

      // No need to compute "direct" Uses(p, v) 
      // i.e. relations where some stmt s in p satisfies Uses(s, v)
      // since that is done by the parser. We just add the relations from
      // procedures that are called by our current procedure.

      std::unordered_set<std::string> proceduresCalledByProcName = getProceduresCalledBy(pkb, procName);
      for (const Row row : pkb.getUsesPTable().getData()) {
        bool isFirstArgCalledByProcName = (proceduresCalledByProcName.count(row[0]) > 0);
        std::string var = row[1];
        if (isFirstArgCalledByProcName) {
          pkb.addUsesP(procName, var);
        }
      }
    }
  }

  /**
   * Populates the ModifiesP table with all indirect Modifies(p, v) relations between
   * procedures p and variables v. Direct relations are when Modifies(p, v) holds because
   * there exists some statement s in p such that Modifies(s, v) holds.
   *
   * Requires that all other types of Modifies() relations, as well as
   * the ParentTTable be populated first.
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillModifiesPTable(Pkb& pkb) {
    Table callsTable = pkb.getCallsTable();
    AdjList invertedProcGraph(pkb.getProcTable().size());

    // We need to build a bijective mapping between
    // node numbers and procedure names since our 
    // adjacency list only allows numeric identifiers
    // for the graph vertices.

    // Maps procedure names to a vertex number.
    std::map<std::string, int> procNameToNum;
    // Maps vertex numbers back to a procedure name.
    std::map<int, std::string> numToProcName;

    int counter = 1;
    for (const Row row : pkb.getProcTable().getData()) {
      std::string procName = row[0];
      procNameToNum.emplace(procName, counter);
      numToProcName.emplace(counter, procName);
      counter++;
    }

    // After mapping all the procedures, the counter
    // should be equal to the number of procedures,
    // plus 1.
    assert(counter == (pkb.getProcTable().size() + 1));

    // Populate the graph
    for (const Row row : callsTable.getData()) {
      std::string caller = row[0];
      std::string callee = row[1];
      invertedProcGraph.insert(procNameToNum.at(callee), procNameToNum.at(caller));
    }

    std::list<int> topoOrder = invertedProcGraph.topologicalOrder();

    // Now we compute the Uses(p, v) relations in topological order.
    for (int nodeNum : topoOrder) {
      std::string procName = numToProcName.at(nodeNum);

      // No need to compute "direct" Uses(p, v) 
      // i.e. relations where some stmt s in p satisfies Uses(s, v)
      // since that is done by the parser. We just add the relations from
      // procedures that are called by our current procedure.

      std::unordered_set<std::string> proceduresCalledByProcName = getProceduresCalledBy(pkb, procName);
      for (const Row row : pkb.getModifiesPTable().getData()) {
        bool isFirstArgCalledByProcName = (proceduresCalledByProcName.count(row[0]) > 0);
        std::string var = row[1];
        if (isFirstArgCalledByProcName) {
          pkb.addModifiesP(procName, var);
        }
      }
    }
  }

  /**
   * Verifies that no cyclic calls are made.
   * e.g. where A -> B indicates that A calls B, we verify
   * that no situation of the form A -> B -> C -> ... -> A exists.
   * If any cyclic calls are found, a std::logic_error exception is thrown.
   * 
   * Requires that the callsTable be populated first.
   * 
   * @param pkb The PKB to reference.
   * @returns none
   */
  void verifyNoCyclicCalls(Pkb& pkb) {
    // Key idea:
    // 1) Get the topological order of the procedure calls
    // 2) Check if the number of vertices (procedures) in the order
    //    equals the total number of procedures. If not, a cycle exists.
    AdjList procGraph(pkb.getProcTable().size());

    std::map<std::string, int> procNameToNum;
    std::map<int, std::string> numToProcName;

    int counter = 1;
    for (const Row row : pkb.getProcTable().getData()) {
      std::string procName = row[0];
      procNameToNum.emplace(procName, counter);
      numToProcName.emplace(counter, procName);
      counter++;
    }

    for (const Row row : pkb.getCallsTable().getData()) {
      std::string caller = row[0];
      std::string callee = row[1];
      procGraph.insert(procNameToNum.at(caller), procNameToNum.at(callee));
    }

    std::list<int> topoOrder = procGraph.topologicalOrder();
    bool isCycleExists = (topoOrder.size() != pkb.getProcTable().size());

    if (isCycleExists) {
      throw std::logic_error(CYCLE_EXISTS_ERROR_MSG + 
        " (expected topological order of length " + 
        std::to_string(pkb.getProcTable().size()) + 
        ", but got " + std::to_string(topoOrder.size()) + 
        ".)");
    }
  }

  /**
   * Verifies that no calls to non-existent procedures exist.
   * If any are found, a std::logic_error exception is thrown.
   * 
   * @returns none
   */
  void verifyNoCallsToNonExistentProcedures(Pkb& pkb) {
    // Table mapping call stmt# to procedure called.
    Table callProcTable = pkb.getCallProcTable();

    // Table detailing all procedures.
    Table procTable = pkb.getProcTable();

    for (const Row row : callProcTable.getData()) {
      bool isCallToExistentProcedure = procTable.getData().count({ row[1] }) > 0;
      if (!isCallToExistentProcedure) {
        throw std::logic_error("[DesignExtractor::verifyNoCallsToNonExistent"
          "Procedures] Call to non-existent procedure (" + row[1] + ") found"
          "at line " + row[0] + ".");
      }
    }
  }
}

// TODO: Should the verification calls be extracted and called somewhere else?
void DesignExtractor::extractDesignAbstractions() {
  fillParentTTable(pkb);
  fillFollowsTTable(pkb);
  fillUsesSTable(pkb);
  fillModifiesSTable(pkb);

  verifyNoCyclicCalls(pkb);
  verifyNoCallsToNonExistentProcedures(pkb);

  // Calls that require as a pre-condition
  // that no cyclic calls exist.
  fillCallsTTable(pkb);
  fillUsesPTable(pkb);
  fillModifiesPTable(pkb);
}
