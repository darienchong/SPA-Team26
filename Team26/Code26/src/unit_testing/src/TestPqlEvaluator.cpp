#include "catch.hpp"

#include <iostream>
#include <list>
#include <string>
#include <sstream>

#include "PqlEvaluator.h"
#include "PqlParser.h"
#include "PqlQuery.h"
#include "Token.h"
#include "Tokeniser.h"


TEST_CASE("[TestPqlEvaluator] Valid Query") {
  SECTION("Single declaration, Single Select") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Query query;
	query.setTarget(target);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause line number") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addFollows(1, 2);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::LINE_NUMBER, "1") , Pql::Entity(Pql::EntityType::LINE_NUMBER, "2") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one synonym on left hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addFollows(1, 2);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::LINE_NUMBER, "2") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one synonym on right hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addFollows(1, 2);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::LINE_NUMBER, "1") , Pql::Entity(Pql::EntityType::STMT, "s") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("2");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one wildcard on left hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::STMT, "3") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one wildcard on right hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::LINE_NUMBER, "2") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one wildcard on left hand side, synonym on right hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::STMT, "s") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause one wildcard on right hand side, synonym on left hand side") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::STMT, "s") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause two wildcards") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::WILDCARD, "_") , Pql::Entity(Pql::EntityType::WILDCARD, "_") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two declarations, Single Select, Single such that clause two synonyms unrelated to Select target") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s1");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::STMT, "s2") , Pql::Entity(Pql::EntityType::STMT, "s3") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Two declarations, Single Select, Single such that clause two synonyms related to Select target") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addStmt(1);
	pkb.addStmt(2);
	pkb.addStmt(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::STMT, "s1");
	Pql::Clause clause(Pql::ClauseType::FOLLOWS,
	  { Pql::Entity(Pql::EntityType::STMT, "s1") , Pql::Entity(Pql::EntityType::STMT, "s2") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - None result") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a1");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a2"),
	  Pql::Entity(Pql::EntityType::VARIABLE, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, "0") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - unrelated to Select target") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a1");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a2"),
	  Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, "1") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (synonym, term)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::EXPRESSION, "1") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (synonym, sub expression)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::VARIABLE_NAME, "c"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "a") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (synonym, wildcard)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::VARIABLE_NAME, "a"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (wildcard, term)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::EXPRESSION, "a") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("2");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (wildcard, sub expression)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "a") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single pattern assignment clause - related to Select target, (wildcard, wildcard)") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addPatternAssign(1, "a", "1");
	pkb.addPatternAssign(2, "b", "a");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::WILDCARD, "_") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");
	expectedResult.push_back("2");
	expectedResult.push_back("3");

	REQUIRE(evaluationResult == expectedResult);
  }

  SECTION("Single declaration, Single Select, Single such that clause and single pattern assignment clause") {
	// Populate pkb to test
	Pkb pkb;
	pkb.addAssign(1);
	pkb.addAssign(2);
	pkb.addAssign(3);
	pkb.addFollows(1, 2);
	pkb.addFollows(2, 3);
	pkb.addPatternAssign(1, "a", "1 + 2");
	pkb.addPatternAssign(2, "b", "1 + 2");
	pkb.addPatternAssign(3, "c", "a b +");
	pkb.addVar("a");
	pkb.addVar("b");
	pkb.addVar("c");

	// Create PQL query to test
	Pql::Entity target(Pql::EntityType::ASSIGN, "a");
	Pql::Clause clause1(Pql::ClauseType::FOLLOWS, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"), Pql::Entity(Pql::EntityType::LINE_NUMBER, "2") });
	Pql::Clause clause2(Pql::ClauseType::PATTERN_ASSIGN, {
	  Pql::Entity(Pql::EntityType::ASSIGN, "a"),
	  Pql::Entity(Pql::EntityType::WILDCARD, "_"), Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "1 + 2") });
	Pql::Query query;
	query.setTarget(target);
	query.addClause(clause1);
	query.addClause(clause2);

	// Initialise results to store results of evaluation
	std::list<std::string> evaluationResult;

	// Get Evaluation result
	Pql::PqlEvaluator pqlEvaluator(pkb, query, evaluationResult);
	pqlEvaluator.evaluateQuery();

	// Build expected result
	std::list<std::string> expectedResult;
	expectedResult.push_back("1");

	REQUIRE(evaluationResult == expectedResult);
  }
}
