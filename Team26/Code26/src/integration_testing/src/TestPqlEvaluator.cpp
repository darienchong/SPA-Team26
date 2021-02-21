#include "catch.hpp"

#include <iostream>
#include <list>
#include <string>
#include <sstream>

#include "PqlEvaluator.h"
#include "PqlParser.h"
#include "PqlQuery.h"
#include "Pkb.h"
#include "Token.h"
#include "Tokeniser.h"

namespace {
  Pkb getPkb() {
    Pkb pkb;
    /*
    Procedure testPqlEvaluator {
      a = 1;
      b = 1  * a + b;
      c = a + b
      if (a == 1) then {
        while (a < 3) {
          a = 1 * a;
        }
      } else {
        a = d + b * c;
      }
      read d;
      print d;
    }
    */

    pkb.addProc("testPqlEvaluator");
    pkb.addConst("1");
    pkb.addConst("3");
    pkb.addVar("a");
    pkb.addVar("b");
    pkb.addVar("c");
    pkb.addVar("d");

    pkb.addAssign(1);
    pkb.addAssign(2);
    pkb.addAssign(3);
    pkb.addAssign(6);
    pkb.addAssign(7);

    pkb.addPatternAssign(1, "a", " 1 ");
    pkb.addPatternAssign(2, "b", " 1 a * b + ");
    pkb.addPatternAssign(3, "c", " a b + ");
    pkb.addPatternAssign(6, "a", " 1 a * ");
    pkb.addPatternAssign(7, "a", " d b c * + ");

    pkb.addIf(4);
    pkb.addWhile(5);
    pkb.addRead(8);
    pkb.addPrint(9);

    pkb.addFollows(1, 2);
    pkb.addFollows(2, 3);
    pkb.addFollows(3, 4);
    pkb.addFollows(4, 8);
    pkb.addFollows(8, 9);

    pkb.addFollowsT(1, 2);
    pkb.addFollowsT(1, 3);
    pkb.addFollowsT(1, 4);
    pkb.addFollowsT(1, 8);
    pkb.addFollowsT(1, 9);
    pkb.addFollowsT(2, 3);
    pkb.addFollowsT(2, 4);
    pkb.addFollowsT(2, 8);
    pkb.addFollowsT(2, 9);
    pkb.addFollowsT(3, 4);
    pkb.addFollowsT(3, 8);
    pkb.addFollowsT(3, 9);
    pkb.addFollowsT(4, 8);
    pkb.addFollowsT(4, 9);
    pkb.addFollowsT(8, 9);

    pkb.addParent(4, 5);
    pkb.addParent(5, 6);
    pkb.addParent(4, 7);

    pkb.addParentT(4, 5);
    pkb.addParentT(4, 6);
    pkb.addParentT(4, 7);
    pkb.addParentT(5, 6);

    pkb.addUsesS(2, "a");
    pkb.addUsesS(2, "b");
    pkb.addUsesS(3, "a");
    pkb.addUsesS(3, "b");
    pkb.addUsesS(4, "a");
    pkb.addUsesS(5, "a");
    pkb.addUsesS(6, "a");
    pkb.addUsesS(7, "a");
    pkb.addUsesS(7, "b");
    pkb.addUsesS(7, "c");
    pkb.addUsesS(9, "d");

    pkb.addModifiesS(1, "a");
    pkb.addModifiesS(2, "b");
    pkb.addModifiesS(3, "c");
    pkb.addModifiesS(6, "a");
    pkb.addModifiesS(7, "a");
    pkb.addModifiesS(8, "d");

    return pkb;
  }
}


TEST_CASE("[TestPqlEvaluator] Valid Query, Single Select") {
  SECTION("Single Select") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Query query;
    query.setTarget(target);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluator] Valid Query, Single Select, Single such that clause") {
  SECTION("Line number") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::STMT_NUMBER, "1") , Pql::Entity(Pql::EntityType::STMT_NUMBER, "2") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One synonym on left hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::PARENT_T,
      { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::STMT_NUMBER, "6") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "4", "5" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One synonym on right hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS_T,
      { Pql::Entity(Pql::EntityType::STMT_NUMBER, "1") , Pql::Entity(Pql::EntityType::STMT, "s") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "2", "3", "4", "8" ,"9" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on left hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS_T,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::STMT, "3") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "5" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on right hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::READ, "r");
    Pql::Clause clause(Pql::ClauseType::PARENT,
      { Pql::Entity(Pql::EntityType::STMT_NUMBER, "4") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "8" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on left hand side, synonym on right hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::VARIABLE, "v");
    Pql::Clause clause(Pql::ClauseType::MODIFIES_S,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "a", "b", "c", "d" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on right hand side, synonym on left hand side") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1","2","3","4","8" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two wildcards") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::PRINT, "pn");
    Pql::Clause clause(Pql::ClauseType::USES_S,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();

    // Build expected result
    std::list<std::string> expectedResult = { "9" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two synonyms unrelated to Select target") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::STMT, "s1") , Pql::Entity(Pql::EntityType::STMT, "s2") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1","2","3","6","7" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two synonyms, one related to Select target") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::PARENT,
      { Pql::Entity(Pql::EntityType::WHILE, "w") , Pql::Entity(Pql::EntityType::STMT, "s2") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "5" };

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluator] Valid Query, Single Select, Single pattern assign clause") {
  SECTION("Unrelated and none result") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::VARIABLE, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, "0") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = {};

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Unrelated to Select target") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a1");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a2"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "2", "3", "6", "7" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, term)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.push_back("1");

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, sub expression)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "6" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, wildcard)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "6", "7" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, term)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, sub expression)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "2", "6" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, expression)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "6" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, wildcard)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "2", "3", "6", "7" };

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluator] Valid Query, Single Select, Single such that clause, Single pattern assign clause") {
  SECTION("Parent, both related") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::PARENT, {
      Pql::Entity(Pql::EntityType::STMT_NUMBER, "5"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "6" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("ParentT, one indirectly related") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::PARENT_T, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "4", "5" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Follows, one indirectly related") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::FOLLOWS, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("FollowsT, both related") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " a b + ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { "1", "2" };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("UsesS, both unrelated to Select target") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"), Pql::Entity(Pql::EntityType::VARIABLE_NAME, "b") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = {  };

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("ModifiesS, both not unrelated to Select and unrelated to each other") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::MODIFIES_S, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"), Pql::Entity(Pql::EntityType::VARIABLE_NAME, "d") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.setTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult = { };

    REQUIRE(evaluationResult == expectedResult);
  }
}
