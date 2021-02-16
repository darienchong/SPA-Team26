#include <iostream>
#include <list>
#include <string>
#include <sstream>

#include "PqlParser.h"
#include "PqlQuery.h"
#include "Token.h"
#include "Tokeniser.h"
#include "catch.hpp"

namespace {
  Tokeniser tokeniser;

  std::list<Token> queryStringToTokens(std::string& queryString) {
    std::stringstream ss;
    ss << queryString << std::endl;
    return tokeniser.tokenise(ss);
  }
}

TEST_CASE("[TestPqlParser] Valid Query") {
  SECTION("Single declaration, Single Select") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; 
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Multiple declarations") {
    // PQL query to test
    std::string queryString(R"(
      read r1, r2; 
      Select r1
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::READ, "r1"));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("All types of declarations") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; read r; print p; while w; if ifs; assign a; 
      variable v; constant c; procedure pr; 
      Select pr
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::PROCEDURE, "pr"));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Follows") {
    // PQL query to test
    std::string queryString(R"(
      print p; while w;
      Select p such that Follows(p, w)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS, {
      Pql::Entity(Pql::EntityType::PRINT, "p"),
      Pql::Entity(Pql::EntityType::WHILE, "w")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Follows*") {
    // PQL query to test
    std::string queryString(R"(
      print p; while w;
      Select p such that Follows*(p, w)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::PRINT, "p"),
      Pql::Entity(Pql::EntityType::WHILE, "w")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Parent") {
    // PQL query to test
    std::string queryString(R"(
      print p; while w;
      Select p such that Parent(w, p)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::PRINT, "p")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Parent*") {
    // PQL query to test
    std::string queryString(R"(
      print p; while w;
      Select p such that Parent*(w, p)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::PRINT, "p")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Uses") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; variable v;
      Select v such that Uses(s, v)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
      Pql::Entity(Pql::EntityType::STMT, "s"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single such that clause - Modifies") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; variable v;
      Select s such that Modifies(s, v)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
      Pql::Entity(Pql::EntityType::STMT, "s"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Single pattern clause") {
    // PQL query to test
    std::string queryString(R"(
      variable v; assign a;
      Select a pattern a(v, _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "x")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Combination of such that and pattern clause") {
    // PQL query to test
    std::string queryString(R"(
      if ifs; assign a;
      Select ifs such that Modifies(ifs, "x") pattern a("x", _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::IF, "ifs"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "x")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE_NAME, "x"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "x")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Inconsistent spacing") {
    // PQL query to test
    std::string queryString(R"(
      stmt s       ; variable      
                  v   , v2
        ;assign a;
      Select s      such 
             that Follows* (    2,s)pattern a
      (v, _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::LINE_NUMBER, "2"),
      Pql::Entity(Pql::EntityType::STMT, "s")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, "x")
      }));

    REQUIRE(parsingResult == expectedResult);
  }
}

TEST_CASE("[TestPqlParser] Query with Syntax Error") {
  SECTION("Empty query string") {
    std::list<Token> emptyList;
    PqlParser pqlParser(emptyList);
    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Invalid keyword / design entity") {
    // PQL query to test
    std::string queryString(R"(
      hello h; Select h;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Extra tokens at the end of query string") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; Select s;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Missing semicolon after declaration") {
    // PQL query to test
    std::string queryString(R"(
      stmt s Select s;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Follows") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such that Follows *(1, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Parent") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such that Parent *(1, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern clause before such that clause") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; assign a;
      Select s1 pattern a(_, _"x"_) such that Follows*(1, s1)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one such that clause - 1") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; 
      Select s1 such that Follows*(1, s1) and Parent(s1, s2)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one such that clause - 2") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; 
      Select s1 such that Follows*(1, s1) such that Parent(s1, s2)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one pattern clause - 1") {
    // PQL query to test
    std::string queryString(R"(
      assign a1, a2; 
      Select a1 pattern a1(_, _"x"_) and a2("z", _"y"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one pattern clause - 2") {
    // PQL query to test
    std::string queryString(R"(
      assign a1, a2; 
      Select a1 pattern a1(_, _"x"_) pattern a2("z", _"y"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Wrong expression-spec") {
    // PQL query to test
    std::string queryString(R"(
      assign a1, a2; 
      Select a1 pattern a1(_, "x")
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Missing symbols") {
    // PQL query to test
    std::string queryString(R"(
      assign a1 a2; 
      Select a1
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }
}

TEST_CASE("[PqlParser] Query with Semantic Error") {
  SECTION("Repeated declarations") {
    // PQL query to test
    std::string queryString(R"(
      while s; assign s; 
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Selecting undeclared synonym") {
    // PQL query to test
    std::string queryString(R"(
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern does not use assign") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s pattern s(_, _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern LHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; while w;
      Select s pattern s(w, _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern RHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; while w;
      Select s pattern s(_, _";"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Follows LHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; procedure pr;
      Select s such that Follows(pr, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Follows RHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; variable v;
      Select s such that Follows(s, v)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Parent LHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; variable v;
      Select s such that Follows(v, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Parent RHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; procedure pr;
      Select s such that Follows(s, pr)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Uses LHS is invalid") {  // Invalid for Iteration 1
    // PQL query to test
    std::string queryString(R"(
      stmt s; procedure pr;
      Select s such that Uses(pr, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Uses RHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; procedure pr;
      Select s such that Uses(s, pr)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Modifies LHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; variable v;
      Select s such that Modifies(v, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Modifies RHS is invalid") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2
      Select s1 such that Modifies(s1, s2)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }
}

