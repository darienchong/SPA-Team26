#include "PqlPreprocessor.h"

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <list>

#include "AdjList.h"

namespace std {
  template <> struct hash<Pql::Entity>
  {
    size_t operator()(const Pql::Entity & e) const
    {
      return hash<string>()(e.getValue());
    }
  };


  template <> struct hash<Pql::Clause>
  {
    size_t operator()(const Pql::Clause& c) const
    {
      // See https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
      std::size_t seed = c.getParams().size();
      for (Pql::Entity param : c.getParams()) {
        seed ^= hash<string>()(param.getValue()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed ^ static_cast<std::size_t>(c.getType());
    }
  };


}

namespace {

  bool isClauseWithoutSynonym(const Pql::Clause clause) {
    std::vector<Pql::Entity> clauseParams = clause.getParams();
    bool isWithoutSynonyms = true;
    for (Pql::Entity entity : clauseParams) {
      if (entity.isSynonym()) {
        isWithoutSynonyms = false;
      }
    }
    return isWithoutSynonyms;
  }

  std::unordered_set<Pql::Entity> getGroupSynonyms(std::vector<Pql::Clause> group) {
    std::unordered_set<Pql::Entity> synonyms;
    for (Pql::Clause clause : group) {
      for (const Pql::Entity entity : clause.getParams()) {
        if (entity.isSynonym()) {
          synonyms.insert(entity);
        }
      }
    }
    return synonyms;
  }


  bool isClauseWithOneConstantOneSynonym(const Pql::Clause clause) {
    std::vector<Pql::Entity> clauseParams = clause.getParams();
    return ((clauseParams[0].isNumber() || clauseParams[0].isName()) && clauseParams[1].isSynonym()) ||
      ((clauseParams[1].isNumber() || clauseParams[1].isName()) && clauseParams[0].isSynonym());
  }

  /**
   * Returns `true` if the two clauses share at least one synonym, `false` otherwise.
   * Guarantees worst case O(clause.getParams.size() * otherClause.getParams.size()) performance.
   */
  bool isClausesShareSynonym(const Pql::Clause clause, const Pql::Clause otherClause) {
    // Short circuit
    // Can't share synonyms if either clause has no synonyms.
    bool isThisClauseWithoutSynonym = isClauseWithoutSynonym(clause);
    bool isOtherClauseWithoutSynonym = isClauseWithoutSynonym(otherClause);
    if (isThisClauseWithoutSynonym || isOtherClauseWithoutSynonym) {
      return false;
    }

    std::vector<Pql::Entity> clauseParams = clause.getParams();
    std::vector<Pql::Entity> otherClauseParams = otherClause.getParams();

    // See https://stackoverflow.com/questions/27131628/check-whether-two-elements-have-a-common-element-in-c
    // Other way is to sort them first - need to implement some sort of total order.
    return std::find_first_of(
      clauseParams.begin(),
      clauseParams.end(),
      otherClauseParams.begin(),
      otherClauseParams.end()
    ) != clauseParams.end();
  }
  
  /**
   * Carries out the following step in Query Optimization:
   * 1. Divide the clauses into multiple groups
   *    - Clauses without synonyms
   *    - Clauses with connected synonyms should be in the same group
   */
  std::vector<std::vector<Pql::Clause>> formGroups(std::vector<Pql::Clause> clauses) {
    std::vector<Pql::Clause> clausesWithoutSynonyms;
    for (Pql::Clause clause : clauses) {
      if (isClauseWithoutSynonym(clause)) {
        clausesWithoutSynonyms.emplace_back(clause);
      }
    }

    // Use of erase-remove idiom to remove all
    // synonym-less clauses from the vector of all clauses.
    clauses.erase(
      std::remove_if(
        clauses.begin(),
        clauses.end(),
        [](Pql::Clause clause) { return isClauseWithoutSynonym(clause); }
      ),
      clauses.end()
    );

    // Use connected-component algorithm to group clauses
    // using connected synonyms.
    // i) Construct a graph where the nodes are clauses and
    //    a bidirectional edge between clauses exists iff
    //    the two clauses share at least one synonym.
    //    * What's the fastest way to do this? 
    //    * Naive algorithm pegs this at O(V^2) (compute edge construction for every pair)
    // ii) Use any fast algorithm to find all connected components
    //     Candidate: https://www.geeksforgeeks.org/connected-components-in-an-undirected-graph/
    //     Worst-case performance: O(V + E)
    // iii) For each connected component, store them in std::vector.
    AdjList clauseGraph(clauses.size());
    std::map<int, Pql::Clause> idxToClause;
    std::unordered_map<Pql::Clause, int> clauseToIdx;

    int counter = 1;
    for (Pql::Clause clause : clauses) {
      idxToClause.emplace(counter, clause);
      clauseToIdx.emplace(clause, counter);
      counter++;
    }

    // Add edges into graph.
    // TODO: Find a better (read: faster) way to do this?
    for (int i = 1; i <= clauses.size(); i++) {
      for (int j = 1; j <= clauses.size(); j++) {
        if (i == j) {
          continue;
        }

        if (isClausesShareSynonym(idxToClause.at(i), idxToClause.at(j))) {
          clauseGraph.insert(i, j);
          clauseGraph.insert(j, i);
        }
      }
    }

    std::list<std::list<int>> connectedComponents = clauseGraph.getAllConnectedComponents();
    // Map this back to a vector of vector of clauses.
    // TODO: Consider if std::vector is the best data structure to use here.
    std::vector<std::vector<Pql::Clause>> vectorOfClauseGroups;
    vectorOfClauseGroups.push_back(clausesWithoutSynonyms);
    for (std::list<int> connectedComponent : connectedComponents) {
      std::vector<Pql::Clause> clauseGroup(connectedComponent.size());
      for (int clauseIdx : connectedComponent) {
        Pql::Clause clause = idxToClause.at(clauseIdx);
        clauseGroup.push_back(clause);
      }
      vectorOfClauseGroups.push_back(clauseGroup);
    }

    return vectorOfClauseGroups;
  }

  /**
   * Carries out the following step in Query Optimization:  
   * 2. Sort groups for evaluation
   *    - Start with groups without synonyms.
   *    - Prioritize groups that do not return results in Select.
   */
  void sortGroups(std::vector<Pql::Entity> targets, std::vector<std::vector<Pql::Clause>>& vectorOfGroups) {
    std::unordered_set<Pql::Entity> targetSet(targets.begin(), targets.end());

    std::vector<std::vector<Pql::Clause>> groupsWithoutSynonyms;
    std::vector<std::vector<Pql::Clause>> groupsNotInSelect;
    std::vector<std::vector<Pql::Clause>> groupsInSelect;

    for (std::vector<Pql::Clause> group : vectorOfGroups) {
      bool hasSynonymInSelect = false;

      std::unordered_set<Pql::Entity> synonyms = getGroupSynonyms(group);
      if (synonyms.size() == 0) {
        groupsWithoutSynonyms.emplace_back(group);
        continue;
      } else {
        for (Pql::Entity entity : synonyms) {
          if (targetSet.count(entity)) {
            hasSynonymInSelect = true;
            break;
          }
        }
      }

      if (hasSynonymInSelect) {
        groupsInSelect.emplace_back(group);
      } else {
        groupsNotInSelect.emplace_back(group);
      }
    }

    // Update the vector of groups
    vectorOfGroups = std::move(groupsWithoutSynonyms);
    vectorOfGroups.insert(
        vectorOfGroups.end(),
        std::make_move_iterator(groupsNotInSelect.begin()),
        std::make_move_iterator(groupsNotInSelect.end())
    );
    vectorOfGroups.insert(
        vectorOfGroups.end(),
        std::make_move_iterator(groupsInSelect.begin()),
        std::make_move_iterator(groupsInSelect.end())
    );
  }

  /**
   * Carries out the following step in Query Optimization:
   * 3. Sort clauses inside the group
   *    - Prioritize clauses with one constant and one synonym.
   *    - Prioritize clauses with less number of results.
   *    - Sort clauses such that at least one synonym has been
   *      computed in a previous clause.
   *    - Prioritize with-clauses - more restrictive than such-that clauses.
   *    - Evaluating pattern-clauses - similar to any such-that clause.
   * - Push Affects/* clauses on the last positions in a group.
   */
  void sortWithinGroup(std::vector<Pql::Clause>& clauseGroup) {
    // TODO
    // if clause group has no synonym, no sorting
    std::vector<Pql::Clause> sortedClauseGroup;
    std::unordered_set<Pql::Clause> notVisited(clauseGroup.begin(), clauseGroup.end());

    Pql::Clause firstClause;
    for (Pql::Clause& clause : clauseGroup) {
      if (isClauseWithOneConstantOneSynonym(clause)) {
        firstClause = clause;
        break;
      }
    }

    if (firstClause.getType() == Pql::ClauseType::UNDEFINED) {
      firstClause = clauseGroup[0];
    }

    notVisited.erase(firstClause);

    std::unordered_set<Pql::Entity> paramsComputed;
    for (Pql::Entity e : firstClause.getParams()) {
      if (e.isSynonym()) {
        paramsComputed.insert(e);
      }
    }

    while (!notVisited.empty()) {
      for (Pql::Clause c : notVisited) {
        bool containsComputedParam = false;
        for (Pql::Entity e : c.getParams()) {
          if (e.isSynonym() && paramsComputed.count(e)) {
            containsComputedParam = true;
            break;
          }
        }
        if (containsComputedParam) {
          for (Pql::Entity e : c.getParams()) {
            if (e.isSynonym()) {
              paramsComputed.insert(e);
            }
          }
          notVisited.erase(c);
          break;
        }
      }
     
    }
  }
}

std::vector<std::vector<Pql::Clause>> PqlPreprocessor::sortClauses(std::vector<Pql::Entity> targets, std::vector<Pql::Clause> clauses) {
  // 1. Divide the clauses into multiple groups
  //    - Clauses without synonyms
  //    - Clauses with connected synonyms should be in the same group
  std::vector<std::vector<Pql::Clause>> vectorOfClauseGroups = formGroups(clauses);

  // 2. Sort groups for evaluation
  //    - Start with clauses without synonyms.
  //    - Prioritize groups that do not return results in Select.
  sortGroups(targets,vectorOfClauseGroups);

  // 3. Sort clauses inside the group
  //    - Prioritize clauses with one constant and one synonym.
  //    - Prioritize clauses with less number of results.
  //    - Sort clauses such that at least one synonym has been
  //      computed in a previous clause.
  //    - Prioritize with-clauses - more restrictive than such-that clauses.
  //    - Evaluating pattern-clauses - similar to any such-that clause.
  //    - Push Affects/* clauses on the last positions in a group.
  for (std::vector<Pql::Clause> clauseGroup : vectorOfClauseGroups) {
    sortWithinGroup(clauseGroup);
  }

  return vectorOfClauseGroups;
}
