#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "PqlQuery.h"

namespace Pql {
  /**
   * Struct to store the processed groups of clauses.
   *
   * There are 3 types of Clauses:
   *   1. Clauses without synonyms
   *   2. Clauses not connected to the select targets
   *   3. Clauses connected to the select targets
   * And one type of Entity:
   *   4. Unused select targets
   */
  struct ClauseGroups {
    std::unordered_set<int> withoutSynonyms;
    std::vector<std::unordered_set<int>> disconnected;
    std::vector<std::unordered_set<int>> connected;
    std::unordered_set<Entity> unusedTargets;
  };

  class PqlPreprocessor {
  private:
    std::vector<Clause>& clauses;
    std::vector<Entity>& targets;

  public:
    /**
     * Constructs the Preprocessor using the given clauses and select targets.
     *
     * @param clauses All clauses to process.
     * @param targets All targets.
     */
    PqlPreprocessor(std::vector<Clause>& clauses, std::vector<Entity>& targets);

    /**
     * Constructs 3 different clause groups and return them.
     * 
     * The 3 types of clause groups.
     *   1. Clauses without synonyms
     *   2. Clauses not connected to the select targets
     *   3. Clauses connected to the select targets
     *   4. Unused select targets
     * 
     * @return Clause groups.
     */
    ClauseGroups generateClauseGroups() const;

  private:
    /**
     * Constructs the group of clauses that are connected to the select targets.
     * 
     * @param result Result to be constructed. 
     * @param visitedSynonyms Set of visited synonyms. 
     * @param remainingSynonyms Set of remaining synonyms.
     * @param synonymToClauseIdxs Map of synonym to clause indexes with the synonym.
     * @param clauseIdxToSynonyms Map of clause index to synonyms the clause contains.
    */
    void fillConnectedResults(
      ClauseGroups& result,
      std::unordered_set<std::string>& visitedSynonyms,
      std::unordered_set<std::string>& remainingSynonyms,
      std::unordered_map<std::string, std::vector<int>>& synonymToClauseIdxs,
      std::unordered_map<int, std::vector<std::string>>& clauseIdxToSynonyms) const;

    /**
     * Constructs the group of clauses that are disconnected to the select targets.
     *
     * @param result Result to be constructed.
     * @param visitedSynonyms Set of visited synonyms.
     * @param remainingSynonyms Set of remaining synonyms.
     * @param synonymToClauseIdxs Map of synonym to clause indexes with the synonym.
     * @param clauseIdxToSynonyms Map of clause index to synonyms the clause contains.
    */
    void fillUnconnectedResults(
      ClauseGroups& result,
      std::unordered_set<std::string>& visitedSynonyms,
      std::unordered_set<std::string>& remainingSynonyms,
      std::unordered_map<std::string, std::vector<int>>& synonymToClauseIdxs,
      std::unordered_map<int, std::vector<std::string>>& clauseIdxToSynonyms) const;
  };
}