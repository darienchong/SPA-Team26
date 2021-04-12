#include "DesignExtractor.h"

#include <assert.h>

#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AdjList.h"
#include "Cfg.h"
#include "Pkb.h"
#include "SpaException.h"
#include "Table.h"

namespace {
  /**
   * Initialises the CFGBip, by adding dummy nodes to the CFG and using the graph explosion method to generate the sequence
   * of CFGBip traversal for each procedure.
   *
   * @param pkb The PKB to refer to.
   */
  void initialiseCfgBip(Pkb& pkb, std::list<std::string>& topoProc) {
    const std::unordered_set<int>& procIntRefs = pkb.getProcIntRefs();

    // Create dummy nodes and adding edge from end statements of each proc to their respective dummy nodes
    for (const int procIntRef : procIntRefs) {
      const std::string& proc = pkb.getEntityFromIntRef(procIntRef);
      const int dummyNode = -1 * pkb.getStartStmtFromProc(proc);

      for (const int end : pkb.getEndStmtsFromProc(proc)) {
        pkb.addCfgEdge(end, dummyNode);
      }
    }

    pkb.initialiseCfgBip(topoProc);
  }

  /**
    * Fills the NextBip and NextBipT relations.
    *
    * @param pkb The PKB to refer to.
    */
  void fillNextBipTable(Pkb& pkb) {
    // Get starting node for each proc
    for (const std::shared_ptr<Cfg::BipNode>& startPtr : pkb.getStartBipNodes()) {
      std::unordered_set<std::shared_ptr<Cfg::BipNode>> outerVisited;
      std::stack<std::shared_ptr<Cfg::BipNode>> outerDfsStack;
      outerDfsStack.push(startPtr); // push startNode to stack
      outerVisited.emplace(startPtr); // mark startNode as visited

      // Do DFS from each node
      while (!outerDfsStack.empty()) {
        const std::shared_ptr<Cfg::BipNode>& currPtr = outerDfsStack.top();
        const int currStmt = currPtr->node;
        outerDfsStack.pop();

        // Only proceed with inner dfs for non dummy nodes
        if (currStmt > 0) {
          std::unordered_set<std::shared_ptr<Cfg::BipNode>> innerVisited;
          std::stack<std::shared_ptr<Cfg::BipNode>> innerDfsStack;
          for (std::shared_ptr<Cfg::BipNode>& nextPtr : currPtr->nexts) {
            // Iterate through to find the first non-dummy node
            while (nextPtr->node < 0) {
              assert(nextPtr->nexts.size() <= 1);
              if (nextPtr->nexts.empty()) {
                break;
              }
              nextPtr = nextPtr->nexts[0];
            }

            // Add to nextBip relation if non-dummy node
            if (nextPtr->node > 0) {
              pkb.addNextBip(currStmt, nextPtr->node);
            }

            innerDfsStack.push(nextPtr); // push nextNodes to stack
            innerVisited.emplace(nextPtr); // mark nextNodes as visited
          }

          // Do DFS
          while (!innerDfsStack.empty()) {
            const std::shared_ptr<Cfg::BipNode>& targetPtr = innerDfsStack.top();
            const int targetStmt = targetPtr->node;
            innerDfsStack.pop();

            if (targetStmt > 0) {
              pkb.addNextBipT(currStmt, targetStmt);
            }

            // Add nextNodes to the stack under certain conditions
            for (const std::shared_ptr<Cfg::BipNode>& nextPtr : targetPtr->nexts) {
              // Add nextNode to stack only if not visited
              if (innerVisited.count(nextPtr) == 0) {
                innerDfsStack.push(nextPtr);
                innerVisited.emplace(nextPtr); // mark nextNode as visited
              }
            }
          }
        }

        // Add nextNodes to the stack under certain conditions
        for (const std::shared_ptr<Cfg::BipNode>& nextPtr : currPtr->nexts) {
          // Add nextNode to stack only if not visited
          if (outerVisited.count(nextPtr) == 0) {
            outerDfsStack.push(nextPtr);
            outerVisited.emplace(nextPtr); // mark nextNode as visited
          }
        }
      }
    }
  }

  /**
  * Fills the AffectsBip and AffectsBipT relations.
  *
  * @param pkb The PKB to refer to.
  */
  void fillAffectsBipTable(Pkb& pkb) {
    std::unordered_set<std::shared_ptr<Cfg::BipNode>> affectsBipPtrs;
    std::unordered_set<std::string> visitedProcs;
    const Table& assignTable = pkb.getAssignTable();
    const Table& callTable = pkb.getCallTable();
    const Table& readTable = pkb.getReadTable();

    // Generate AffectsBip graph and create edges for AffectsBipT
    for (const std::shared_ptr<Cfg::BipNode>& startPtr : pkb.getStartBipNodes()) {

      std::unordered_set<std::shared_ptr<Cfg::BipNode>> outerVisited;
      std::stack<std::shared_ptr<Cfg::BipNode>> outerDfsStack;
      outerDfsStack.push(startPtr); // push startNode to stack
      outerVisited.emplace(startPtr); // mark startNode as visited

      // Do DFS from each node
      while (!outerDfsStack.empty()) {
        const std::shared_ptr<Cfg::BipNode>& currPtr = outerDfsStack.top();
        const int currStmt = currPtr->node;
        outerDfsStack.pop();

        // Only proceed with inner dfs for assign statements
        if (assignTable.contains({ pkb.getIntRefFromStmtNum(currStmt) })) {
          std::unordered_set<std::shared_ptr<Cfg::BipNode>> innerVisited;
          std::stack<std::shared_ptr<Cfg::BipNode>> innerDfsStack;

          // Extract the varModifed by currTargetAssign
          std::string varModified;
          for (const std::string& variable : pkb.getModifiedBy(currStmt)) {
            varModified = variable;
          }

          // Extract the assign statements that uses varModified
          const std::unordered_set<int>& potentiallyAffectedAssigns = pkb.getAssignUses(varModified);
          if (potentiallyAffectedAssigns.empty()) {
            for (const std::shared_ptr<Cfg::BipNode>& nextPtr : currPtr->nexts) {
              // Add nextNode to stack only if not visited
              if (outerVisited.count(nextPtr) == 0) {
                outerDfsStack.push(nextPtr);
                outerVisited.emplace(nextPtr); // mark nextNode as visited
              }
            }
            continue;  // shortcircuit if no assign uses the varModified
          }

          for (std::shared_ptr<Cfg::BipNode>& nextPtr : currPtr->nexts) {
            // Iterate through to find the first non-dummy node
            innerDfsStack.push(nextPtr); // push nextNodes to stack
            innerVisited.emplace(nextPtr); // mark nextNodes as visited
          }
          assert(currPtr->nexts.size() <= 1);

          // Do DFS
          while (!innerDfsStack.empty()) {
            const std::shared_ptr<Cfg::BipNode>& targetPtr = innerDfsStack.top();
            const int targetStmt = targetPtr->node;
            innerDfsStack.pop();

            // Check if currentStmt satisfy the AffectsBip(currStmt, targetStmt) relation
            if (potentiallyAffectedAssigns.count(targetStmt) == 1) {
              pkb.addAffectsBip(currStmt, targetStmt);
              currPtr->affectsNexts.push_back(targetPtr);
              affectsBipPtrs.emplace(currPtr);
            }

            // Check if targetStmt modifies the varModified
            const bool isModified = pkb.getModifiedBy(targetStmt).count(varModified) == 1;
            const int ref = pkb.getIntRefFromStmtNum(targetStmt);
            const bool isReadOrAssignStmt = readTable.contains({ ref }) || assignTable.contains({ ref });
            // Stop searching this path if the targetStmt is an assign stmt or read stmt
            if (isModified && isReadOrAssignStmt) {
              continue;
            }

            // Add nextNodes to the stack under certain conditions
            for (const std::shared_ptr<Cfg::BipNode>& nextPtr : targetPtr->nexts) {
              // Add nextNode to stack only if not visited
              if (innerVisited.count(nextPtr) == 0) {
                innerDfsStack.push(nextPtr);
                innerVisited.emplace(nextPtr); // mark nextNode as visited
              }
            }
          }
        }

        // Add nextNodes to the stack under certain conditions
        for (const std::shared_ptr<Cfg::BipNode>& nextPtr : currPtr->nexts) {
          // Add nextNode to stack only if not visited
          if (outerVisited.count(nextPtr) == 0) {
            outerDfsStack.push(nextPtr);
            outerVisited.emplace(nextPtr); // mark nextNode as visited
          }
        }
      }
    }

    // Adding of AffectsBipT relations
    for (const std::shared_ptr<Cfg::BipNode>& affectsBipPtr : affectsBipPtrs) {
      const int currAssign = affectsBipPtr->node;
      std::stack<std::shared_ptr<Cfg::BipNode>> dfsStack;
      std::unordered_set<std::shared_ptr<Cfg::BipNode>> visited;
      for (std::shared_ptr<Cfg::BipNode>& nextAssignPtr : affectsBipPtr->affectsNexts) {
        dfsStack.push(nextAssignPtr);
        visited.emplace(nextAssignPtr);
      }

      while (!dfsStack.empty()) {
        const std::shared_ptr<Cfg::BipNode>& assignPtr = dfsStack.top();
        dfsStack.pop();

        pkb.addAffectsBipT(currAssign, assignPtr->node);

        for (std::shared_ptr<Cfg::BipNode>& nextAssignPtr : assignPtr->affectsNexts) {
          if (visited.count(nextAssignPtr) == 0) {
            dfsStack.push(nextAssignPtr);
            visited.emplace(nextAssignPtr);
          }
        }
      }
    }
  }

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
    const std::unordered_set<int>& assignIntRefs = pkb.getAssignIntRefs();
    const std::unordered_set<int>& ifIntRefs = pkb.getIfIntRefs();
    const std::unordered_set<int>& whileIntRefs = pkb.getWhileIntRefs();

    // Fill up the affects table for each assign statement
    for (const int assignIntRef : assignIntRefs) {
      const int affecterAssignStmt = pkb.getStmtNumFromIntRef(assignIntRef);

      // Extract the varModifed by affecterAssignStmt
      std::string varModified;
      for (const std::string& variable : pkb.getModifiedBy(affecterAssignStmt)) {
        varModified = variable;
      }

      const std::unordered_set<int>& potentiallyAffectedAssigns = pkb.getAssignUses(varModified);
      if (potentiallyAffectedAssigns.empty()) {
        continue;  // shortcircuit if no assign uses the varModified
      }

      // =============== //
      //  DFS algorithm  //
      // =============== //
      std::unordered_set<int> visited; // keep track of visited nodes
      std::stack<int> stack; // stack for dfs

      // Get the next stmts in the CFG from the starting affecterAssignStmt stmt and push to the stack
      for (const int nextStmt : pkb.getNextStmtsFromCfg(affecterAssignStmt)) {
        stack.push(nextStmt);
        visited.emplace(nextStmt);
      }

      while (!stack.empty()) {
        int currentStmt = stack.top();
        stack.pop();

        // Check if currentStmt satisfy the Affects(affecterAssignStmt, currentStmt) relation
        if (potentiallyAffectedAssigns.count(currentStmt) == 1) {
          pkb.addAffects(affecterAssignStmt, currentStmt); // add Affects relation to pkb
        }

        // Check if currentStmt modifies the varModified
        // Stop searching this path if the currentStmt is not a container stmt and it modifies the varModified
        const bool isModified = pkb.getModifiedBy(currentStmt).count(varModified) == 1;
        const bool isContainerStmt =
          ifIntRefs.count(pkb.getIntRefFromStmtNum(currentStmt)) == 1 ||
          whileIntRefs.count(pkb.getIntRefFromStmtNum(currentStmt)) == 1;
        if (isModified && !isContainerStmt) {
          continue;
        }

        // Push all next stmts into the stack
        for (const int nextStmt : pkb.getNextStmtsFromCfg(currentStmt)) {
          // Add non-visited nextStmt to stack and mark as visited
          if (visited.count(nextStmt) == 0) {
            stack.push(nextStmt);
            visited.emplace(nextStmt);
          }
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
   * @param table The table to use and modify when generating
   *     the transitive closure.
   * @param listOfEntities The list of all entities in the table.
   */
  void generateTransitiveClosure(Table& table, const std::list<int>& listOfEntities) {
    assert(table.getHeader().size() == 2); // Guaranteed to receive a table with 2 columns

    int numEntities = listOfEntities.size();

    // Tables may sometimes have non-consecutive entries.
    // We use this to perform conversion to and from.
    std::unordered_map<int, int> nameToNum;
    std::unordered_map<int, int> numToName;
    int counter = 1;
    for (const int name : listOfEntities) {
      nameToNum.emplace(name, counter);
      numToName.emplace(counter, name);
      counter++;
    }

    // We insert the initial relations into the adjacency list
    // for use in the Warshall algorithm later.
    AdjList adjList(numEntities);
    for (const Row& row : table.getData()) {
      adjList.insert(nameToNum.at(row[0]), nameToNum.at(row[1]));
    }

    adjList.applyWarshallAlgorithm();

    // For each true entry in the transitive
    // closure matrix, add a new row to the 
    // table to return.
    for (int i = 1; i <= numEntities; i++) {
      for (int j = 1; j <= numEntities; j++) {
        if (adjList.get(i, j)) {
          table.insertRow({ numToName.at(i), numToName.at(j) });
        }
      }
    }
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
   * @param table Copy of the table to use/modify.
   * @param parentTTable Copy of the parentT table to refer to.
   * @returns The table with new indirect relation entries.
   */
  Table fillIndirectRelation(Table& table, Table& parentTTable) {
    const bool areTableColumnCountTwo = (table.getHeader().size() == 2) &&
      (parentTTable.getHeader().size() == 2);
    assert(areTableColumnCountTwo); // Guaranteed to receive tables with 2 columns

    parentTTable.innerJoin(table, 1, 0);
    parentTTable.dropColumn(1);
    table.concatenate(parentTTable);

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
   */
  void fillParentTTable(Pkb& pkb) {
    std::list<int> stmtList;
    for (const Row& row : pkb.getStmtTable().getData()) {
      stmtList.push_back(row[0]);
    }

    Table& parentTTable = pkb.getParentTable();
    generateTransitiveClosure(parentTTable, stmtList);
    for (const Row& row : parentTTable.getData()) {
      pkb.addParentT(pkb.getStmtNumFromIntRef(row[0]), pkb.getStmtNumFromIntRef(row[1]));
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
   */
  void fillFollowsTTable(Pkb& pkb) {
    std::list<int> stmtList;
    for (const Row& row : pkb.getStmtTable().getData()) {
      stmtList.push_back(row[0]);
    }

    Table& followsTTable = pkb.getFollowsTable();
    generateTransitiveClosure(followsTTable, stmtList);
    for (const Row& row : followsTTable.getData()) {
      pkb.addFollowsT(pkb.getStmtNumFromIntRef(row[0]), pkb.getStmtNumFromIntRef(row[1]));
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
    std::list<int> procList;
    for (const Row& row : pkb.getProcTable().getData()) {
      procList.push_back(row[0]);
    }

    Table& callsTTable = pkb.getCallsTable();
    generateTransitiveClosure(callsTTable, procList);
    for (const Row& row : callsTTable.getData()) {
      pkb.addCallsT(pkb.getEntityFromIntRef(row[0]), pkb.getEntityFromIntRef(row[1]));
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
    std::list<int> stmtList;
    for (const Row& row : pkb.getStmtTable().getData()) {
      stmtList.push_back(row[0]);
    }

    Table& nextTTable = pkb.getNextTable();
    generateTransitiveClosure(nextTTable, stmtList);
    for (const Row& row : nextTTable.getData()) {
      pkb.addNextT(pkb.getStmtNumFromIntRef(row[0]), pkb.getStmtNumFromIntRef(row[1]));
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
   */
  void fillAffectsTTable(Pkb& pkb) {
    std::list<int> assignStmtList;
    for (const Row& row : pkb.getAssignTable().getData()) {
      assignStmtList.push_back(row[0]);
    }

    Table& affectsTTable = pkb.getAffectsTable();
    generateTransitiveClosure(affectsTTable, assignStmtList);
    for (const Row& row : affectsTTable.getData()) {
      pkb.addAffectsT(pkb.getStmtNumFromIntRef(row[0]), pkb.getStmtNumFromIntRef(row[1]));
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
   */
  void fillUsesSTableNonCallStmts(Pkb& pkb) {
    const Table& newUsesSTable = fillIndirectRelation(pkb.getUsesSTable(), pkb.getParentTTable());
    for (const Row& row : newUsesSTable.getData()) {
      pkb.addUsesS(pkb.getStmtNumFromIntRef(row[0]), pkb.getEntityFromIntRef(row[1]));
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
   */
  void fillUsesSTableCallStmts(Pkb& pkb) {
    Table callProcTable = pkb.getCallProcTable();
    const Table& usesPTable = pkb.getUsesPTable();

    // Inner join the callProcTable{"stmtNum","procName"}
    // and usesPTable{"proc","var"}, so we only retain the 
    // rows where call statements are made to procedures
    // that have some Uses(p, v) relation in the table.
    // We then drop the middle column to obtain a table of 
    // {"stmtNum", "var"} of Uses(c, v) relations.
    callProcTable.innerJoin(usesPTable, 1, 0);
    callProcTable.dropColumn(1);

    for (const Row& row : callProcTable.getData()) {
      pkb.addUsesS(pkb.getStmtNumFromIntRef(row[0]), pkb.getEntityFromIntRef(row[1]));
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

    for (const Row& row : callProcTable.getData()) {
      pkb.addModifiesS(pkb.getStmtNumFromIntRef(row[0]), pkb.getEntityFromIntRef(row[1]));
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
   */
  void fillModifiesSTableNonCallStmts(Pkb& pkb) {
    const Table& newModifiesSTable = fillIndirectRelation(pkb.getModifiesSTable(), pkb.getParentTTable());
    for (const Row& row : newModifiesSTable.getData()) {
      pkb.addModifiesS(pkb.getStmtNumFromIntRef(row[0]), pkb.getEntityFromIntRef(row[1]));
    }
  }

  /**
   * Helper method to get all the procedures directly called by procName.
   * Requires that the callsTable be populated.
   *
   * @param pkb The PKB to refer to.
   * @param procName The procedure to check against.
   */
  std::unordered_set<std::string> getProceduresCalledBy(const Pkb& pkb, std::string procName) {
    Table callsTable = pkb.getCallsTable();
    std::unordered_set<std::string> toReturn;
    toReturn.reserve(callsTable.size());

    for (const Row row : callsTable.getData()) {
      bool isProcNameCaller = (pkb.getEntityFromIntRef(row[0]) == procName);
      if (isProcNameCaller) {
        toReturn.insert(pkb.getEntityFromIntRef(row[1]));
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
   * @param topoProc The list of topologically sorted procedures in reverse order.
   */
  void fillUsesPTable(Pkb& pkb,
    std::list<std::string>& reverseTopoSortedProcs) {
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

    // We compute the Uses(p, v) relations in topological order.
    for (const std::string& procName : reverseTopoSortedProcs) {

      // No need to compute "direct" Uses(p, v) 
      // i.e. relations where some stmt s in p satisfies Uses(s, v)
      // since that is done by the parser. We just add the relations from
      // procedures that are called by our current procedure.

      const std::unordered_set<std::string>& proceduresCalledByProcName = getProceduresCalledBy(pkb, procName);
      for (const Row row : pkb.getUsesPTable().getData()) {
        bool isFirstArgCalledByProcName = (proceduresCalledByProcName.count(pkb.getEntityFromIntRef(row[0])) > 0);
        std::string var = pkb.getEntityFromIntRef(row[1]);
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
   * @param topoProc The list of topologically sorted procedures in reverse order.
   */
  void fillModifiesPTable(Pkb& pkb,
    std::list<std::string>& reverseTopoSortedProcs) {
    // We compute the Uses(p, v) relations in reverse topological order.
    for (const std::string& procName : reverseTopoSortedProcs) {
      // No need to compute "direct" Uses(p, v) 
      // i.e. relations where some stmt s in p satisfies Uses(s, v)
      // since that is done by the parser. We just add the relations from
      // procedures that are called by our current procedure.

      std::unordered_set<std::string> proceduresCalledByProcName = getProceduresCalledBy(pkb, procName);
      for (const Row row : pkb.getModifiesPTable().getData()) {
        bool isFirstArgCalledByProcName = (proceduresCalledByProcName.count(pkb.getEntityFromIntRef(row[0])) > 0);
        std::string var = pkb.getEntityFromIntRef(row[1]);
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
   * @param topoProc The list of topologically sorted procedures.
   */
  void verifyNoCyclicCalls(Pkb& pkb, std::list<std::string>& topoSortedProcs) {
    // Key idea:
    // 1) Get the topological order of the procedure calls
    // 2) Check if the number of vertices (procedures) in the order
    //    equals the total number of procedures. If not, a cycle exists.
    const int topoSortedProcsSize = topoSortedProcs.size();
    const int numProcs = pkb.getProcIntRefs().size();
    bool isCycleExists = (topoSortedProcsSize != numProcs);

    if (isCycleExists) {
      throw SourceProcessor::SemanticError(
        SourceProcessor::ErrorMessage::SEMANTIC_ERROR_RECURSIVE_OR_CYCLIC_PROCEDURE_CALL +
        "\nExpected topological order of length " + std::to_string(numProcs) +
        " but got " + std::to_string(topoSortedProcsSize) + "."
      );
    }
  }

  void initialiseTopoSortedProcs(const Pkb& pkb,
    std::list<std::string>& topoSortedProcs,
    std::list<std::string>& reverseTopoSortedProcs) {
    // Key idea:
    // 1) Get the topological order of the procedure calls
    // 2) Check if the number of vertices (procedures) in the order
    //    equals the total number of procedures. If not, a cycle exists.
    const std::unordered_set<int>& procIntRefs = pkb.getProcIntRefs();
    const int numProcs = procIntRefs.size();

    // Construct mapping
    std::unordered_map<std::string, int> procNameToNum;
    procNameToNum.reserve(numProcs);
    std::unordered_map<int, std::string> numToProcName;
    numToProcName.reserve(numProcs);

    int counter = 1;
    for (const int intRef : procIntRefs) {
      std::string procName = pkb.getEntityFromIntRef(intRef);
      procNameToNum.emplace(procName, counter);
      numToProcName.emplace(counter, procName);
      counter++;
    }

    // Construct graphs
    AdjList procGraph(numProcs);
    AdjList reverseProcGraph(numProcs);

    for (const Row row : pkb.getCallsTable().getData()) {
      std::string caller = pkb.getEntityFromIntRef(row[0]);
      std::string callee = pkb.getEntityFromIntRef(row[1]);
      procGraph.insert(procNameToNum.at(caller), procNameToNum.at(callee));
      reverseProcGraph.insert(procNameToNum.at(callee), procNameToNum.at(caller));
    }

    const std::list<int>& topoOrder = procGraph.topologicalOrder();
    const std::list<int>& reverseTopoOrder = reverseProcGraph.topologicalOrder();
    for (const int num : topoOrder) {
      std::string proc = numToProcName.at(num);
      topoSortedProcs.push_back(proc);
    }
    for (const int num : reverseTopoOrder) {
      std::string proc = numToProcName.at(num);
      reverseTopoSortedProcs.push_back(proc);
    }
  }

  /**
   * Verifies that no calls to non-existent procedures exist.
   * If any are found, an exception is thrown.
   *
   * @param pkb The PKB to reference.
   */
  void verifyNoCallsToNonExistentProcedures(const Pkb& pkb) {
    // Table of references mapping call stmt# to procedure called.
    const Table& callProcTable = pkb.getCallProcTable();

    // Set detailing all procedures references.
    const std::unordered_set<int> procIntRefs = pkb.getProcIntRefs();

    for (const Row row : callProcTable.getData()) {
      bool isCallToExistentProcedure = procIntRefs.count(row[1]) > 0;
      if (!isCallToExistentProcedure) {
        throw SourceProcessor::SemanticError(
          SourceProcessor::ErrorMessage::SEMANTIC_ERROR_CALL_TO_NON_EXISTENT_PROCEDURE +
          SourceProcessor::ErrorMessage::APPEND_STMT_NUMBER + pkb.getEntityFromIntRef(row[0]) +
          SourceProcessor::ErrorMessage::APPEND_PROC_NAME + pkb.getEntityFromIntRef(row[1])
        );
      }
    }
  }
}

namespace SourceProcessor {
  DesignExtractor::DesignExtractor(Pkb& pkb) : pkb(pkb) {
  }

  void DesignExtractor::extractDesignAbstractions() {
    // Verification
    verifyNoCallsToNonExistentProcedures(pkb);
    initialiseTopoSortedProcs(pkb, topoSortedProcs, reverseTopoSortedProcs);
    verifyNoCyclicCalls(pkb, topoSortedProcs);

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
    fillUsesPTable(pkb, reverseTopoSortedProcs);
    fillUsesSTableCallStmts(pkb);
    fillUsesSTableNonCallStmts(pkb);

    // Same strategy as for UsesS/P.
    // We assume that other (sub-)components (Parser, etc.) have already
    // extracted the following:
    // 1) All Modifies(p, v) due to some s != c in p satisfying Uses(s, v)
    // 2) All Modifies(a, v), Modifies(r, v)
    fillModifiesSTableNonCallStmts(pkb);
    fillModifiesPTable(pkb, reverseTopoSortedProcs);
    fillModifiesSTableCallStmts(pkb);
    fillModifiesSTableNonCallStmts(pkb);

    fillAffectsTable(pkb);
    fillAffectsTTable(pkb);
  }

  void DesignExtractor::extractIter3DesignAbstractions() {
    // Iteration 3 extensions
    initialiseCfgBip(pkb, topoSortedProcs);
    fillNextBipTable(pkb);
    fillAffectsBipTable(pkb);
  }

  void DesignExtractor::extractAllDesignAbstractions() {
    extractDesignAbstractions();
    extractIter3DesignAbstractions();
  }
}
