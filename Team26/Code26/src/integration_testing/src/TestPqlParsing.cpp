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

TEST_CASE("[TestPqlParsing] Valid Query", "[PqlParser][Valid]") {
  SECTION("Declarations", "[declaration]") {
    SECTION("Single declaration") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s"));

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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::READ, "r1"));

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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p1"));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("All types of declarations") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; read r; print p; while w; if ifs; assign a; 
        variable v; constant c; procedure pr; call cl; prog_line n;
        Select pr
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PROCEDURE, "pr"));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Declaration of synonym using keywords") {
      // PQL query to test
      std::string queryString(R"(
        stmt Select, such, that, pattern, with, Follows, Parent, Uses, Modifies, 
        Next, Calls, Affects, stmt, read, print, while, if, assign, variable, 
        constant, procedure, call, prog, line, and, procName, varName, value;
        Select Select
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "Select"));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Select", "[select]") {
    SECTION("Single BOOLEAN select") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Single attribute reference select") {
      // PQL query to test
      std::string queryString(R"(
        stmt s;
        Select s.stmt#
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s", Pql::AttributeRefType::STMT_NUMBER));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Tuple select") {
      // PQL query to test
      std::string queryString(R"(
        stmt s; read r; print p; while w; if ifs; assign a; 
        variable v; constant c; procedure pr; call cl; prog_line n;
        Select <s, r, p, w, ifs, a, v, c, pr, cl, n, s.stmt#, r.stmt#, 
        p.stmt#, w.stmt#, ifs.stmt#, a.stmt#, cl.stmt#, c.value, 
        pr.procName, cl.procName, v.varName, r.varName, p.varName>
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::READ, "r"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::WHILE, "w"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::IF, "ifs"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::CONSTANT, "c"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PROCEDURE, "pr"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::CALL, "cl"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PROG_LINE, "n"));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::READ, "r", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::WHILE, "w", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::IF, "ifs", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::STMT_NUMBER));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::CONSTANT, "c", Pql::AttributeRefType::VALUE));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PROCEDURE, "pr", Pql::AttributeRefType::PROC_NAME));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::PROC_NAME));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v", Pql::AttributeRefType::VAR_NAME));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::READ, "r", Pql::AttributeRefType::VAR_NAME));
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p", Pql::AttributeRefType::VAR_NAME));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Single Such that clause", "[suchthat-cl]") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS, {
        Pql::Entity(Pql::EntityType::NUMBER, "5"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
        Pql::Entity(Pql::EntityType::NUMBER, "5"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::PRINT, "p")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent(wildcard, wildcard)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent(stmtNum, stmtNum)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
        Pql::Entity(Pql::EntityType::NUMBER, "5"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::PRINT, "p")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent*(wildcard, wildcard)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Parent*(stmtNum, stmtNum)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::PRINT, "p"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT_T, {
        Pql::Entity(Pql::EntityType::NUMBER, "5"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesS(synonym, synonym)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::STMT, "s"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesS(stmtNum, variableName)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::NUMBER, "7"),
        Pql::Entity(Pql::EntityType::NAME, "x")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesS(synonym, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        prog_line n;
        Select BOOLEAN such that Uses(n, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_S, {
        Pql::Entity(Pql::EntityType::PROG_LINE, "n"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesP(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr; variable v;
        Select BOOLEAN such that Uses(pr, v)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_P, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesP(procName, variableName)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Uses("proc1", "var1")
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_P, {
        Pql::Entity(Pql::EntityType::NAME, "proc1"),
        Pql::Entity(Pql::EntityType::NAME, "var1")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("UsesP(synonym, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr;
        Select BOOLEAN such that Uses(pr, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_P, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesS(synonym, synonym)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
        Pql::Entity(Pql::EntityType::STMT, "s"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesS(stmtNum, variableName)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
        Pql::Entity(Pql::EntityType::NUMBER, "7"),
        Pql::Entity(Pql::EntityType::NAME, "x")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesS(synonym, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        prog_line n;
        Select BOOLEAN such that Modifies(n, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
        Pql::Entity(Pql::EntityType::PROG_LINE, "n"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesP(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr; variable v;
        Select BOOLEAN such that Modifies(pr, v)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_P, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesP(procName, variableName)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Modifies("proc1", "var1")
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_P, {
        Pql::Entity(Pql::EntityType::NAME, "proc1"),
        Pql::Entity(Pql::EntityType::NAME, "var1")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("ModifiesP(synonym, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr;
        Select BOOLEAN such that Modifies(pr, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_P, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr1, pr2;
        Select BOOLEAN such that Calls(pr1, pr2)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr1"),
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Calls(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls(procName, procName)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Calls("proc1", "proc2")
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS, {
        Pql::Entity(Pql::EntityType::NAME, "proc1"),
        Pql::Entity(Pql::EntityType::NAME, "proc2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls*(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr1, pr2;
        Select BOOLEAN such that Calls*(pr1, pr2)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS_T, {
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr1"),
        Pql::Entity(Pql::EntityType::PROCEDURE, "pr2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls*(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Calls*(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Calls*(procName, procName)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Calls*("proc1", "proc2")
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS_T, {
        Pql::Entity(Pql::EntityType::NAME, "proc1"),
        Pql::Entity(Pql::EntityType::NAME, "proc2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        print p; call cl;
        Select BOOLEAN such that Next(p, cl)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT, {
        Pql::Entity(Pql::EntityType::PRINT, "p"),
        Pql::Entity(Pql::EntityType::CALL, "cl")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Next(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Next(10, 6)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT, {
        Pql::Entity(Pql::EntityType::NUMBER, "10"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next*(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        print p; call cl;
        Select BOOLEAN such that Next*(p, cl)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT_T, {
        Pql::Entity(Pql::EntityType::PRINT, "p"),
        Pql::Entity(Pql::EntityType::CALL, "cl")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next*(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Next*(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Next*(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Next*(10, 6)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT_T, {
        Pql::Entity(Pql::EntityType::NUMBER, "10"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        assign a1, a2;
        Select BOOLEAN such that Affects(a1, a2)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
        Pql::Entity(Pql::EntityType::ASSIGN, "a2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Affects(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Affects(10, 6)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS, {
        Pql::Entity(Pql::EntityType::NUMBER, "10"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects*(synonym, synonym)") {
      // PQL query to test
      std::string queryString(R"(
        assign a1, a2;
        Select BOOLEAN such that Affects*(a1, a2)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS_T, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a1"),
        Pql::Entity(Pql::EntityType::ASSIGN, "a2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects*(wildcard, wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Affects*(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS_T, {
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("Affects*(stmtNum, stmtNum)") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Affects*(10, 6)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS_T, {
        Pql::Entity(Pql::EntityType::NUMBER, "10"),
        Pql::Entity(Pql::EntityType::NUMBER, "6")
        }));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Single Pattern clause", "[pattern-cl]") {
    SECTION("assign (wildcard, wildcard)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("assign (synonym, subExpr)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v"),
        Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x y z * + ")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("assign (varName, expr)") {
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
      expectedResult.addTarget(Pql::Entity(Pql::EntityType::ASSIGN, "a"));
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
        Pql::Entity(Pql::EntityType::ASSIGN, "a"),
        Pql::Entity(Pql::EntityType::NAME, "z"),
        Pql::Entity(Pql::EntityType::EXPRESSION, " x y z * + ")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("if (wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        if ifs;
        Select BOOLEAN pattern ifs(_, _, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_IF, {
        Pql::Entity(Pql::EntityType::IF, "ifs"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("if (synonym)") {
      // PQL query to test
      std::string queryString(R"(
        if ifs; variable v;
        Select BOOLEAN pattern ifs(v, _, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_IF, {
        Pql::Entity(Pql::EntityType::IF, "ifs"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("if (varName)") {
      // PQL query to test
      std::string queryString(R"(
        if ifs;
        Select BOOLEAN pattern ifs("var2", _, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_IF, {
        Pql::Entity(Pql::EntityType::IF, "ifs"),
        Pql::Entity(Pql::EntityType::NAME, "var2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("while (wildcard)") {
      // PQL query to test
      std::string queryString(R"(
        while w;
        Select BOOLEAN pattern w(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_WHILE, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::WILDCARD, "_")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("while (synonym)") {
      // PQL query to test
      std::string queryString(R"(
        while w; variable v;
        Select BOOLEAN pattern w(v, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_WHILE, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::VARIABLE, "v")
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("while (varName)") {
      // PQL query to test
      std::string queryString(R"(
        while w;
        Select BOOLEAN pattern w("var2", _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_WHILE, {
        Pql::Entity(Pql::EntityType::WHILE, "w"),
        Pql::Entity(Pql::EntityType::NAME, "var2")
        }));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Single With clause", "[with-cl]") {
    SECTION("number = number") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN with 10 = 11
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::NUMBER, "10"),
        Pql::Entity(Pql::EntityType::NUMBER, "11"),
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("name = name") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN with "name1" = "name2"
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::NAME, "name1"),
        Pql::Entity(Pql::EntityType::NAME, "name2"),
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("attrRef = attrRef (number)") {
      // PQL query to test
      std::string queryString(R"(
        constant c; print p;
        Select BOOLEAN with c.value = p.stmt#
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::CONSTANT, "c", Pql::AttributeRefType::VALUE),
        Pql::Entity(Pql::EntityType::PRINT, "p", Pql::AttributeRefType::STMT_NUMBER),
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("attrRef = attrRef (name)") {
      // PQL query to test
      std::string queryString(R"(
        call cl; print p;
        Select BOOLEAN with cl.procName = p.varName
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::PROC_NAME),
        Pql::Entity(Pql::EntityType::PRINT, "p", Pql::AttributeRefType::VAR_NAME),
        }));

      REQUIRE(parsingResult == expectedResult);
    }

    SECTION("synonym = synonym (prog_line only)") {
      // PQL query to test
      std::string queryString(R"(
        prog_line n1, n2;
        Select BOOLEAN with n1 = n2
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      // Get parsing result
      Pql::Query parsingResult = pqlParser.parseQuery();

      // Build expected result
      Pql::Query expectedResult;
      expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
        Pql::Entity(Pql::EntityType::PROG_LINE, "n1"),
        Pql::Entity(Pql::EntityType::PROG_LINE, "n2"),
        }));

      REQUIRE(parsingResult == expectedResult);
    }
  }

  SECTION("Multi-clauses", "[suchthat-cl][pattern-cl][with-cl]") {
    // PQL query to test
    std::string queryString(R"(
        if ifs; assign a; procedure pr; while w; variable v; call cl; prog_line n; constant c;
        Select BOOLEAN 
        such that Modifies(ifs, "x") and Uses(pr, "x")
        pattern a("x", _"x + 1"_) and ifs(v, _, _) and w("y", _)
        pattern a(v, _)
        with pr.procName = "proc1" and n = c.value
        with cl.stmt# = n
        such that Next*(ifs, 10) and Affects(a, a) and Calls*(pr, "proc3") and Follows*(1, n)
        with pr.procName = v.varName and "proc2" = cl.procName
        such that Parent(ifs, a)
      )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::MODIFIES_S, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::NAME, "x")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_P, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
      Pql::Entity(Pql::EntityType::NAME, "x")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::NAME, "x"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x 1 + ")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_IF, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_WHILE, {
      Pql::Entity(Pql::EntityType::WHILE, "w"),
      Pql::Entity(Pql::EntityType::NAME, "y")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "pr", Pql::AttributeRefType::PROC_NAME),
      Pql::Entity(Pql::EntityType::NAME, "proc1")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::PROG_LINE, "n"),
      Pql::Entity(Pql::EntityType::CONSTANT, "c", Pql::AttributeRefType::VALUE)
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::STMT_NUMBER),
      Pql::Entity(Pql::EntityType::PROG_LINE, "n")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::NEXT_T, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::NUMBER, "10")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::AFFECTS, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::ASSIGN, "a")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::CALLS_T, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "pr"),
      Pql::Entity(Pql::EntityType::NAME, "proc3")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::NUMBER, "1"),
      Pql::Entity(Pql::EntityType::PROG_LINE, "n")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::PROCEDURE, "pr", Pql::AttributeRefType::PROC_NAME),
      Pql::Entity(Pql::EntityType::VARIABLE, "v", Pql::AttributeRefType::VAR_NAME)
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::WITH, {
      Pql::Entity(Pql::EntityType::NAME, "proc2"),
      Pql::Entity(Pql::EntityType::CALL, "cl", Pql::AttributeRefType::PROC_NAME)
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PARENT, {
      Pql::Entity(Pql::EntityType::IF, "ifs"),
      Pql::Entity(Pql::EntityType::ASSIGN, "a")
      }));

    REQUIRE(parsingResult == expectedResult);
  }

  SECTION("Irregular spacing") {
    // PQL query to test
    std::string queryString;
    queryString
      .append("stmt s    \v    ; variable    \n")
      .append("  \f   v   , v2\n")
      .append(" ;assign a;\n")
      .append("  Select < s   .   \t stmt#, v \f. varName\v>  such that\n")
      .append("    \t    \f   Follows* (   \v 2,s)pattern a\r\n")
      .append("  \r  (v, _\"          x +    1\"_) such that Uses(\" test      \v \" , _)     ");

    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    // Get parsing result
    Pql::Query parsingResult = pqlParser.parseQuery();

    // Build expected result
    Pql::Query expectedResult;
    expectedResult.addTarget(Pql::Entity(Pql::EntityType::STMT, "s", Pql::AttributeRefType::STMT_NUMBER));
    expectedResult.addTarget(Pql::Entity(Pql::EntityType::VARIABLE, "v", Pql::AttributeRefType::VAR_NAME));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::FOLLOWS_T, {
      Pql::Entity(Pql::EntityType::NUMBER, "2"),
      Pql::Entity(Pql::EntityType::STMT, "s")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::PATTERN_ASSIGN, {
      Pql::Entity(Pql::EntityType::ASSIGN, "a"),
      Pql::Entity(Pql::EntityType::VARIABLE, "v"),
      Pql::Entity(Pql::EntityType::SUB_EXPRESSION, " x 1 + ")
      }));
    expectedResult.addClause(Pql::Clause(Pql::ClauseType::USES_P, {
      Pql::Entity(Pql::EntityType::NAME, "test"),
      Pql::Entity(Pql::EntityType::WILDCARD, "_")
      }));

    REQUIRE(parsingResult == expectedResult);
  }
}

TEST_CASE("[TestPqlParsing] Query with Syntax Error", "[PqlParser][SyntaxError]") {
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
      Select BOOLEAN such that Follows *(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Parent") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Parent *(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Calls") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Calls *(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Next") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Next *(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Transitive relation has space between the relation and * - Affects") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Affects *(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Space between stmt and # for 'stmt#' attrRef") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select s.stmt #
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Empty varRef") {
    // PQL query to test
    std::string queryString(R"(
      assign a;
      Select BOOLEAN pattern a("", _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Empty procRef") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Uses("", _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Invalid expression-spec for Pattern clause") {
    // PQL query to test
    std::string queryString(R"(
        stmt s; while w;
        Select s pattern s(_, _"1+"_)
      )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }
}

TEST_CASE("[TestPqlParsing] Query with Semantic Error", "[PqlParser][Semantic Error]") {
  SECTION("Declaration using synonym named BOOLEAN") {
    // PQL query to test
    std::string queryString(R"(
      stmt BOOLEAN; 
      Select BOOLEAN
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

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

  SECTION("Using undeclared synonym - in Clause") {
    // PQL query to test
    std::string queryString(R"(
      Select BOOLEAN such that Follows(a, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Pattern does not use assign OR while OR if") {
    // PQL query to test
    std::string queryString(R"(
      stmt s;
      Select BOOLEAN pattern s(_, _)
    )");
    std::list<Token> queryTokens = queryStringToTokens(queryString);
    Pql::PqlParser pqlParser(queryTokens);

    REQUIRE_THROWS(pqlParser.parseQuery());
  }

  SECTION("Illegal arguments") {
    SECTION("Non-varRef synonym - procedure") {
      // PQL query to test
      std::string queryString(R"(
        assign a; procedure pr;
        Select BOOLEAN pattern a(pr, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-varRef synonym - stmt") {
      // PQL query to test
      std::string queryString(R"(
        while w; stmt s;
        Select BOOLEAN pattern w(s, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-varRef synonym - constant") {
      // PQL query to test
      std::string queryString(R"(
        if ifs; constant c;
        Select BOOLEAN pattern ifs(c, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Zero for stmtRef") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Follows(0, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-stmtRef synonym - constant") {
      // PQL query to test
      std::string queryString(R"(
        constant c;
        Select BOOLEAN such that Follows(_, c)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-stmtRef synonym - variable") {
      // PQL query to test
      std::string queryString(R"(
        variable v;
        Select BOOLEAN such that Follows(v, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-stmtRef synonym - procedure") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr;
        Select BOOLEAN such that Follows(_, pr)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-procRef synonym - variable") {
      // PQL query to test
      std::string queryString(R"(
        variable v;
        Select BOOLEAN such that Modifies(v, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-procRef synonym - constant") {
      // PQL query to test
      std::string queryString(R"(
        constant c;
        Select BOOLEAN such that Uses(c, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Non-procRef synonym - stmt") {
      // PQL query to test
      std::string queryString(R"(
        stmt s;
        Select BOOLEAN such that Calls(s, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Wildcard for Uses LHS") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Uses(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("Wildcard for Modifies LHS") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN such that Modifies(_, _)
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }
  }

  SECTION("With clause params of different types") {
    SECTION("number = name") {
      // PQL query to test
      std::string queryString(R"(
        Select BOOLEAN with 10 = "x"
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("number = name (attrRef)") {
      // PQL query to test
      std::string queryString(R"(
        procedure pr;
        Select BOOLEAN with 10 = pr.procName
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("number (attrRef) = name") {
      // PQL query to test
      std::string queryString(R"(
        constant c;
        Select BOOLEAN with c.value = "x"
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }

    SECTION("number (attrRef) = name (attrRef)") {
      // PQL query to test
      std::string queryString(R"(
        constant c; variable v;
        Select BOOLEAN with c.value = v.varName
      )");
      std::list<Token> queryTokens = queryStringToTokens(queryString);
      Pql::PqlParser pqlParser(queryTokens);

      REQUIRE_THROWS(pqlParser.parseQuery());
    }
  }
}
