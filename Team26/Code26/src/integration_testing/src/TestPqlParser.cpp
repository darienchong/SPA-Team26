#include "catch.hpp"

#include <iostream>
#include <list>
#include <string>
#include <sstream>

#include "PqlParser.h"
#include "PqlQuery.h"
#include "Token.h"
#include "Tokeniser.h"

namespace {
  std::list<Token> queryStringToTokens(std::string& queryString) {
    std::stringstream ss;
    ss << queryString << std::endl;
    return Tokeniser()
      .allowingLeadingZeroes()
      .notConsumingWhitespace()
      .tokenise(ss);
  }
}

TEST_CASE("[TestPqlParser] Valid Query", "[PqlParser][Tokeniser]") {
  SECTION("Declarations", "[declaration]") {
    SECTION("Single declaration, Single Select") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; 
        Select s
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Multiple declarations in single declaration statement") {
      // PQL query to test
      std::string queryString(R"(
        read r1, r2; 
        Select r1
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::READ, "r1"));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Multiple declarations of same type in multiple declaration statement") {
      // PQL query to test
      std::string queryString(R"(
        print p1; print p2; 
        Select p1
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p1"));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Multiple declarations of different types in multiple declaration statement") {
      // PQL query to test
      std::string queryString(R"(
        constant c; read r; 
        Select c
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::CONSTANT, "c"));

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
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PROCEDURE, "pr"));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Such that clause", "[suchthat-cl]") {
    SECTION("Follows(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          print p; while w;
          Select p such that Follows(p, w)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

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

    SECTION("Follows(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Follows(_, _)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Follows(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Follows(5, 6)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "5"),
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Follows*(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          print p; while w;
          Select p such that Follows*(p, w)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

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

    SECTION("Follows*(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Follows*(_, _)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Follows*(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Follows*(5, 6)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "5"),
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          print p; while w;
          Select p such that Parent(w, p)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

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

    SECTION("Parent(_, _)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Parent(_, _)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent(5, 6)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Parent(5, 6)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "5"),
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent*(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          print p; while w;
          Select p such that Parent*(w, p)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

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

    SECTION("Parent*(_, _)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Parent*(_, _)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent*(5, 6)") {
      // PQL query to test
      std::string queryString(R"(
          print p;
          Select p such that Parent*(5, 6)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "5"),
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Uses(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          stmt s; variable v;
          Select v such that Uses(s, v)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

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

    SECTION("Uses(stmtNum, variableName)") {
      // PQL query to test
      std::string queryString(R"(
          stmt s;
          Select s such that Uses(7, "x")
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "7"),
        Pql::Entity(Pql::EntityType::VARIABLE_NAME, "x")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Uses(_, variableName) - For Iteration 1 only") {  // Valid for Iteration 1
      // PQL query to test
      std::string queryString(R"(
          stmt s;
          Select s such that Uses(_, "x")
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::VARIABLE_NAME, "x")
        }));

      REQUIRE(parsingResult == expectedResult);
    }
    SECTION("Modifies(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
          stmt s; variable v;
          Select v such that Modifies(s, v)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
        Pql::Entity(Pql::EntityType::STMT, "s"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Modifies(stmtNum, variableName)") {
      // PQL query to test
      std::string queryString(R"(
          stmt s;
          Select s such that Modifies(7, "x")
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
        Pql::Entity(Pql::EntityType::STMT_NUMBER, "7"),
        Pql::Entity(Pql::EntityType::VARIABLE_NAME, "x")
        }));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Pattern clause", "[pattern-cl]") {
    SECTION("pattern(_, _)") {
      // PQL query to test
      std::string queryString(R"(
          assign a;
          Select a pattern a(_, _)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("pattern(synonym, subExpr)") {
      // PQL query to test
      std::string queryString(R"(
          variable v; assign a;
          Select a pattern a(v, _"x + y * z"_)
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v"),
        Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x y z * + ")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("pattern(varName, expr)") {
      // PQL query to test
      std::string queryString(R"(
          assign a;
          Select a pattern a("z", "x + y * z")
        )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.setTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::VARIABLE_NAME, "z"),
        Pql::Entity(Pql::EntityType::EXPRESSION, " x y z * + ")
        }));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Combination of such that and pattern clause", "[suchthat-cl][pattern-cl]") {
    // PQL query to test
    std::string queryString(R"(
        if ifs; assign a;
        Select ifs such that Modifies(ifs, "x") pattern a("x", _"x"_)
      )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

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
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x ")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Irregular spacing") {
    // PQL query to test
    std::string queryString;
    queryString
      .append("stmt s   \v    ; variable    \n")
      .append("  \f   v   , v2\n")
      .append(" ;assign a;\n")
      .append("  Select s   \t   such that\n")
      .append("    \t    \f   Follows* (   \v 2,s)pattern a\r\n")
      .append("  \r  (v, _\"x\"_)");

    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.setTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::STMT_NUMBER, "2"),
      Pql::Entity(Pql::EntityType::STMT, "s")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x ")
      }));

    REQUIRE(parsingResult == expectedResult);
  }
}


TEST_CASE("[TestPqlParser] Query with Syntax Error", "[PqlParser][Tokeniser]") {
  SECTION("Empty query string") {
    std::list<Token> emptyList;
    Pql::PqlParser pqlParser(emptyList);
    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Invalid keyword / design entity") {
    // PQL query to test
    std::string queryString(R"(
      hello h; Select h;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Missing symbols") {
    // PQL query to test
    std::string queryString(R"(
      assign a1 a2; 
      Select a1
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Extra tokens at the end of query string") {
    // PQL query to test
    std::string queryString(R"(
      stmt s; Select s;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Missing semicolon after declaration") {
    // PQL query to test
    std::string queryString(R"(
      stmt s Select s;
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("'such' and 'that' separated by more than a space") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such  that Follows(1, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Follows") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such that Follows *(1, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Parent") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such that Parent *(1, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern clause before such that clause") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; assign a;
      Select s1 pattern a(_, _"x"_) such that Follows*(1, s1)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one such that clause - using 'and'") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; 
      Select s1 such that Follows*(1, s1) and Parent(s1, s2)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one such that clause - using 'such that'") {
    // PQL query to test
    std::string queryString(R"(
      stmt s1, s2; 
      Select s1 such that Follows*(1, s1) such that Parent(s1, s2)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one pattern clause - using 'and'") {
    // PQL query to test
    std::string queryString(R"(
      assign a1, a2; 
      Select a1 pattern a1(_, _"x"_) and a2("z", _"y"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("More than one pattern clause - using 'pattern'") {
    // PQL query to test
    std::string queryString(R"(
      assign a1, a2; 
      Select a1 pattern a1(_, _"x"_) pattern a2("z", _"y"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Wrong expression-spec for Pattern clause") {
    // PQL query to test
    std::string queryString(R"(
        stmt s; while w;
        Select s pattern s(_, _";"_)
      )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }
}

TEST_CASE("[PqlParser] Query with Semantic Error") {
  SECTION("Repeated declarations of same name - same types") {
    // PQL query to test
    std::string queryString(R"(
      while s; while s; 
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Repeated declarations of same name - different types") {
    // PQL query to test
    std::string queryString(R"(
      while s; assign s; 
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Using undeclared synonym - in Select") {
    // PQL query to test
    std::string queryString(R"(
      Select s
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Using undeclared synonym - in Select") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s such that Follows(a, s)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern does not use assign") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s pattern s(_, _"x"_)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Illegal arguments") {
    SECTION("Pattern LHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; while w;
        Select s pattern s(w, _"x"_)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Follows LHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; procedure pr;
        Select s such that Follows(pr, s)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Follows RHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; variable v;
        Select s such that Follows(s, v)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Parent LHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; variable v;
        Select s such that Follows(v, s)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Parent RHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; procedure pr;
        Select s such that Follows(s, pr)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Uses LHS - For Iteration 1 Only") {  // Invalid for Iteration 1
      // PQL query to test
      std::string queryString(R"(
        stmt s; procedure pr;
        Select s such that Uses(pr, s)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Uses RHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; procedure pr;
        Select s such that Uses(s, pr)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Modifies LHS") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; variable v;
        Select s such that Modifies(v, s)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Modifies RHS is invalid") {
      // PQL query to test
      std::string queryString(R"(
        stmt s1, s2
        Select s1 such that Modifies(s1, s2)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }
  }

}
