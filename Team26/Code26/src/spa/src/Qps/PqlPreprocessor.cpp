#include "PqlPreprocessor.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stack>

#include "AdjList.h"

namespace Pql {
  PqlPreprocessor::PqlPreprocessor(std::vector<Clause>& clauses, std::vector<Entity>& targets)
    : clauses(clauses), targets(targets) {
  }

  ClauseGroups PqlPreprocessor::generateClauseGroups() const {
    const int numClauses = clauses.size();
    const int numTargets = targets.size();

    ClauseGroups result;
    result.unusedTargets.reserve(numTargets);

    std::unordered_set<Clause> visitedClauses;
    visitedClauses.reserve(numClauses);

    std::unordered_set<std::string> visitedSynonyms;
    visitedSynonyms.reserve(2 * numClauses + numTargets);

    std::unordered_set<std::string> remainingSynonyms;
    remainingSynonyms.reserve(2 * numClauses + numTargets);

    std::unordered_map<std::string, std::vector<int>> synonymToClauseIdxs;
    synonymToClauseIdxs.reserve(2 * numClauses + numTargets);

    std::unordered_map<int, std::vector<std::string>> clauseIdxToSynonyms;
    clauseIdxToSynonyms.reserve(numClauses);

    // Initailise and add clauses without synonyms
    for (int i = 0; i < (int)clauses.size(); i++) {
      const Clause& currentClause = clauses[i];

      // Skip currentClause if already processed
      if (visitedClauses.count(currentClause) == 1) {
        continue;
      }

      // Add mapping
      const std::vector<Entity>& params = currentClause.getParams();

      bool hasSynonym = false;
      for (const Entity& entity : params) {
        if (entity.isSynonym()) {
          hasSynonym = true;
          const std::string& synonymName = entity.getValue();
          // Add to remainingSynonyms
          remainingSynonyms.emplace(synonymName);
          synonymToClauseIdxs[synonymName].emplace_back(i);
          clauseIdxToSynonyms[i].emplace_back(synonymName);

        }
      }

      if (!hasSynonym) {
        result.withoutSynonyms.emplace(i);
      }
      visitedClauses.emplace(currentClause);
    }

    fillConnectedResults(
      result,
      visitedSynonyms,
      remainingSynonyms,
      synonymToClauseIdxs,
      clauseIdxToSynonyms
    );

    fillUnconnectedResults(
      result,
      visitedSynonyms,
      remainingSynonyms,
      synonymToClauseIdxs,
      clauseIdxToSynonyms
    );

    return result;
  }

  void PqlPreprocessor::fillConnectedResults(
    ClauseGroups& result,
    std::unordered_set<std::string>& visitedSynonyms,
    std::unordered_set<std::string>& remainingSynonyms,
    std::unordered_map<std::string, std::vector<int>>& synonymToClauseIdxs,
    std::unordered_map<int, std::vector<std::string>>& clauseIdxToSynonyms) const {

    std::stack<std::string> stack;
    for (const Entity& target : targets) {
      const std::string& startSynonym = target.getValue();

      // Skip if already processed
      if (visitedSynonyms.count(startSynonym) == 1) {
        continue;
      }

      std::unordered_set<int> currentGroupOfClauses;
      // Push first synonym to stack and mark is visited
      stack.push(startSynonym);
      visitedSynonyms.emplace(startSynonym);
      remainingSynonyms.erase(startSynonym);

      // Start DFS
      while (!stack.empty()) {
        const std::string currentSynonym = stack.top();
        stack.pop();

        // Skip if synonym is not mapped to any clause
        if (synonymToClauseIdxs.count(currentSynonym) == 0) {
          continue;
        }

        const std::vector<int>& connetedClauseIdxs = synonymToClauseIdxs.at(currentSynonym);
        for (const int idx : connetedClauseIdxs) {
          currentGroupOfClauses.emplace(idx);
          for (const std::string& neighbourSynonym : clauseIdxToSynonyms.at(idx)) {
            // If not processed, add to stack and mark as visited
            if (visitedSynonyms.count(neighbourSynonym) == 0) {
              stack.push(neighbourSynonym);
              visitedSynonyms.emplace(neighbourSynonym);
              remainingSynonyms.erase(neighbourSynonym);
            }
          }
          currentGroupOfClauses.emplace(idx);
        }
      }

      // End of dfs. Empty result signify that target is unused.
      if (currentGroupOfClauses.empty()) {
        result.unusedTargets.emplace(target);
      } else {
        result.connected.emplace_back(std::move(currentGroupOfClauses));
      }
    }
  }

  void PqlPreprocessor::fillUnconnectedResults(
    ClauseGroups& result,
    std::unordered_set<std::string>& visitedSynonyms,
    std::unordered_set<std::string>& remainingSynonyms,
    std::unordered_map<std::string, std::vector<int>>& synonymToClauseIdxs,
    std::unordered_map<int, std::vector<std::string>>& clauseIdxToSynonyms) const {

    for (const std::string& startSynonym : remainingSynonyms) {
      std::stack<std::string> stack;

      // Skip if already processed
      if (visitedSynonyms.count(startSynonym) == 1) {
        continue;
      }

      std::unordered_set<int> currentGroupOfClauses;
      // Push first synonym to stack and mark is visited
      stack.push(startSynonym);
      visitedSynonyms.emplace(startSynonym);

      // Start DFS
      while (!stack.empty()) {
        const std::string currentSynonym = stack.top();
        stack.pop();

        // Skip if synonym is not mapped to any clause
        if (synonymToClauseIdxs.count(currentSynonym) == 0) {
          continue;
        }

        const std::vector<int>& connetedClauseIdxs = synonymToClauseIdxs.at(currentSynonym);
        for (const int idx : connetedClauseIdxs) {
          currentGroupOfClauses.emplace(idx);
          for (const std::string& neighbourSynonym : clauseIdxToSynonyms.at(idx)) {
            // If not processed, add to stack and mark as visited
            if (visitedSynonyms.count(neighbourSynonym) == 0) {
              stack.push(neighbourSynonym);
              visitedSynonyms.emplace(neighbourSynonym);
            }
          }
          currentGroupOfClauses.emplace(idx);
        }
      }

      result.disconnected.emplace_back(std::move(currentGroupOfClauses));
    }
  }
}
