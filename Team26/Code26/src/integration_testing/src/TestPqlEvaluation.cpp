#include "catch.hpp"

#include <iostream>
#include <list>
#include <sstream>
#include <string>

#include "Pkb.h"
#include "PqlEvaluator.h"
#include "PqlParser.h"
#include "PqlQuery.h"
#include "Token.h"
#include "Tokeniser.h"

namespace {
  Pkb getPkb() {
    Pkb pkb;
    /*
    procedure TestPqlEvaluation {
      a = 1;
      b = 1 * a + b;
      c = a + b;
      if (a == 1) then {
        while (a < 3) {
          a = 1 * a;
        }
      } else {
        a = d + b * c;
      }
      read d;
      print d;
      call next;
    }
    
    procedure next {
      a = 3;
    }
    */

    // Add procs
    pkb.addProc("TestPqlEvaluation");
    pkb.addProc("next");

    // Add consts
    pkb.addConst("1");
    pkb.addConst("3");

    // Add Vars
    pkb.addVar("a");
    pkb.addVar("b");
    pkb.addVar("c");
    pkb.addVar("d");

    // Add stmt 1 to 11
    pkb.addAssign(1);
    pkb.addAssign(2);
    pkb.addAssign(3);
    pkb.addIf(4);
    pkb.addWhile(5);
    pkb.addAssign(6);
    pkb.addAssign(7);
    pkb.addRead(8);
    pkb.addPrint(9);
    pkb.addCall(10);
    pkb.addAssign(11);

    // Add Mappings
    pkb.addReadVar(8, "d");
    pkb.addPrintVar(9, "d");
    pkb.addCallProc(10, "next");

    // Add pattern assign
    pkb.addPatternAssign(1, "a", " 1 ");
    pkb.addPatternAssign(2, "b", " 1 a * b + ");
    pkb.addPatternAssign(3, "c", " a b + ");
    pkb.addPatternAssign(6, "a", " 1 a * ");
    pkb.addPatternAssign(7, "a", " d b c * + ");
    pkb.addPatternAssign(11, "a", " 3 ");

    // Add pattern if
    pkb.addPatternIf(4, "a");

    // Add pattern while
    pkb.addPatternWhile(5, "a");

    // Add Follows 
    pkb.addFollows(1, 2);
    pkb.addFollows(2, 3);
    pkb.addFollows(3, 4);
    pkb.addFollows(4, 8);
    pkb.addFollows(8, 9);
    pkb.addFollows(9, 10);

    // Add FollowsT
    pkb.addFollowsT(1, 2);
    pkb.addFollowsT(1, 3);
    pkb.addFollowsT(1, 4);
    pkb.addFollowsT(1, 8);
    pkb.addFollowsT(1, 9);
    pkb.addFollowsT(1, 10);
    pkb.addFollowsT(2, 3);
    pkb.addFollowsT(2, 4);
    pkb.addFollowsT(2, 8);
    pkb.addFollowsT(2, 9);
    pkb.addFollowsT(2, 10);
    pkb.addFollowsT(3, 4);
    pkb.addFollowsT(3, 8);
    pkb.addFollowsT(3, 9);
    pkb.addFollowsT(4, 8);
    pkb.addFollowsT(4, 9);
    pkb.addFollowsT(4, 10);
    pkb.addFollowsT(8, 9);
    pkb.addFollowsT(8, 10);

    // Add Parent
    pkb.addParent(4, 5);
    pkb.addParent(5, 6);
    pkb.addParent(4, 7);

    // Add ParentT
    pkb.addParentT(4, 5);
    pkb.addParentT(4, 6);
    pkb.addParentT(4, 7);
    pkb.addParentT(5, 6);

    // Add UsesS
    pkb.addUsesS(2, "a");
    pkb.addUsesS(2, "b");
    pkb.addUsesS(3, "a");
    pkb.addUsesS(3, "b");
    pkb.addUsesS(4, "a");
    pkb.addUsesS(4, "b");
    pkb.addUsesS(4, "c");
    pkb.addUsesS(4, "d");
    pkb.addUsesS(5, "a");
    pkb.addUsesS(6, "a");
    pkb.addUsesS(7, "a");
    pkb.addUsesS(7, "b");
    pkb.addUsesS(7, "c");
    pkb.addUsesS(7, "d");
    pkb.addUsesS(9, "d");

    // Add UsesP
    pkb.addUsesP("TestPqlEvaluation", "a");
    pkb.addUsesP("TestPqlEvaluation", "b");
    pkb.addUsesP("TestPqlEvaluation", "c");
    pkb.addUsesP("TestPqlEvaluation", "d");

    // Add ModifiesS
    pkb.addModifiesS(1, "a");
    pkb.addModifiesS(2, "b");
    pkb.addModifiesS(3, "c");
    pkb.addModifiesS(6, "a");
    pkb.addModifiesS(7, "a");
    pkb.addModifiesS(8, "d");
    pkb.addModifiesS(11, "a");

    // Add ModifiesP
    pkb.addModifiesP("TestPqlEvaluation", "a");
    pkb.addModifiesP("TestPqlEvaluation", "b");
    pkb.addModifiesP("TestPqlEvaluation", "c");
    pkb.addModifiesP("TestPqlEvaluation", "d");
    pkb.addModifiesP("next", "a");

    // Add Calls
    pkb.addCalls("TestPqlEvaluation", "next");

    // Add CallsT
    pkb.addCallsT("TestPqlEvaluation", "next");

    // Add Next
    pkb.addNext(1, 2);
    pkb.addNext(2, 3);
    pkb.addNext(3, 4);
    pkb.addNext(4, 5);
    pkb.addNext(5, 6);
    pkb.addNext(6, 5);
    pkb.addNext(5, 8);
    pkb.addNext(4, 7);
    pkb.addNext(7, 8);
    pkb.addNext(8, 9);
    pkb.addNext(9, 10);

    // Add NextT
    pkb.addNextT(1, 2);
    pkb.addNextT(1, 3);
    pkb.addNextT(1, 4);
    pkb.addNextT(1, 5);
    pkb.addNextT(1, 6);
    pkb.addNextT(1, 7);
    pkb.addNextT(1, 8);
    pkb.addNextT(1, 9);
    pkb.addNextT(1, 10);
    pkb.addNextT(2, 3);
    pkb.addNextT(2, 4);
    pkb.addNextT(2, 5);
    pkb.addNextT(2, 6);
    pkb.addNextT(2, 7);
    pkb.addNextT(2, 8);
    pkb.addNextT(2, 9);
    pkb.addNextT(2, 10);
    pkb.addNextT(3, 4);
    pkb.addNextT(3, 5);
    pkb.addNextT(3, 6);
    pkb.addNextT(3, 7);
    pkb.addNextT(3, 8);
    pkb.addNextT(3, 9);
    pkb.addNextT(3, 10);
    pkb.addNextT(4, 5);
    pkb.addNextT(4, 6);
    pkb.addNextT(4, 7);
    pkb.addNextT(4, 8);
    pkb.addNextT(4, 9);
    pkb.addNextT(4, 10);
    pkb.addNextT(5, 5);
    pkb.addNextT(5, 6);
    pkb.addNextT(5, 8);
    pkb.addNextT(5, 9);
    pkb.addNextT(5, 10);
    pkb.addNextT(6, 5);
    pkb.addNextT(6, 6);
    pkb.addNextT(6, 8);
    pkb.addNextT(6, 9);
    pkb.addNextT(6, 10);
    pkb.addNextT(7, 8);
    pkb.addNextT(7, 9);
    pkb.addNextT(7, 10);
    pkb.addNextT(8, 9);
    pkb.addNextT(8, 10);
    pkb.addNextT(9, 10);

    // Add Affects
    pkb.addAffects(1, 2);
    pkb.addAffects(1, 3);
    pkb.addAffects(1, 6);
    pkb.addAffects(2, 3);
    pkb.addAffects(2, 7);
    pkb.addAffects(3, 7);
    pkb.addAffects(6, 6);

    // Add AffectsT
    pkb.addAffectsT(1, 2);
    pkb.addAffectsT(1, 3);
    pkb.addAffectsT(1, 6);
    pkb.addAffectsT(1, 7);
    pkb.addAffectsT(2, 3);
    pkb.addAffectsT(2, 7);
    pkb.addAffectsT(3, 7);
    pkb.addAffectsT(6, 6);

    // Add NextBip
    pkb.addNextBip(1, 2);
    pkb.addNextBip(2, 3);
    pkb.addNextBip(3, 4);
    pkb.addNextBip(4, 5);
    pkb.addNextBip(5, 6);
    pkb.addNextBip(6, 5);
    pkb.addNextBip(5, 8);
    pkb.addNextBip(4, 7);
    pkb.addNextBip(7, 8);
    pkb.addNextBip(8, 9);
    pkb.addNextBip(9, 10);
    pkb.addNextBip(10, 11);

    // Add NextBipT
        // Add NextT
    pkb.addNextBipT(1, 2);
    pkb.addNextBipT(1, 3);
    pkb.addNextBipT(1, 4);
    pkb.addNextBipT(1, 5);
    pkb.addNextBipT(1, 6);
    pkb.addNextBipT(1, 7);
    pkb.addNextBipT(1, 8);
    pkb.addNextBipT(1, 9);
    pkb.addNextBipT(1, 10);
    pkb.addNextBipT(1, 11);
    pkb.addNextBipT(2, 3);
    pkb.addNextBipT(2, 4);
    pkb.addNextBipT(2, 5);
    pkb.addNextBipT(2, 6);
    pkb.addNextBipT(2, 7);
    pkb.addNextBipT(2, 8);
    pkb.addNextBipT(2, 9);
    pkb.addNextBipT(2, 10);
    pkb.addNextBipT(2, 11);
    pkb.addNextBipT(3, 4);
    pkb.addNextBipT(3, 5);
    pkb.addNextBipT(3, 6);
    pkb.addNextBipT(3, 7);
    pkb.addNextBipT(3, 8);
    pkb.addNextBipT(3, 9);
    pkb.addNextBipT(3, 10);
    pkb.addNextBipT(3, 11);
    pkb.addNextBipT(4, 5);
    pkb.addNextBipT(4, 6);
    pkb.addNextBipT(4, 7);
    pkb.addNextBipT(4, 8);
    pkb.addNextBipT(4, 9);
    pkb.addNextBipT(4, 10);
    pkb.addNextBipT(4, 11);
    pkb.addNextBipT(5, 5);
    pkb.addNextBipT(5, 6);
    pkb.addNextBipT(5, 8);
    pkb.addNextBipT(5, 9);
    pkb.addNextBipT(5, 10);
    pkb.addNextBipT(5, 11);
    pkb.addNextBipT(6, 5);
    pkb.addNextBipT(6, 6);
    pkb.addNextBipT(6, 8);
    pkb.addNextBipT(6, 9);
    pkb.addNextBipT(6, 10);
    pkb.addNextBipT(6, 11);
    pkb.addNextBipT(7, 8);
    pkb.addNextBipT(7, 9);
    pkb.addNextBipT(7, 10);
    pkb.addNextBipT(7, 11);
    pkb.addNextBipT(8, 9);
    pkb.addNextBipT(8, 10);
    pkb.addNextBipT(9, 10);
    pkb.addNextBipT(9, 11);
    pkb.addNextBipT(10, 11);

    // Add AffectsBip
    pkb.addAffectsBip(1, 2);
    pkb.addAffectsBip(1, 3);
    pkb.addAffectsBip(1, 6);
    pkb.addAffectsBip(2, 3);
    pkb.addAffectsBip(2, 7);
    pkb.addAffectsBip(3, 7);
    pkb.addAffectsBip(6, 6);

    // Add AffectsBipT
    pkb.addAffectsBipT(1, 2);
    pkb.addAffectsBipT(1, 3);
    pkb.addAffectsBipT(1, 6);
    pkb.addAffectsBipT(1, 7);
    pkb.addAffectsBipT(2, 3);
    pkb.addAffectsBipT(2, 7);
    pkb.addAffectsBipT(3, 7);
    pkb.addAffectsBipT(6, 6);

    return pkb;
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Select", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Single Select") {
    SECTION("Empty Result") {
      // Create PQL query to test
      Pql::Entity target(Pql::EntityType::PROG_LINE, "n");
      Pql::Clause clause(Pql::ClauseType::WITH,
        { Pql::Entity(Pql::EntityType::NUMBER, "1") , Pql::Entity(Pql::EntityType::NUMBER, "2") });

      Pql::Query query;
      query.addTarget(target);
      query.addClause(clause);

      // Get Evaluation result
      std::list<std::string> evaluationResult;
      Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
      pqlEvaluator.evaluateQuery();
      evaluationResult.sort();

      // Build expected result
      std::list<std::string> expectedResult;
      expectedResult.sort();

      REQUIRE(evaluationResult == expectedResult);
    }

    SECTION("Non-Empty Resut") {
      // Create PQL query to test
      Pql::Entity target(Pql::EntityType::PROG_LINE, "n");
      Pql::Query query;
      query.addTarget(target);

      // Get Evaluation result
      std::list<std::string> evaluationResult;
      Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
      pqlEvaluator.evaluateQuery();
      evaluationResult.sort();

      // Build expected result
      std::list<std::string> expectedResult{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
      expectedResult.sort();

      REQUIRE(evaluationResult == expectedResult);
    }
  }

  SECTION("BOOLEAN Select") {
    SECTION("TRUE") {
      // Create PQL query to test
      Pql::Query query;

      // Get Evaluation result
      std::list<std::string> evaluationResult;
      Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
      pqlEvaluator.evaluateQuery();
      evaluationResult.sort();

      // Build expected result
      std::list<std::string> expectedResult{ "TRUE" };
      expectedResult.sort();

      REQUIRE(evaluationResult == expectedResult);
    }

    SECTION("FALSE") {
      // Create PQL query to test
      Pql::Clause clause(Pql::ClauseType::WITH,
        { Pql::Entity(Pql::EntityType::NUMBER, "1") , Pql::Entity(Pql::EntityType::NUMBER, "2") });

      Pql::Query query;
      query.addClause(clause);

      // Get Evaluation result
      std::list<std::string> evaluationResult;
      Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
      pqlEvaluator.evaluateQuery();
      evaluationResult.sort();

      // Build expected result
      std::list<std::string> expectedResult{ "FALSE" };
      expectedResult.sort();

      REQUIRE(evaluationResult == expectedResult);
    }
  }

  SECTION("Tuple Select") {
    // Create PQL query to test
    Pql::Entity target1(Pql::EntityType::IF, "ifs");
    Pql::Entity target2(Pql::EntityType::WHILE, "w");
    Pql::Query query;
    query.addTarget(target1);
    query.addTarget(target2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4 5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("AttrRef Select") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::PROC_NAME);
    Pql::Query query;
    query.addTarget(target);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "next" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single such that clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Line number") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::NUMBER, "1") , Pql::Entity(Pql::EntityType::NUMBER, "2") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
    expectedResult.sort();
    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One synonym on left hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::PARENT_T,
      { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::NUMBER, "6") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4", "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One synonym on right hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS_T,
      { Pql::Entity(Pql::EntityType::NUMBER, "1") , Pql::Entity(Pql::EntityType::STMT, "s") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2", "3", "4", "8" ,"9", "10" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on left hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS_T,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::STMT, "3") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on right hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::READ, "r");
    Pql::Clause clause(Pql::ClauseType::PARENT,
      { Pql::Entity(Pql::EntityType::NUMBER, "4") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "8" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on left hand side, synonym on right hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::PROG_LINE, "n");
    Pql::Clause clause(Pql::ClauseType::NEXT,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::PROG_LINE, "n") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2", "3", "4", "5", "6", "7", "8", "9", "10" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("One wildcard on right hand side, synonym on left hand side") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "4", "8", "9" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two wildcards") {
    // Create PQL query to test
    Pql::Clause clause(Pql::ClauseType::CALLS,
      { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();

    // Build expected result
    std::list<std::string> expectedResult{ "TRUE" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two synonyms unrelated to Select target") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::FOLLOWS,
      { Pql::Entity(Pql::EntityType::STMT, "s1") , Pql::Entity(Pql::EntityType::STMT, "s2") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two synonyms, one related to Select target") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::PARENT,
      { Pql::Entity(Pql::EntityType::WHILE, "w") , Pql::Entity(Pql::EntityType::STMT, "s2") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single pattern assign clause") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Unrelated to Select target, (variable, sub expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a2");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1","2","3","6","7","11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Unrelated to select target, none result, (variable, expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a2");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"), Pql::Entity(Pql::EntityType::EXPRESSION, "0") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (variable, wildcard)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::VARIABLE, "v");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "a", "b", "c" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Unrelated to Select target, (synonym, sub expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a1");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a2"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Unrelated to Select target, (synonym, expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a1");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a2"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, term)") {
    // Populate pkb to test
    Pkb pkb = getPkb();

    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, sub expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "6" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();

    // Build expected result
    std::list<std::string> expectedResult{ "6" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (synonym, wildcard)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, term)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, sub expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2", "6" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, expression)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "6" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, (wildcard, wildcard)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single pattern if clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Related to Select target, wildcard") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::IF, "ifs");
    Pql::Clause clause(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, synonym") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::IF, "ifs");
    Pql::Clause clause1(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Clause clause2(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::NUMBER, "2"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v") });

    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);
    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, variable name") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::IF, "ifs");
    Pql::Clause clause(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::NAME, "b") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single pattern while clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Related to Select target, wildcard") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, synonym") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause1(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Clause clause2(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::NUMBER, "9"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v") });

    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);
    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Related to Select target, variable name") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::NAME, "a") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single such that clause, Single pattern assign clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Parent, both related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::PARENT, {
      Pql::Entity(Pql::EntityType::NUMBER, "5"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "6" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("ParentT, one indirectly related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::PARENT_T, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 a * ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4", "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Follows, one indirectly related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::FOLLOWS, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("FollowsT, both related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::STMT, "s"), Pql::Entity(Pql::EntityType::ASSIGN, "a") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, " a b + ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("UsesS, both unrelated to Select target") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"), Pql::Entity(Pql::EntityType::NAME, "b") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "a"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " 1 ") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("ModifiesS, both unrelated to Select and unrelated to each other") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::MODIFIES_S, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"), Pql::Entity(Pql::EntityType::NAME, "d") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult;
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single such that clause, Single pattern if clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Next, both related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::IF, "ifs");
    Pql::Clause clause1(Pql::ClauseType::NEXT, {
      Pql::Entity(Pql::EntityType::PROG_LINE, "3"), Pql::Entity(Pql::EntityType::IF, "ifs") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("NextT, both related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::IF, "ifs");
    Pql::Clause clause1(Pql::ClauseType::NEXT_T, {
      Pql::Entity(Pql::EntityType::PROG_LINE, "1"), Pql::Entity(Pql::EntityType::IF, "ifs") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::NAME, "a") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "4" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("UsesP, one indirectly related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::PROCEDURE, "proc");
    Pql::Clause clause1(Pql::ClauseType::USES_P, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "proc"), Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "TestPqlEvaluation" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("ModifiesP, one indirectly related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::PROCEDURE, "proc");
    Pql::Clause clause1(Pql::ClauseType::MODIFIES_P, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "proc"), Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "TestPqlEvaluation", "next" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("UsesS, both unrelated") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::NAME, "a") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Calls, both unrelated and unrelated to each other") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::CALLS, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "TestPqlEvaluation"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single such that clause, Single pattern while clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("NextBip, both related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::WHILE, "w");
    Pql::Clause clause1(Pql::ClauseType::NEXT_BIP, {
      Pql::Entity(Pql::EntityType::WHILE, "w"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "5" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("NextBipT, one indirectly related") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::VARIABLE, "v");
    Pql::Clause clause1(Pql::ClauseType::NEXT_BIP_T, {
      Pql::Entity(Pql::EntityType::WHILE, "w"), Pql::Entity(Pql::EntityType::PROG_LINE, "11") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"), Pql::Entity(Pql::EntityType::VARIABLE, "v") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "a" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("CallsT, both unrelated and unrelated to each other") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::CALL, "cl");
    Pql::Clause clause1(Pql::ClauseType::CALLS_T, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "proc"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Clause clause2(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "10" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single with clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("no attr ref mapping (attr ref, attr ref)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::STMT, "s");
    Pql::Clause clause1(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::STMT, "s", Pql::AttributeRefType::STMT_NUMBER),
      Pql::Entity(Pql::EntityType::PROG_LINE, "n", Pql::AttributeRefType::NONE) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("no attr ref mapping (attr ref, attr ref)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
      Pql::Entity(Pql::EntityType::CONSTANT, "c", Pql::AttributeRefType::VALUE) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "3" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("attr ref mapping (attr ref, attr ref)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::READ, "r");
    Pql::Clause clause1(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::READ, "r", Pql::AttributeRefType::VAR_NAME),
      Pql::Entity(Pql::EntityType::PRINT, "pr", Pql::AttributeRefType::VAR_NAME) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "8" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

TEST_CASE("[TestPqlEvaluation] Valid Query, Single Select, Single such that clause, Single with clause", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();

  SECTION("Affects, both unrelated, (constant, constant)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::AFFECTS, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::PROG_LINE, "2") });
    Pql::Clause clause2(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::NUMBER, "10", Pql::AttributeRefType::NONE),
      Pql::Entity(Pql::EntityType::NUMBER, "10", Pql::AttributeRefType::NONE) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("AffectsT, both unrelated and not related to each other, (constant, attr ref)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::AFFECTS_T, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
      Pql::Entity(Pql::EntityType::ASSIGN, "a2") });
    Pql::Clause clause2(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::NAME, "d", Pql::AttributeRefType::NONE),
      Pql::Entity(Pql::EntityType::READ, "r", Pql::AttributeRefType::VAR_NAME) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "1", "2", "3", "6", "7", "11" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("AffectsBip, one indirectly related, (attr ref, constant)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::AFFECTS_BIP, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::ASSIGN, "a1") });
    Pql::Clause clause2(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a1", Pql::AttributeRefType::STMT_NUMBER),
      Pql::Entity(Pql::EntityType::NUMBER, "7", Pql::AttributeRefType::NONE) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2", "3" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("AffectsBipT, both related, (attr ref, attr ref)") {
    // Create PQL query to test
    Pql::Entity target(Pql::EntityType::ASSIGN, "a");
    Pql::Clause clause1(Pql::ClauseType::AFFECTS_BIP_T, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::ASSIGN, "a1") });
    Pql::Clause clause2(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
      Pql::Entity(Pql::EntityType::NUMBER, "2", Pql::AttributeRefType::NONE) });
    Pql::Query query;
    query.addTarget(target);
    query.addClause(clause1);
    query.addClause(clause2);

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

}

TEST_CASE("[TestPqlEvaluation] Valid Query, Complex query", "[PqlEvaluator]") {
  // Populate pkb to test
  Pkb pkb = getPkb();
  SECTION("Tuple Select, two targets, Single clause") {
    // Create PQL query to test
    std::vector<Pql::Entity> targets{
      Pql::Entity(Pql::EntityType::STMT, "s"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v1", Pql::AttributeRefType::VAR_NAME),
      Pql::Entity(Pql::EntityType::PRINT, "p", Pql::AttributeRefType::VAR_NAME)
    };
    std::vector<Pql::Clause> clauses{
      Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::STMT, "s"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v1")
      }),
      Pql::Clause(Pql::ClauseType::AFFECTS_T, {
        Pql::Entity(Pql::EntityType::STMT, "s"),
        Pql::Entity(Pql::EntityType::NUMBER, "7")
      }),
      Pql::Clause(Pql::ClauseType::PATTERN_WHILE, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v2")
      }),
      Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::VARIABLE, "v1", Pql::AttributeRefType::VAR_NAME),
        Pql::Entity(Pql::EntityType::VARIABLE, "v2", Pql::AttributeRefType::VAR_NAME)
      })
    };

    Pql::Query query;
    for (const Pql::Entity& target : targets) {
      query.addTarget(target);
    }
    for (const Pql::Clause& clause : clauses) {
      query.addClause(clause);
    }

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "2 a d", "3 a d" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("BOOLEAN Select, Two clauses, related, none empty, TRUE") {
    // Create PQL query to test
    std::vector<Pql::Clause> clauses{
      Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
        Pql::Entity(Pql::EntityType::STMT, "s", Pql::AttributeRefType::STMT_NUMBER)
      }),
      Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
        Pql::Entity(Pql::EntityType::NUMBER, "2", Pql::AttributeRefType::NONE)
      })
    };

    Pql::Query query;
    for (const Pql::Clause& clause : clauses) {
      query.addClause(clause);
    }

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "TRUE" };
    expectedResult.sort();

  }

  SECTION("BOOLEAN Select, Two clauses, related, none empty, FALSE") {
    // Create PQL query to test
    std::vector<Pql::Clause> clauses{
      Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
        Pql::Entity(Pql::EntityType::NUMBER, "1", Pql::AttributeRefType::NONE)
      }),
      Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER),
        Pql::Entity(Pql::EntityType::NUMBER, "2", Pql::AttributeRefType::NONE)
      })
    };

    Pql::Query query;
    for (const Pql::Clause& clause : clauses) {
      query.addClause(clause);
    }

    // Get Evaluation result
    std::list<std::string> evaluationResult;
    Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
    pqlEvaluator.evaluateQuery();
    evaluationResult.sort();

    // Build expected result
    std::list<std::string> expectedResult{ "FALSE" };
    expectedResult.sort();

    REQUIRE(evaluationResult == expectedResult);
  }
}

