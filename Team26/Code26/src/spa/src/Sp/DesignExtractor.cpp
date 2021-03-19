#include "DesignExtractor.h"

#include <assert.h>

#include <string>
#include <vector>
#include <stack>
#include <stdexcept>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "Table.h"
#include "Pkb.h"
#include "AdjList.h"
#include "Cfg.h"
#include "SpaException.h"

namespace {
  /**
   * Fills in the Affects relation based on the CFG.
   *
   * Pre-conditions:
   *   1) Requires that all Modifies relations are filled in.
   *   2) Requires that all Uses relations are filled in.
   *   3) Requires that all assign stmts are filled in.
   *   4) Requires that the CFG is built.
   *
   * @param pkb The PKB to refer to.
   */
  void fillAffectsTable(Pkb& pkb) {
    Table assignTable = pkb.getAssignTable();
    Table ifTable = pkb.getIfTable();
    Table whileTable = pkb.getWhileTable();
    Table callTable = pkb.getCallTable();
    Table modifiesPTable = pkb.getModifiesPTable();

    // Fill up the affects table for each assign statement
    for (Row assignRow : assignTable.getData()) {
      int affecterAssignStmt = std::stoi(assignRow[0]);

      // Extract the varModifed by affecterAssignStmt
      std::string varModified;
      for (const std::string& variable : pkb.getModifiedBy(affecterAssignStmt)) {
        varModified = variable;
      }

      std::unordered_set<int> potentiallyAffectedAssigns = pkb.getAssignUses(varModified);

      // =============== //
      //  DFS algorithm  //
      // =============== //
      std::unordered_set<int> visited; // keep track of visited nodes
      std::stack<int> stack; // stack for dfs

      // Get the next stmts in the CFG from the starting affecterAssignStmt stmt and push to the stack
      for (const int nextStmt : pkb.getNextStmtsFromCfg(affecterAssignStmt)) {
        stack.push(nextStmt);
      }

      while (!stack.empty()) {
        int currentStmt = stack.top();
        stack.pop();

        // Check if currentStmt has been visited
        // Skip this currentStmt if already visited
        if (visited.count(currentStmt) == 1) {
          continue;
        }
        visited.emplace(currentStmt); // Set currentStmt as visited

        // Check if currentStmt satisfy the Affects(affecterAssignStmt, currentStmt) relation
        if (potentiallyAffectedAssigns.count(currentStmt) == 1) {
          pkb.addAffects(affecterAssignStmt, currentStmt); // add Affects relation to pkb
        }

        // Check if currentStmt modifies the varModified
        // Stop searching this path if the currentStmt is not a container stmt and it modifies the varModified
        const bool isModified = pkb.getModifiedBy(currentStmt).count(varModified) == 1;
        const bool isContainerStmt = ifTable.contains({ std::to_string(currentStmt) }) || whileTable.contains({ std::to_string(currentStmt) });
        if (isModified && !isContainerStmt) {
          continue;
        }

        // Push all next stmts into the stack
        for (const int nextStmt : pkb.getNextStmtsFromCfg(currentStmt)) {
          stack.push(nextStmt);
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
   * @param listOfEntities The list of all entities in the table.
   * @returns A Table with entries as detailed above.
   */
  Table generateTransitiveClosure(const Table& table, const std::list<std::string>& listOfEntities) {
    assert(table.getHeader().size() == 2); // Guaranteed to receive a table with 2 columns

    int numEntities = listOfEntities.size();
    Table newTable = table;

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
    const bool areTableColumnCountTwo = (table.getHeader().size() == 2) &&
      (parentTTable.getHeader().size() == 2);
    assert(areTableColumnCountTwo); // Guaranteed to receive tables with 2 columns

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
   * Given the Next() relations between statements, writes in all the
   * transitive Next*() relations.
   *
   * Pre-conditions:
   *   1) Requires that all the Next() relations be populated
   *      in the PKB first.
   *   2) Requires that pkb.getStmtTable() returns a table
   *      containing all the statements in the program.
   *
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillNextTTable(Pkb& pkb) {
    std::list<std::string> stmtList;
    for (const Row row : pkb.getStmtTable().getData()) {
      stmtList.push_back(row[0]);
    }

    Table nextTTable = generateTransitiveClosure(pkb.getNextTable(),
      stmtList);
    for (const Row row : nextTTable.getData()) {
      pkb.addNextT(std::stoi(row[0]), std::stoi(row[1]));
    }
  }

  /**
   * Given the Affects() relations between statements, writes in all the
   * transitive Affects*() relations.
   *
   * Pre-conditions:
   *   1) Requires that all the Affects() relations be populated
   *      in the PKB first.
   *   2) Requires that pkb.getAssignTable() returns a table
   *      containing all the assign statements in the program.
   *
   * @param pkb The PKB to refer to.
   * @returns
   */
  void fillAffectsTTable(Pkb& pkb) {
    std::list<std::string> assignStmtList;
    for (const Row row : pkb.getAssignTable().getData()) {
      assignStmtList.push_back(row[0]);
    }

    Table affectsTTable = generateTransitiveClosure(pkb.getAffectsTable(),
      assignStmtList);
    for (const Row row : affectsTTable.getData()) {
      pkb.addAffectsT(std::stoi(row[0]), std::stoi(row[1]));
    }
  }

  /**
   * Populates the given PKB's UsesS table with all Uses(ifs/w, v) relations where
   * the relation holds due to some Uses(s, v) for some s in the container, if the
   * Uses(s, v) relation is recorded in the UsesS table.
   *
   * Requires that the following be populated first:
   * 1) ParentTTable
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillUsesSTableNonCallStmts(Pkb& pkb) {
    Table newUsesSTable = fillIndirectRelation(pkb.getUsesSTable(), pkb.getParentTTable());
    for (const Row row : newUsesSTable.getData()) {
      int stmtNum = std::stoi(row[0]);
      std::string var = row[1];
      pkb.addUsesS(stmtNum, var);
    }
  }

  /**
   * Populates the given PKB's UsesS table with all Uses(c, v) relations,
   * where c is a call stmt that calls a procedure such that Uses(p, v) holds.
   *
   * Requires that the following be populated first:
   * 1) callProcTable in PKB
   * 2) usesPTable in PKB
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillUsesSTableCallStmts(Pkb& pkb) {
    Table callProcTable = pkb.getCallProcTable();
    Table usesPTable = pkb.getUsesPTable();

    // Inner join the callProcTable{"stmtNum","procName"}
    // and usesPTable{"proc","var"}, so we only retain the 
    // rows where call statements are made to procedures
    // that have some Uses(p, v) relation in the table.
    // We then drop the middle column to obtain a table of 
    // {"stmtNum", "var"} of Uses(c, v) relations.
    callProcTable.innerJoin(usesPTable, 1, 0);
    callProcTable.dropColumn(1);

    for (const Row row : callProcTable.getData()) {
      pkb.addUsesS(std::stoi(row[0]), row[1]);
    }
  }

  /**
   * Populates the given PKB's ModifiesS table with all Modifies(c, v) relations,
   * where c is a call stmt that calls a procedure such that Modifies(p, v) holds.
   *
   * Requires that the following be populated first:
   * 1) callProcTable in PKB
   * 2) modifiesPTable in PKB
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillModifiesSTableCallStmts(Pkb& pkb) {
    Table callProcTable = pkb.getCallProcTable();
    Table modifiesPTable = pkb.getModifiesPTable();

    // Inner join the callProcTable{"stmtNum","procName"}
    // and usesPTable{"proc","var"}, so we only retain the 
    // rows where call statements are made to procedures
    // that have some Uses(p, v) relation in the table.
    // We then drop the middle column to obtain a table of 
    // {"stmtNum", "var"} of Uses(c, v) relations.
    callProcTable.innerJoin(modifiesPTable, 1, 0);
    callProcTable.dropColumn(1);

    for (const Row row : callProcTable.getData()) {
      pkb.addModifiesS(std::stoi(row[0]), row[1]);
    }
  }

  /**
   * Populates the given PKB's ModifiesS table with all Modifies(ifs/w, v) relations where
   * the relation holds due to some Modifies(s, v) for some s in the container, if the
   * Modifies(s, v) relation is recorded in the ModifiesS table.
   *
   * Requires that the following be populated first:
   * 1) ParentTTable
   *
   * @param pkb The PKB to use/modify.
   * @returns
   */
  void fillModifiesSTableNonCallStmts(Pkb& pkb) {
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
  std::unordered_set<std::string> getProceduresCalledBy(Pkb& pkb, std::string procName) {
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
   * If any cyclic calls are found, an exception is thrown.
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
      throw SourceProcessor::SemanticError(
        SourceProcessor::ErrorMessage::SEMANTIC_ERROR_RECURSIVE_OR_CYCLIC_PROCEDURE_CALL +
        "\nExpected topological order of length " + std::to_string(pkb.getProcTable().size()) +
        " but got " + std::to_string(topoOrder.size()) + "."
      );
    }
  }

  /**
   * Verifies that no calls to non-existent procedures exist.
   * If any are found, an exception is thrown.
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
        throw SourceProcessor::SemanticError(
          SourceProcessor::ErrorMessage::SEMANTIC_ERROR_CALL_TO_NON_EXISTENT_PROCEDURE +
          SourceProcessor::ErrorMessage::APPEND_STMT_NUMBER + row[0] +
          SourceProcessor::ErrorMessage::APPEND_PROC_NAME + row[1]
        );
      }
    }
  }
}

namespace SourceProcessor {
  // TODO: Should the verification calls be extracted and called somewhere else?
  void DesignExtractor::extractDesignAbstractions() {
    // Verification
    verifyNoCallsToNonExistentProcedures(pkb);
    verifyNoCyclicCalls(pkb);

    // Transitive relations
    fillParentTTable(pkb);
    fillFollowsTTable(pkb);
    fillCallsTTable(pkb);
    fillNextTTable(pkb);

    // Note the specific order of method calls here;
    // We require that `fillUsesSTableNonCallStmts` be called
    // again at the end to extract Uses(ifs/w, v) relations due to
    // call statements inside the container.
    // The order of UsesS/P design abstraction extraction by DE is as follows:
    // 
    // 1) All Uses(ifs/w, v) due to Uses(s != c, v)
    // 2) All Uses(p, v) due to Calls(p, p') && Uses(p', v)
    // 3) All Uses(c, v) due to call stmt c calling p && Uses(p, v)
    // 4) All Uses(ifs/w, v) due to Uses(c, v)
    // 
    // We assume that other (sub-)components (Parser, etc.) have already
    // extracted the following:
    // 1) All Uses(ifs/w, v) due to conditional using v
    // 2) All Uses(p, v) due to some s != c in p satisfying Uses(s, v)
    // 3) All Uses(a, v), Uses(pn, v)
    fillUsesSTableNonCallStmts(pkb);
    fillUsesPTable(pkb);
    fillUsesSTableCallStmts(pkb);
    fillUsesSTableNonCallStmts(pkb);

    // Same strategy as for UsesS/P.
    // We assume that other (sub-)components (Parser, etc.) have already
    // extracted the following:
    // 1) All Modifies(p, v) due to some s != c in p satisfying Uses(s, v)
    // 2) All Modifies(a, v), Modifies(r, v)
    fillModifiesSTableNonCallStmts(pkb);
    fillModifiesPTable(pkb);
    fillModifiesSTableCallStmts(pkb);
    fillModifiesSTableNonCallStmts(pkb);

    fillAffectsTable(pkb);
    fillAffectsTTable(pkb);
  }
}