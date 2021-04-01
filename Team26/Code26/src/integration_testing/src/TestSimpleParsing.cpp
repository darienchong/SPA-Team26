#include "catch.hpp"

#include <iostream>
#include <string>
#include <list>
#include <sstream>

#include "DesignExtractor.h"
#include "Pkb.h"
#include "SimpleParser.h"
#include "Table.h"
#include "Tokeniser.h"
#include "Token.h"
#include "CfgBip.h"


/*
* Integration testing of SimpleParser with Tokeniser, ExprParser, Pkb and De
*/

namespace {
  std::list<Token> expressionStringToTokens(std::string& string) {
    std::stringstream ss;
    ss << string << std::endl;
    return Tokeniser()
      .notAllowingLeadingZeroes()
      .consumingWhitespace()
      .tokenise(ss);
  }
}

///////////////
// Procedure //
///////////////

TEST_CASE("[TestSimpleParser] Procedures with semantic errors", "[SimpleParser][Procedure][Semantic Error]") {
  SECTION("Duplicated procedure name") {
    std::string string("procedure proc1 {x=1;} procedure proc1 {x=2;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Call to non-existing procedure") {
    std::string string("procedure proc1 {call proc2;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();
    REQUIRE_THROWS(SourceProcessor::DesignExtractor(pkb).extractDesignAbstractions());
  }

  SECTION("Recursive call of procedure") {
    std::string string("procedure proc1 {call proc1;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();
    REQUIRE_THROWS(SourceProcessor::DesignExtractor(pkb).extractDesignAbstractions());
  }

  SECTION("Cyclic call of procedure") {
    std::string string("procedure proc1 {call proc2;} procedure proc2 {call proc1;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();
    REQUIRE_THROWS(SourceProcessor::DesignExtractor(pkb).extractDesignAbstractions());
  }
}

TEST_CASE("[TestSimpleParser] Invalid parsing procedure", "[SimpleParser][Procedure][Syntax Error]") {
  SECTION("Missing procedure keyword") {
    std::string string("computeAverage {}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Misspell procedure keyword") {
    std::string string("proc computeAverage {}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Capitalised procedure keyword") {
    std::string string("proceduRe computeAverage {}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Procedure with empty statement list") {
    std::string string("procedure computeAverage {}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Procedure with extra tokens after '}'") {
    std::string string("procedure computeAverage {x=1;} ;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Multiple procedures - independent", "[SimpleParser][Procedures]") {
  std::string string("procedure computeAverage {x=1;} procedure b{x=2;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table callProcTable = pkb.getCallProcTable();
  Table procTable = pkb.getProcTable();
  Table modifiesPTable = pkb.getModifiesPTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table usesPTable = pkb.getUsesPTable();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(callProcTable.size() == 0);
  REQUIRE(procTable.contains({ "computeAverage" }));
  REQUIRE(procTable.contains({ "b" }));
  REQUIRE(procTable.size() == 2);
  REQUIRE(modifiesPTable.contains({ "computeAverage", "x" }));
  REQUIRE(modifiesPTable.contains({ "b", "x" }));
  REQUIRE(modifiesPTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ "1", "x" }));
  REQUIRE(modifiesSTable.contains({ "2", "x" }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(usesPTable.size() == 0);
  REQUIRE(usesSTable.size() == 0);

  SECTION("Transitive") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table modifiesPTable = pkb.getModifiesPTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table usesPTable = pkb.getUsesPTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(modifiesPTable.contains({ "computeAverage", "x" }));
    REQUIRE(modifiesPTable.contains({ "b", "x" }));
    REQUIRE(modifiesPTable.size() == 2);
    REQUIRE(modifiesSTable.size() == 2);
    REQUIRE(usesPTable.size() == 0);
    REQUIRE(usesSTable.size() == 0);
  }
}

TEST_CASE("[TestSimpleParser] Multiple procedures - Direct calls", "[SimpleParser][Procedures]") {
  std::string string("procedure a{x=1;call b;call c;} procedure b{y=2;} procedure c{print z;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table callProcTable = pkb.getCallProcTable();
  Table callsTable = pkb.getCallsTable();
  Table procTable = pkb.getProcTable();
  Table modifiesPTable = pkb.getModifiesPTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table usesPTable = pkb.getUsesPTable();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(callProcTable.contains({ "2", "b" }));
  REQUIRE(callProcTable.contains({ "3", "c" }));
  REQUIRE(callProcTable.size() == 2);
  REQUIRE(callsTable.contains({ "a", "b" }));
  REQUIRE(callsTable.contains({ "a", "c" }));
  REQUIRE(callsTable.size() == 2);
  REQUIRE(procTable.contains({ "a" }));
  REQUIRE(procTable.contains({ "b" }));
  REQUIRE(procTable.contains({ "c" }));
  REQUIRE(procTable.size() == 3);
  REQUIRE(modifiesPTable.contains({ "a", "x" }));
  REQUIRE(modifiesPTable.contains({ "b", "y" }));
  REQUIRE(modifiesPTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ "1", "x" }));
  REQUIRE(modifiesSTable.contains({ "4", "y" }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(usesPTable.contains({ "c", "z" }));
  REQUIRE(usesPTable.size() == 1);
  REQUIRE(usesSTable.contains({ "5", "z" }));
  REQUIRE(usesSTable.size() == 1);

  SECTION("Transitive") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table callsTTable = pkb.getCallsTTable();
    Table modifiesPTable = pkb.getModifiesPTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table usesPTable = pkb.getUsesPTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(callsTTable.size() == 2);
    REQUIRE(modifiesPTable.contains({ "a", "y" }));
    REQUIRE(modifiesPTable.size() == 3);
    REQUIRE(modifiesSTable.contains({ "2", "y" }));
    REQUIRE(modifiesSTable.size() == 3);
    REQUIRE(usesPTable.contains({ "a", "z" }));
    REQUIRE(usesPTable.size() == 2);
    REQUIRE(usesSTable.contains({ "3", "z" }));
    REQUIRE(usesSTable.size() == 2);
  }
}

TEST_CASE("[TestSimpleParser] Multiple procedures - Indirect calls", "[SimpleParser][Procedures]") {
  std::string string("procedure a{x=1;call b;} procedure b{y=v;call c;} procedure c{w=z;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table callProcTable = pkb.getCallProcTable();
  Table callsTable = pkb.getCallsTable();
  Table procTable = pkb.getProcTable();
  Table modifiesPTable = pkb.getModifiesPTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table usesPTable = pkb.getUsesPTable();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(callProcTable.contains({ "2", "b" }));
  REQUIRE(callProcTable.contains({ "4", "c" }));
  REQUIRE(callProcTable.size() == 2);
  REQUIRE(callsTable.contains({ "a", "b" }));
  REQUIRE(callsTable.contains({ "b", "c" }));
  REQUIRE(callsTable.size() == 2);
  REQUIRE(procTable.contains({ "a" }));
  REQUIRE(procTable.contains({ "b" }));
  REQUIRE(procTable.contains({ "c" }));
  REQUIRE(procTable.size() == 3);
  REQUIRE(modifiesPTable.contains({ "a", "x" }));
  REQUIRE(modifiesPTable.contains({ "b", "y" }));
  REQUIRE(modifiesPTable.contains({ "c", "w" }));
  REQUIRE(modifiesPTable.size() == 3);
  REQUIRE(modifiesSTable.contains({ "1", "x" }));
  REQUIRE(modifiesSTable.contains({ "3", "y" }));
  REQUIRE(modifiesSTable.contains({ "5", "w" }));
  REQUIRE(modifiesSTable.size() == 3);
  REQUIRE(usesPTable.contains({ "b", "v" }));
  REQUIRE(usesPTable.contains({ "c", "z" }));
  REQUIRE(usesPTable.size() == 2);
  REQUIRE(usesSTable.contains({ "3", "v" }));
  REQUIRE(usesSTable.contains({ "5", "z" }));
  REQUIRE(usesSTable.size() == 2);

  SECTION("Transitive") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table callsTTable = pkb.getCallsTTable();
    Table modifiesPTable = pkb.getModifiesPTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table usesPTable = pkb.getUsesPTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(callsTTable.contains({ "a", "c" }));
    REQUIRE(callsTTable.size() == 3);
    REQUIRE(modifiesPTable.contains({ "a", "y" }));
    REQUIRE(modifiesPTable.contains({ "a", "w" }));
    REQUIRE(modifiesPTable.contains({ "b", "w" }));
    REQUIRE(modifiesPTable.size() == 6);
    REQUIRE(modifiesSTable.contains({ "2", "y" }));
    REQUIRE(modifiesSTable.contains({ "2", "w" }));
    REQUIRE(modifiesSTable.contains({ "4", "w" }));
    REQUIRE(modifiesSTable.size() == 6);
    REQUIRE(usesPTable.contains({ "a", "z" }));
    REQUIRE(usesPTable.contains({ "a", "v" }));
    REQUIRE(usesPTable.contains({ "b", "z" }));
    REQUIRE(usesPTable.size() == 5);
    REQUIRE(usesSTable.contains({ "2", "v" }));
    REQUIRE(usesSTable.contains({ "2", "z" }));
    REQUIRE(usesSTable.contains({ "4", "z" }));
    REQUIRE(usesSTable.size() == 5);
  }
}

////////////////////
// Read statement //
////////////////////

TEST_CASE("[TestSimpleParser] Invalid read statement", "[SimpleParser][Read][Syntax Error]") {
  SECTION("Missing read keyword") {
    std::string string("procedure computeAverage {x;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Misspell read keyword") {
    std::string string("procedure computeAverage {reaD x;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Read into a constant") {
    std::string string("procedure computeAverage {read 3;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Read without ending ;") {
    std::string string("procedure computeAverage {read varX}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Valid read statement", "[SimpleParser][Read]") {
  std::string string("procedure computeAverage {read s3s1kd03kd42d;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table stmtTable = pkb.getStmtTable();
  Table modifiesSTable = pkb.getModifiesSTable();

  REQUIRE(procTable.contains({ "computeAverage" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(readTable.contains({ "1" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "s3s1kd03kd42d" }));
  REQUIRE(varTable.size() == 1);
  REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
  REQUIRE(modifiesSTable.contains({ "1", "s3s1kd03kd42d" }));
  REQUIRE(modifiesSTable.size() == 1);
}

/////////////////////
// Print statement //
/////////////////////

TEST_CASE("[TestSimpleParser] Invalid print statement", "[SimpleParser][Print][Syntax Error]") {
  SECTION("Missing print keyword") {
    std::string string("procedure computeAverage {x;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Misspell print keyword") {
    std::string string("procedure computeAverage {PRINT x;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Print a constant") {
    std::string string("procedure computeAverage {print 3;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Print without ending ;") {
    std::string string("procedure computeAverage {print varX}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Valid print statement", "[SimpleParser][Print]") {
  std::string string("procedure computeAverage {print s3s1kd03kd42d;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table printTable = pkb.getPrintTable();
  Table varTable = pkb.getVarTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();

  REQUIRE(procTable.contains({ "computeAverage" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(printTable.contains({ "1" }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(varTable.contains({ "s3s1kd03kd42d" }));
  REQUIRE(varTable.size() == 1);
  REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "1", "s3s1kd03kd42d" }));
  REQUIRE(usesSTable.size() == 1);
}

//////////////////////
// Assign statement //
//////////////////////

TEST_CASE("[TestSimpleParser] Invalid assign statement - LHS constant", "[SimpleParser][Assign][Syntax Error]") {
  SECTION("Single constant - no =, no rhs") {
    std::string string("procedure computeAverage {3;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Expression - no =, no rhs") {
    std::string string("procedure computeAverage {3+2;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("=, no rhs") {
    std::string string("procedure computeAverage {3=;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("no =, rhs") {
    std::string string("procedure computeAverage {3 varx + 3;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("=, rhs") {
    std::string string("procedure computeAverage {3 = varx + 3;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Invalid assign statement - LHS variable", "[SimpleParser][Assign][Syntax Error]") {
  SECTION("Single variable - no =, no rhs") {
    std::string string("procedure computeAverage {varX;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Expression - no =, no rhs") {
    std::string string("procedure computeAverage {himum + hidad;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("=, no rhs") {
    std::string string("procedure computeAverage {cya=;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("no =, rhs") {
    std::string string("procedure computeAverage {happy cny;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Assign without ending ;") {
    std::string string("procedure computeAverage {varX = 3}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Valid assign statement - Basic", "[SimpleParser][Assign]") {
  SECTION("Single variable") {
    std::string string("procedure computeAverage {x= y;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();

    Table procTable = pkb.getProcTable();
    Table assignTable = pkb.getAssignTable();
    Table varTable = pkb.getVarTable();
    Table stmtTable = pkb.getStmtTable();
    Table usesSTable = pkb.getUsesSTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table patternAssignTable = pkb.getPatternAssignTable();

    std::string expectedPostFixString(" y ");

    REQUIRE(procTable.contains({ "computeAverage" }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ "1" }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ "x" }));
    REQUIRE(varTable.contains({ "y" }));
    REQUIRE(varTable.size() == 2);
    REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
    REQUIRE(usesSTable.contains({ "1", "y" }));
    REQUIRE(usesSTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ "1", "x", expectedPostFixString }));
    REQUIRE(patternAssignTable.size() == 1);
  }

  SECTION("Single constant") {
    std::string string("procedure computeAverage {x= 0;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();

    Table procTable = pkb.getProcTable();
    Table assignTable = pkb.getAssignTable();
    Table varTable = pkb.getVarTable();
    Table constTable = pkb.getConstTable();
    Table stmtTable = pkb.getStmtTable();
    Table usesSTable = pkb.getUsesSTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table patternAssignTable = pkb.getPatternAssignTable();

    std::string expectedPostFixString(" 0 ");

    REQUIRE(procTable.contains({ "computeAverage" }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ "1" }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ "x" }));
    REQUIRE(varTable.size() == 1);
    REQUIRE(constTable.contains({ "0" }));
    REQUIRE(constTable.size() == 1);
    REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ "1", "x", expectedPostFixString }));
    REQUIRE(patternAssignTable.size() == 1);
  }

  SECTION("Multiple assign") {
    std::string string("procedure computeAverage {x= 0; y=x;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();

    Table procTable = pkb.getProcTable();
    Table assignTable = pkb.getAssignTable();
    Table varTable = pkb.getVarTable();
    Table constTable = pkb.getConstTable();
    Table stmtTable = pkb.getStmtTable();
    Table usesSTable = pkb.getUsesSTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table followsTable = pkb.getFollowsTable();
    Table patternAssignTable = pkb.getPatternAssignTable();

    std::string expectedPostFixString1(" 0 ");
    std::string expectedPostFixString2(" x ");

    REQUIRE(procTable.contains({ "computeAverage" }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ "1" }));
    REQUIRE(assignTable.contains({ "2" }));
    REQUIRE(assignTable.size() == 2);
    REQUIRE(varTable.contains({ "x" }));
    REQUIRE(varTable.contains({ "y" }));
    REQUIRE(varTable.size() == 2);
    REQUIRE(constTable.contains({ "0" }));
    REQUIRE(constTable.size() == 1);
    REQUIRE(stmtTable.size() == 2); // maybe can also check the statement num? maybe not necessary
    REQUIRE(usesSTable.contains({ "2", "x" }));
    REQUIRE(usesSTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.contains({ "2", "y" }));
    REQUIRE(modifiesSTable.size() == 2);
    REQUIRE(followsTable.contains({ "1", "2" }));
    REQUIRE(followsTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ "1", "x", expectedPostFixString1 }));
    REQUIRE(patternAssignTable.contains({ "2", "y", expectedPostFixString2 }));
    REQUIRE(patternAssignTable.size() == 2);
  }
}

TEST_CASE("[TestSimpleParser] Valid assign statement - Advanced", "[SimpleParser][Assign]") {
  SECTION("Geekforgeeks example") {
    std::string string("procedure geeks{z=a + b * (c * d - e) * (f + g * h) - i;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();

    Table procTable = pkb.getProcTable();
    Table assignTable = pkb.getAssignTable();
    Table varTable = pkb.getVarTable();
    Table stmtTable = pkb.getStmtTable();
    Table usesSTable = pkb.getUsesSTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table patternAssignTable = pkb.getPatternAssignTable();

    std::string expectedPostFixString(" a b c d * e - * f g h * + * + i - ");

    REQUIRE(procTable.contains({ "geeks" }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ "1" }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ "a" }));
    REQUIRE(varTable.contains({ "b" }));
    REQUIRE(varTable.contains({ "c" }));
    REQUIRE(varTable.contains({ "d" }));
    REQUIRE(varTable.contains({ "e" }));
    REQUIRE(varTable.contains({ "f" }));
    REQUIRE(varTable.contains({ "g" }));
    REQUIRE(varTable.contains({ "h" }));
    REQUIRE(varTable.contains({ "i" }));
    REQUIRE(varTable.contains({ "z" }));
    REQUIRE(varTable.size() == 10);
    REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
    REQUIRE(usesSTable.contains({ "1", "a" }));
    REQUIRE(usesSTable.contains({ "1", "b" }));
    REQUIRE(usesSTable.contains({ "1", "c" }));
    REQUIRE(usesSTable.contains({ "1", "d" }));
    REQUIRE(usesSTable.contains({ "1", "e" }));
    REQUIRE(usesSTable.contains({ "1", "f" }));
    REQUIRE(usesSTable.contains({ "1", "g" }));
    REQUIRE(usesSTable.contains({ "1", "h" }));
    REQUIRE(usesSTable.contains({ "1", "i" }));
    REQUIRE(usesSTable.size() == 9);
    REQUIRE(modifiesSTable.contains({ "1", "z" }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ "1", "z", expectedPostFixString }));
    REQUIRE(patternAssignTable.size() == 1);
  }

  SECTION("Geeksforgeeks comment example") {
    std::string string("procedure f{c=96*9+var1340*4+2-76*(8-varY)*2/8*6+45/(1+varZ*1/4+(57/2/9*29+6)-v13dj3d0a3kd/4)+3-4+74*2;}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    parser.parse();

    Table procTable = pkb.getProcTable();
    Table assignTable = pkb.getAssignTable();
    Table varTable = pkb.getVarTable();
    Table constTable = pkb.getConstTable();
    Table stmtTable = pkb.getStmtTable();
    Table usesSTable = pkb.getUsesSTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table patternAssignTable = pkb.getPatternAssignTable();

    std::string expectedPostFixString(" 96 9 * var1340 4 * + 2 + 76 8 varY - * 2 * 8 / 6 * - 45 1 varZ 1 * 4 / + 57 2 / 9 / 29 * 6 + + v13dj3d0a3kd 4 / - / + 3 + 4 - 74 2 * + ");

    REQUIRE(procTable.contains({ "f" }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ "1" }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ "var1340" }));
    REQUIRE(varTable.contains({ "varY" }));
    REQUIRE(varTable.contains({ "varZ" }));
    REQUIRE(varTable.contains({ "v13dj3d0a3kd" }));
    REQUIRE(varTable.contains({ "c" }));
    REQUIRE(varTable.size() == 5);
    REQUIRE(constTable.contains({ "1" }));
    REQUIRE(constTable.contains({ "2" }));
    REQUIRE(constTable.contains({ "3" }));
    REQUIRE(constTable.contains({ "4" }));
    REQUIRE(constTable.contains({ "6" }));
    REQUIRE(constTable.contains({ "8" }));
    REQUIRE(constTable.contains({ "9" }));
    REQUIRE(constTable.contains({ "29" }));
    REQUIRE(constTable.contains({ "45" }));
    REQUIRE(constTable.contains({ "57" }));
    REQUIRE(constTable.contains({ "74" }));
    REQUIRE(constTable.contains({ "76" }));
    REQUIRE(constTable.contains({ "96" }));
    REQUIRE(constTable.size() == 13);
    REQUIRE(stmtTable.size() == 1); // maybe can also check the statement num? maybe not necessary
    REQUIRE(usesSTable.contains({ "1", "var1340" }));
    REQUIRE(usesSTable.contains({ "1", "varY" }));
    REQUIRE(usesSTable.contains({ "1", "varZ" }));
    REQUIRE(usesSTable.contains({ "1", "v13dj3d0a3kd" }));
    REQUIRE(usesSTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ "1", "c" }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ "1", "c", expectedPostFixString }));
    REQUIRE(patternAssignTable.size() == 1);
  }
}

//////////////////
// If statement //
//////////////////

TEST_CASE("[TestSimpleParser] Invalid if statement - Parenthesis", "[SimpleParser][If][Syntax Error]") {
  SECTION("No parenthesis with cond expr") {
    std::string string("procedure if{if while==3 then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("No parenthesis no cond expr") {
    std::string string("procedure if{if then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Mismatched parenthesis") {
    std::string string("procedure if{if (while==3 then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Invalid if statement - Missing from grammar/semantic", "[SimpleParser][If][Syntax Error]") {
  SECTION("No conditional expression") {
    std::string string("procedure if{if()then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("No 'then' keyword") {
    std::string string("procedure then{if(x==1){x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Empty 'then' statement list") {
    std::string string("procedure then{if(x==1)then{}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("No 'else' keyword") {
    std::string string("procedure then{if(x==1)then{x=1;}{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Empty 'else' statement list") {
    std::string string("procedure then{if(x==1)then{x=1;}else{}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Invalid if statement - Conditional expr", "[SimpleParser][If][Syntax Error]") {
  SECTION("Variable as conditional expr") {
    std::string string("procedure then{if(true)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Constant as conditional expr") {
    std::string string("procedure then{if(5)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Expr as conditional expr") {
    std::string string("procedure then{if(x=6)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Valid if statement - Basic", "[SimpleParser][If]") {
  std::string string("procedure then{if(then==else)then{x=1;}else{read y;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table assignTable = pkb.getAssignTable();
  Table ifTable = pkb.getIfTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString(" 1 ");

  REQUIRE(procTable.contains({ "then" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ "2" }));
  REQUIRE(assignTable.size() == 1);
  REQUIRE(ifTable.contains({ "1" }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(readTable.contains({ "3" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "x" }));
  REQUIRE(varTable.contains({ "y" }));
  REQUIRE(varTable.contains({ "then" }));
  REQUIRE(varTable.contains({ "else" }));
  REQUIRE(varTable.size() == 4);
  REQUIRE(constTable.contains({ "1" }));
  REQUIRE(constTable.size() == 1);
  REQUIRE(stmtTable.size() == 3); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "1", "then" }));
  REQUIRE(usesSTable.contains({ "1", "else" }));
  REQUIRE(usesSTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ "2", "x" }));
  REQUIRE(modifiesSTable.contains({ "3", "y" }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.contains({ "1", "3" }));
  REQUIRE(parentTable.size() == 2);
  REQUIRE(patternAssignTable.contains({ "2", "x", expectedPostFixString }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.contains({ "1", "y" }));
    REQUIRE(modifiesSTable.size() == 4);
    REQUIRE(parentTTable.size() == 2);
    REQUIRE(usesSTable.size() == 2);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.size() == 0);
      REQUIRE(affectsTTable.size() == 0);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "then", "x" }));
      REQUIRE(modifiesPTable.contains({ "then", "y" }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "1", "3" }));
      REQUIRE(nextTable.size() == 2);
      REQUIRE(nextTTable.contains({ "1", "2" }));
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.size() == 2);
      REQUIRE(patternIfTable.contains({ "1", "then" }));
      REQUIRE(patternIfTable.contains({ "1", "else" }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ "then", "then" }));
      REQUIRE(usesPTable.contains({ "then", "else" }));
      REQUIRE(usesPTable.size() == 2);
    }
  }
}

TEST_CASE("[TestSimpleParser] Valid if statement - Singly nested if", "[SimpleParser][If]") {
  std::string string("procedure then{if(then==else)then{if(else==while)then{print x;}else{read x;}}else{if(if==read)then{y=x;}else{x=y;}}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table assignTable = pkb.getAssignTable();
  Table ifTable = pkb.getIfTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString1(" x ");
  std::string expectedPostFixString2(" y ");

  REQUIRE(procTable.contains({ "then" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ "6" }));
  REQUIRE(assignTable.contains({ "7" }));
  REQUIRE(assignTable.size() == 2);
  REQUIRE(ifTable.contains({ "1" }));
  REQUIRE(ifTable.contains({ "2" }));
  REQUIRE(ifTable.contains({ "5" }));
  REQUIRE(ifTable.size() == 3);
  REQUIRE(printTable.contains({ "3" }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ "4" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "x" }));
  REQUIRE(varTable.contains({ "y" }));
  REQUIRE(varTable.contains({ "then" }));
  REQUIRE(varTable.contains({ "else" }));
  REQUIRE(varTable.contains({ "if" }));
  REQUIRE(varTable.contains({ "read" }));
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.size() == 7);
  REQUIRE(constTable.size() == 0);
  REQUIRE(stmtTable.size() == 7); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "1", "then" }));
  REQUIRE(usesSTable.contains({ "1", "else" }));
  REQUIRE(usesSTable.contains({ "2", "else" }));
  REQUIRE(usesSTable.contains({ "2", "while" }));
  REQUIRE(usesSTable.contains({ "3", "x" }));
  REQUIRE(usesSTable.contains({ "5", "if" }));
  REQUIRE(usesSTable.contains({ "5", "read" }));
  REQUIRE(usesSTable.contains({ "6", "x" }));
  REQUIRE(usesSTable.contains({ "7", "y" }));
  REQUIRE(usesSTable.size() == 9);
  REQUIRE(modifiesSTable.contains({ "4", "x" }));
  REQUIRE(modifiesSTable.contains({ "6", "y" }));
  REQUIRE(modifiesSTable.contains({ "7", "x" }));
  REQUIRE(modifiesSTable.size() == 3);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.contains({ "1", "5" }));
  REQUIRE(parentTable.contains({ "2", "3" }));
  REQUIRE(parentTable.contains({ "2", "4" }));
  REQUIRE(parentTable.contains({ "5", "6" }));
  REQUIRE(parentTable.contains({ "5", "7" }));
  REQUIRE(parentTable.size() == 6);
  REQUIRE(patternAssignTable.contains({ "6", "y", expectedPostFixString1 }));
  REQUIRE(patternAssignTable.contains({ "7", "x", expectedPostFixString2 }));
  REQUIRE(patternAssignTable.size() == 2);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.contains({ "1", "y" }));
    REQUIRE(modifiesSTable.contains({ "2", "x" }));
    REQUIRE(modifiesSTable.contains({ "5", "x" }));
    REQUIRE(modifiesSTable.contains({ "5", "y" }));
    REQUIRE(modifiesSTable.size() == 8);
    REQUIRE(parentTTable.contains({ "1", "3" }));
    REQUIRE(parentTTable.contains({ "1", "4" }));
    REQUIRE(parentTTable.contains({ "1", "6" }));
    REQUIRE(parentTTable.contains({ "1", "7" }));
    REQUIRE(parentTTable.size() == 10);
    REQUIRE(usesSTable.contains({ "1", "while" }));
    REQUIRE(usesSTable.contains({ "1", "x" }));
    REQUIRE(usesSTable.contains({ "1", "if" }));
    REQUIRE(usesSTable.contains({ "1", "read" }));
    REQUIRE(usesSTable.contains({ "1", "y" }));
    REQUIRE(usesSTable.contains({ "2", "x" }));
    REQUIRE(usesSTable.contains({ "5", "x" }));
    REQUIRE(usesSTable.contains({ "5", "y" }));
    REQUIRE(usesSTable.size() == 17);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.size() == 0);
      REQUIRE(affectsTTable.size() == 0);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "then", "x" }));
      REQUIRE(modifiesPTable.contains({ "then", "y" }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "1", "5" }));
      REQUIRE(nextTable.contains({ "2", "3" }));
      REQUIRE(nextTable.contains({ "2", "4" }));
      REQUIRE(nextTable.contains({ "5", "6" }));
      REQUIRE(nextTable.contains({ "5", "7" }));
      REQUIRE(nextTable.size() == 6);
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "6" }));
      REQUIRE(nextTTable.contains({ "1", "7" }));
      REQUIRE(nextTTable.size() == 10);
      REQUIRE(patternIfTable.contains({ "1", "then" }));
      REQUIRE(patternIfTable.contains({ "1", "else" }));
      REQUIRE(patternIfTable.contains({ "2", "else" }));
      REQUIRE(patternIfTable.contains({ "2", "while" }));
      REQUIRE(patternIfTable.contains({ "5", "if" }));
      REQUIRE(patternIfTable.contains({ "5", "read" }));
      REQUIRE(patternIfTable.size() == 6);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ "then", "then" }));
      REQUIRE(usesPTable.contains({ "then", "else" }));
      REQUIRE(usesPTable.contains({ "then", "while" }));
      REQUIRE(usesPTable.contains({ "then", "x" }));
      REQUIRE(usesPTable.contains({ "then", "y" }));
      REQUIRE(usesPTable.contains({ "then", "if" }));
      REQUIRE(usesPTable.contains({ "then", "read" }));
      REQUIRE(usesPTable.size() == 7);
    }
  }
}

TEST_CASE("[TestSimpleParser] Valid if statement - Doubly nested if", "[SimpleParser][If]") {
  std::string string("procedure a{if(2==3)then{x1=y1;}else{if(while==3)then{read x;print x;}else{z=3;if(b!=2)then{z=6;}else{c=f+g/5;read d;}}}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table assignTable = pkb.getAssignTable();
  Table ifTable = pkb.getIfTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString1(" y1 ");
  std::string expectedPostFixString2(" 3 ");
  std::string expectedPostFixString3(" 6 ");
  std::string expectedPostFixString4(" f g 5 / + ");

  REQUIRE(procTable.contains({ "a" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ "2" }));
  REQUIRE(assignTable.contains({ "6" }));
  REQUIRE(assignTable.contains({ "8" }));
  REQUIRE(assignTable.contains({ "9" }));
  REQUIRE(assignTable.size() == 4);
  REQUIRE(ifTable.contains({ "1" }));
  REQUIRE(ifTable.contains({ "3" }));
  REQUIRE(ifTable.contains({ "7" }));
  REQUIRE(ifTable.size() == 3);
  REQUIRE(printTable.contains({ "5" }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ "4" }));
  REQUIRE(readTable.contains({ "10" }));
  REQUIRE(readTable.size() == 2);
  REQUIRE(varTable.contains({ "x1" }));
  REQUIRE(varTable.contains({ "y1" }));
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.contains({ "x" }));
  REQUIRE(varTable.contains({ "z" }));
  REQUIRE(varTable.contains({ "b" }));
  REQUIRE(varTable.contains({ "c" }));
  REQUIRE(varTable.contains({ "f" }));
  REQUIRE(varTable.contains({ "g" }));
  REQUIRE(varTable.contains({ "d" }));
  REQUIRE(varTable.size() == 10);
  REQUIRE(constTable.contains({ "2" }));
  REQUIRE(constTable.contains({ "3" }));
  REQUIRE(constTable.contains({ "5" }));
  REQUIRE(constTable.contains({ "6" }));
  REQUIRE(constTable.size() == 4);
  REQUIRE(stmtTable.size() == 10); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "2", "y1" }));
  REQUIRE(usesSTable.contains({ "3", "while" }));
  REQUIRE(usesSTable.contains({ "5", "x" }));
  REQUIRE(usesSTable.contains({ "7", "b" }));
  REQUIRE(usesSTable.contains({ "9", "f" }));
  REQUIRE(usesSTable.contains({ "9", "g" }));
  REQUIRE(usesSTable.size() == 6);
  REQUIRE(modifiesSTable.contains({ "2", "x1" }));
  REQUIRE(modifiesSTable.contains({ "4", "x" }));
  REQUIRE(modifiesSTable.contains({ "6", "z" }));
  REQUIRE(modifiesSTable.contains({ "8", "z" }));
  REQUIRE(modifiesSTable.contains({ "9", "c" }));
  REQUIRE(modifiesSTable.contains({ "10", "d" }));
  REQUIRE(modifiesSTable.size() == 6);
  REQUIRE(followsTable.contains({ "4", "5" }));
  REQUIRE(followsTable.contains({ "6", "7" }));
  REQUIRE(followsTable.contains({ "9", "10" }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.contains({ "1", "3" }));
  REQUIRE(parentTable.contains({ "3", "4" }));
  REQUIRE(parentTable.contains({ "3", "5" }));
  REQUIRE(parentTable.contains({ "3", "6" }));
  REQUIRE(parentTable.contains({ "3", "7" }));
  REQUIRE(parentTable.contains({ "7", "8" }));
  REQUIRE(parentTable.contains({ "7", "9" }));
  REQUIRE(parentTable.contains({ "7", "10" }));
  REQUIRE(parentTable.size() == 9);
  REQUIRE(patternAssignTable.contains({ "2", "x1", expectedPostFixString1 }));
  REQUIRE(patternAssignTable.contains({ "6", "z", expectedPostFixString2 }));
  REQUIRE(patternAssignTable.contains({ "8", "z", expectedPostFixString3 }));
  REQUIRE(patternAssignTable.contains({ "9", "c", expectedPostFixString4 }));
  REQUIRE(patternAssignTable.size() == 4);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 3);
    REQUIRE(modifiesSTable.contains({ "1", "x1" }));
    REQUIRE(modifiesSTable.contains({ "1", "x" }));
    REQUIRE(modifiesSTable.contains({ "1", "z" }));
    REQUIRE(modifiesSTable.contains({ "1", "c" }));
    REQUIRE(modifiesSTable.contains({ "1", "d" }));
    REQUIRE(modifiesSTable.contains({ "3", "x" }));
    REQUIRE(modifiesSTable.contains({ "3", "z" }));
    REQUIRE(modifiesSTable.contains({ "3", "c" }));
    REQUIRE(modifiesSTable.contains({ "3", "d" }));
    REQUIRE(modifiesSTable.contains({ "7", "z" }));
    REQUIRE(modifiesSTable.contains({ "7", "c" }));
    REQUIRE(modifiesSTable.contains({ "7", "d" }));
    REQUIRE(modifiesSTable.size() == 18);
    REQUIRE(parentTTable.contains({ "1", "4" }));
    REQUIRE(parentTTable.contains({ "1", "5" }));
    REQUIRE(parentTTable.contains({ "1", "6" }));
    REQUIRE(parentTTable.contains({ "1", "7" }));
    REQUIRE(parentTTable.contains({ "1", "8" }));
    REQUIRE(parentTTable.contains({ "1", "9" }));
    REQUIRE(parentTTable.contains({ "1", "10" }));
    REQUIRE(parentTTable.contains({ "3", "8" }));
    REQUIRE(parentTTable.contains({ "3", "9" }));
    REQUIRE(parentTTable.contains({ "3", "10" }));
    REQUIRE(parentTTable.size() == 19);
    REQUIRE(usesSTable.contains({ "1", "y1" }));
    REQUIRE(usesSTable.contains({ "1", "while" }));
    REQUIRE(usesSTable.contains({ "1", "x" }));
    REQUIRE(usesSTable.contains({ "1", "b" }));
    REQUIRE(usesSTable.contains({ "1", "f" }));
    REQUIRE(usesSTable.contains({ "1", "g" }));
    REQUIRE(usesSTable.contains({ "3", "x" }));
    REQUIRE(usesSTable.contains({ "3", "b" }));
    REQUIRE(usesSTable.contains({ "3", "f" }));
    REQUIRE(usesSTable.contains({ "3", "g" }));
    REQUIRE(usesSTable.contains({ "7", "f" }));
    REQUIRE(usesSTable.contains({ "7", "g" }));
    REQUIRE(usesSTable.size() == 18);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.size() == 0);
      REQUIRE(affectsTTable.size() == 0);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "a", "x1" }));
      REQUIRE(modifiesPTable.contains({ "a", "x" }));
      REQUIRE(modifiesPTable.contains({ "a", "z" }));
      REQUIRE(modifiesPTable.contains({ "a", "c" }));
      REQUIRE(modifiesPTable.contains({ "a", "d" }));
      REQUIRE(modifiesPTable.size() == 5);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "1", "3" }));
      REQUIRE(nextTable.contains({ "3", "4" }));
      REQUIRE(nextTable.contains({ "3", "6" }));
      REQUIRE(nextTable.contains({ "4", "5" }));
      REQUIRE(nextTable.contains({ "6", "7" }));
      REQUIRE(nextTable.contains({ "7", "8" }));
      REQUIRE(nextTable.contains({ "7", "9" }));
      REQUIRE(nextTable.contains({ "9", "10" }));
      REQUIRE(nextTable.size() == 9);
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "5" }));
      REQUIRE(nextTTable.contains({ "1", "6" }));
      REQUIRE(nextTTable.contains({ "1", "7" }));
      REQUIRE(nextTTable.contains({ "1", "8" }));
      REQUIRE(nextTTable.contains({ "1", "9" }));
      REQUIRE(nextTTable.contains({ "1", "10" }));
      REQUIRE(nextTTable.contains({ "3", "5" }));
      REQUIRE(nextTTable.contains({ "3", "7" }));
      REQUIRE(nextTTable.contains({ "3", "8" }));
      REQUIRE(nextTTable.contains({ "3", "9" }));
      REQUIRE(nextTTable.contains({ "3", "10" }));
      REQUIRE(nextTTable.contains({ "6", "8" }));
      REQUIRE(nextTTable.contains({ "6", "9" }));
      REQUIRE(nextTTable.contains({ "6", "10" }));
      REQUIRE(nextTTable.contains({ "7", "10" }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.contains({ "3", "while" }));
      REQUIRE(patternIfTable.contains({ "7", "b" }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ "a", "y1" }));
      REQUIRE(usesPTable.contains({ "a", "while" }));
      REQUIRE(usesPTable.contains({ "a", "x" }));
      REQUIRE(usesPTable.contains({ "a", "b" }));
      REQUIRE(usesPTable.contains({ "a", "f" }));
      REQUIRE(usesPTable.contains({ "a", "g" }));
      REQUIRE(usesPTable.size() == 6);
    }
  }
}

/////////////////////
// While statement //
/////////////////////

TEST_CASE("[TestSimpleParser] Invalid while statement - Parenthesis", "[SimpleParser][While][Syntax Error]") {
  SECTION("No parenthesis with cond expr") {
    std::string string("procedure while{while x==2 {x=1;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("No parenthesis no cond expr") {
    std::string string("procedure while{while {x=1;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Mismatched parenthesis") {
    std::string string("procedure while{while (x==2 {x=1;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Invalid while statement - Others", "[SimpleParser][While][Syntax Error]") {
  SECTION("No conditional expression") {
    std::string string("procedure while{while(){x=1;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Empty statement list") {
    std::string string("procedure while{while(if==else){}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Random chars between ')' and '{'") {
    std::string string("procedure while{while (if==else) himum { x = 1; }}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Variable as conditional expr") {
    std::string string("procedure then{if(true)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Constant as conditional expr") {
    std::string string("procedure then{if(5)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Expr as conditional expr") {
    std::string string("procedure then{if(x=6)then{x=1;}else{y=2;}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}

TEST_CASE("[TestSimpleParser] Valid while statement - Basic", "[SimpleParser][While]") {
  std::string string("procedure procedure{while(!((if==else)&&(43-3<while))){read read;}print print;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table whileTable = pkb.getWhileTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  REQUIRE(procTable.contains({ "procedure" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ "1" }));
  REQUIRE(whileTable.size() == 1);
  REQUIRE(printTable.contains({ "3" }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ "2" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "if" }));
  REQUIRE(varTable.contains({ "else" }));
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.contains({ "read" }));
  REQUIRE(varTable.contains({ "print" }));
  REQUIRE(varTable.size() == 5);
  REQUIRE(constTable.contains({ "43" }));
  REQUIRE(constTable.contains({ "3" }));
  REQUIRE(constTable.size() == 2);
  REQUIRE(stmtTable.size() == 3); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "1", "if" }));
  REQUIRE(usesSTable.contains({ "1", "else" }));
  REQUIRE(usesSTable.contains({ "1", "while" }));
  REQUIRE(usesSTable.contains({ "3", "print" }));
  REQUIRE(usesSTable.size() == 4);
  REQUIRE(modifiesSTable.contains({ "2", "read" }));
  REQUIRE(modifiesSTable.size() == 1);
  REQUIRE(followsTable.contains({ "1", "3" }));
  REQUIRE(followsTable.size() == 1);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.size() == 1);
  REQUIRE(patternAssignTable.size() == 0);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ "1", "read" }));
    REQUIRE(modifiesSTable.size() == 2);
    REQUIRE(parentTTable.size() == 1);
    REQUIRE(usesSTable.size() == 4);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.size() == 0);
      REQUIRE(affectsTTable.size() == 0);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "procedure", "read" }));
      REQUIRE(modifiesPTable.size() == 1);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "1", "3" }));
      REQUIRE(nextTable.contains({ "2", "1" }));
      REQUIRE(nextTable.size() == 3);
      REQUIRE(nextTTable.contains({ "1", "1" }));
      REQUIRE(nextTTable.contains({ "2", "2" }));
      REQUIRE(nextTTable.contains({ "2", "3" }));
      REQUIRE(nextTTable.size() == 6);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ "1", "if" }));
      REQUIRE(patternWhileTable.contains({ "1", "else" }));
      REQUIRE(patternWhileTable.contains({ "1", "while" }));
      REQUIRE(patternWhileTable.size() == 3);
      REQUIRE(usesPTable.contains({ "procedure", "if" }));
      REQUIRE(usesPTable.contains({ "procedure", "else" }));
      REQUIRE(usesPTable.contains({ "procedure", "while" }));
      REQUIRE(usesPTable.contains({ "procedure", "print" }));
      REQUIRE(usesPTable.size() == 4);
    }
  }
}

TEST_CASE("[TestSimpleParser] Valid while statement - Singly nested while", "[SimpleParser][While]") {
  std::string string("procedure i{read while;while(while<=2){while(!(i>while)){i=i+1;}print i;}print while;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table whileTable = pkb.getWhileTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString(" i 1 + ");

  REQUIRE(procTable.contains({ "i" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ "2" }));
  REQUIRE(whileTable.contains({ "3" }));
  REQUIRE(whileTable.size() == 2);
  REQUIRE(printTable.contains({ "5" }));
  REQUIRE(printTable.contains({ "6" }));
  REQUIRE(printTable.size() == 2);
  REQUIRE(readTable.contains({ "1" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.contains({ "i" }));
  REQUIRE(varTable.size() == 2);
  REQUIRE(constTable.contains({ "1" }));
  REQUIRE(constTable.contains({ "2" }));
  REQUIRE(constTable.size() == 2);
  REQUIRE(stmtTable.size() == 6); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "2", "while" }));
  REQUIRE(usesSTable.contains({ "3", "while" }));
  REQUIRE(usesSTable.contains({ "3", "i" }));
  REQUIRE(usesSTable.contains({ "4", "i" }));
  REQUIRE(usesSTable.contains({ "5", "i" }));
  REQUIRE(usesSTable.contains({ "6", "while" }));
  REQUIRE(usesSTable.size() == 6);
  REQUIRE(modifiesSTable.contains({ "1", "while" }));
  REQUIRE(modifiesSTable.contains({ "4", "i" }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.contains({ "1", "2" }));
  REQUIRE(followsTable.contains({ "3", "5" }));
  REQUIRE(followsTable.contains({ "2", "6" }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ "2", "3" }));
  REQUIRE(parentTable.contains({ "2", "5" }));
  REQUIRE(parentTable.contains({ "3", "4" }));
  REQUIRE(parentTable.size() == 3);
  REQUIRE(patternAssignTable.contains({ "4", "i", expectedPostFixString }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ "1", "6" }));
    REQUIRE(followsTTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ "2", "i" }));
    REQUIRE(modifiesSTable.contains({ "3", "i" }));
    REQUIRE(modifiesSTable.size() == 4);
    REQUIRE(parentTTable.contains({ "2", "4" }));
    REQUIRE(parentTTable.size() == 4);
    REQUIRE(usesSTable.contains({ "2", "i" }));
    REQUIRE(usesSTable.size() == 7);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.contains({ "4", "4" }));
      REQUIRE(affectsTable.size() == 1);
      REQUIRE(affectsTTable.size() == 1);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "i", "while" }));
      REQUIRE(modifiesPTable.contains({ "i", "i" }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "2", "3" }));
      REQUIRE(nextTable.contains({ "2", "6" }));
      REQUIRE(nextTable.contains({ "3", "4" }));
      REQUIRE(nextTable.contains({ "3", "5" }));
      REQUIRE(nextTable.contains({ "4", "3" }));
      REQUIRE(nextTable.contains({ "5", "2" }));
      REQUIRE(nextTable.size() == 7);
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "5" }));
      REQUIRE(nextTTable.contains({ "1", "6" }));
      REQUIRE(nextTTable.contains({ "2", "2" }));
      REQUIRE(nextTTable.contains({ "2", "4" }));
      REQUIRE(nextTTable.contains({ "2", "5" }));
      REQUIRE(nextTTable.contains({ "3", "2" }));
      REQUIRE(nextTTable.contains({ "3", "3" }));
      REQUIRE(nextTTable.contains({ "3", "6" }));
      REQUIRE(nextTTable.contains({ "4", "2" }));
      REQUIRE(nextTTable.contains({ "4", "4" }));
      REQUIRE(nextTTable.contains({ "4", "5" }));
      REQUIRE(nextTTable.contains({ "4", "6" }));
      REQUIRE(nextTTable.contains({ "5", "3" }));
      REQUIRE(nextTTable.contains({ "5", "4" }));
      REQUIRE(nextTTable.contains({ "5", "5" }));
      REQUIRE(nextTTable.contains({ "5", "6" }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ "2", "while" }));
      REQUIRE(patternWhileTable.contains({ "3", "i" }));
      REQUIRE(patternWhileTable.contains({ "3", "while" }));
      REQUIRE(patternWhileTable.size() == 3);
      REQUIRE(usesPTable.contains({ "i", "i" }));
      REQUIRE(usesPTable.contains({ "i", "while" }));
      REQUIRE(usesPTable.size() == 2);
    }
  }
}

TEST_CASE("[TestSimpleParser] Valid while statement - Doubly nested while", "[SimpleParser][While]") {
  std::string string("procedure i{while(a==b){print=print+1;while(c>d){while(e%f==4){read=read/3;}if=if*2;}while=while%0;}call=call-call;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table whileTable = pkb.getWhileTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString1(" print 1 + ");
  std::string expectedPostFixString2(" read 3 / ");
  std::string expectedPostFixString3(" if 2 * ");
  std::string expectedPostFixString4(" while 0 % ");
  std::string expectedPostFixString5(" call call - ");

  REQUIRE(procTable.contains({ "i" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ "1" }));
  REQUIRE(whileTable.contains({ "3" }));
  REQUIRE(whileTable.contains({ "4" }));
  REQUIRE(whileTable.size() == 3);
  REQUIRE(printTable.size() == 0);
  REQUIRE(readTable.size() == 0);
  REQUIRE(varTable.contains({ "a" }));
  REQUIRE(varTable.contains({ "b" }));
  REQUIRE(varTable.contains({ "c" }));
  REQUIRE(varTable.contains({ "d" }));
  REQUIRE(varTable.contains({ "e" }));
  REQUIRE(varTable.contains({ "f" }));
  REQUIRE(varTable.contains({ "call" }));
  REQUIRE(varTable.contains({ "if" }));
  REQUIRE(varTable.contains({ "print" }));
  REQUIRE(varTable.contains({ "read" }));
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.size() == 11);
  REQUIRE(constTable.contains({ "0" }));
  REQUIRE(constTable.contains({ "1" }));
  REQUIRE(constTable.contains({ "2" }));
  REQUIRE(constTable.contains({ "3" }));
  REQUIRE(constTable.contains({ "4" }));
  REQUIRE(constTable.size() == 5);
  REQUIRE(stmtTable.size() == 8); // maybe can also check the statement num? maybe not necessary
  REQUIRE(usesSTable.contains({ "1", "a" }));
  REQUIRE(usesSTable.contains({ "1", "b" }));
  REQUIRE(usesSTable.contains({ "2", "print" }));
  REQUIRE(usesSTable.contains({ "3", "c" }));
  REQUIRE(usesSTable.contains({ "3", "d" }));
  REQUIRE(usesSTable.contains({ "4", "e" }));
  REQUIRE(usesSTable.contains({ "4", "f" }));
  REQUIRE(usesSTable.contains({ "5", "read" }));
  REQUIRE(usesSTable.contains({ "6", "if" }));
  REQUIRE(usesSTable.contains({ "7", "while" }));
  REQUIRE(usesSTable.contains({ "8", "call" }));
  REQUIRE(usesSTable.size() == 11);
  REQUIRE(modifiesSTable.contains({ "2", "print" }));
  REQUIRE(modifiesSTable.contains({ "5", "read" }));
  REQUIRE(modifiesSTable.contains({ "6", "if" }));
  REQUIRE(modifiesSTable.contains({ "7", "while" }));
  REQUIRE(modifiesSTable.contains({ "8", "call" }));
  REQUIRE(modifiesSTable.size() == 5);
  REQUIRE(followsTable.contains({ "1", "8" }));
  REQUIRE(followsTable.contains({ "2", "3" }));
  REQUIRE(followsTable.contains({ "3", "7" }));
  REQUIRE(followsTable.contains({ "4", "6" }));
  REQUIRE(followsTable.size() == 4);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.contains({ "1", "3" }));
  REQUIRE(parentTable.contains({ "1", "7" }));
  REQUIRE(parentTable.contains({ "3", "4" }));
  REQUIRE(parentTable.contains({ "3", "6" }));
  REQUIRE(parentTable.contains({ "4", "5" }));
  REQUIRE(parentTable.size() == 6);
  REQUIRE(patternAssignTable.contains({ "2", "print", expectedPostFixString1 }));
  REQUIRE(patternAssignTable.contains({ "5", "read", expectedPostFixString2 }));
  REQUIRE(patternAssignTable.contains({ "6", "if", expectedPostFixString3 }));
  REQUIRE(patternAssignTable.contains({ "7", "while", expectedPostFixString4 }));
  REQUIRE(patternAssignTable.contains({ "8", "call", expectedPostFixString5 }));
  REQUIRE(patternAssignTable.size() == 5);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ "2", "7" }));
    REQUIRE(followsTTable.size() == 5);
    REQUIRE(modifiesSTable.contains({ "1", "print" }));
    REQUIRE(modifiesSTable.contains({ "1", "read" }));
    REQUIRE(modifiesSTable.contains({ "1", "if" }));
    REQUIRE(modifiesSTable.contains({ "1", "while" }));
    REQUIRE(modifiesSTable.contains({ "3", "read" }));
    REQUIRE(modifiesSTable.contains({ "3", "if" }));
    REQUIRE(modifiesSTable.contains({ "4", "read" }));
    REQUIRE(modifiesSTable.size() == 12);
    REQUIRE(parentTTable.contains({ "1", "4" }));
    REQUIRE(parentTTable.contains({ "1", "5" }));
    REQUIRE(parentTTable.contains({ "1", "6" }));
    REQUIRE(parentTTable.contains({ "3", "5" }));
    REQUIRE(parentTTable.size() == 10);
    REQUIRE(usesSTable.contains({ "1", "print" }));
    REQUIRE(usesSTable.contains({ "1", "c" }));
    REQUIRE(usesSTable.contains({ "1", "d" }));
    REQUIRE(usesSTable.contains({ "1", "e" }));
    REQUIRE(usesSTable.contains({ "1", "f" }));
    REQUIRE(usesSTable.contains({ "1", "read" }));
    REQUIRE(usesSTable.contains({ "1", "if" }));
    REQUIRE(usesSTable.contains({ "1", "while" }));
    REQUIRE(usesSTable.contains({ "3", "e" }));
    REQUIRE(usesSTable.contains({ "3", "f" }));
    REQUIRE(usesSTable.contains({ "3", "read" }));
    REQUIRE(usesSTable.contains({ "3", "if" }));
    REQUIRE(usesSTable.contains({ "4", "read" }));
    REQUIRE(usesSTable.size() == 24);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.contains({ "2", "2" }));
      REQUIRE(affectsTable.contains({ "5", "5" }));
      REQUIRE(affectsTable.contains({ "6", "6" }));
      REQUIRE(affectsTable.contains({ "7", "7" }));
      REQUIRE(affectsTable.size() == 4);
      REQUIRE(affectsTTable.size() == 4);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "i", "print" }));
      REQUIRE(modifiesPTable.contains({ "i", "read" }));
      REQUIRE(modifiesPTable.contains({ "i", "if" }));
      REQUIRE(modifiesPTable.contains({ "i", "while" }));
      REQUIRE(modifiesPTable.contains({ "i", "call" }));
      REQUIRE(modifiesPTable.size() == 5);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "1", "8" }));
      REQUIRE(nextTable.contains({ "2", "3" }));
      REQUIRE(nextTable.contains({ "3", "4" }));
      REQUIRE(nextTable.contains({ "3", "7" }));
      REQUIRE(nextTable.contains({ "4", "5" }));
      REQUIRE(nextTable.contains({ "4", "6" }));
      REQUIRE(nextTable.contains({ "5", "4" }));
      REQUIRE(nextTable.contains({ "6", "3" }));
      REQUIRE(nextTable.contains({ "7", "1" }));
      REQUIRE(nextTable.size() == 10);
      REQUIRE(nextTTable.contains({ "1", "1" }));
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "5" }));
      REQUIRE(nextTTable.contains({ "1", "6" }));
      REQUIRE(nextTTable.contains({ "1", "7" }));
      REQUIRE(nextTTable.contains({ "2", "1" }));
      REQUIRE(nextTTable.contains({ "2", "2" }));
      REQUIRE(nextTTable.contains({ "2", "4" }));
      REQUIRE(nextTTable.contains({ "2", "5" }));
      REQUIRE(nextTTable.contains({ "2", "6" }));
      REQUIRE(nextTTable.contains({ "2", "7" }));
      REQUIRE(nextTTable.contains({ "2", "8" }));
      REQUIRE(nextTTable.contains({ "3", "1" }));
      REQUIRE(nextTTable.contains({ "3", "2" }));
      REQUIRE(nextTTable.contains({ "3", "3" }));
      REQUIRE(nextTTable.contains({ "3", "5" }));
      REQUIRE(nextTTable.contains({ "3", "6" }));
      REQUIRE(nextTTable.contains({ "3", "8" }));
      REQUIRE(nextTTable.contains({ "4", "1" }));
      REQUIRE(nextTTable.contains({ "4", "2" }));
      REQUIRE(nextTTable.contains({ "4", "3" }));
      REQUIRE(nextTTable.contains({ "4", "4" }));
      REQUIRE(nextTTable.contains({ "4", "7" }));
      REQUIRE(nextTTable.contains({ "4", "8" }));
      REQUIRE(nextTTable.contains({ "5", "1" }));
      REQUIRE(nextTTable.contains({ "5", "2" }));
      REQUIRE(nextTTable.contains({ "5", "3" }));
      REQUIRE(nextTTable.contains({ "5", "5" }));
      REQUIRE(nextTTable.contains({ "5", "6" }));
      REQUIRE(nextTTable.contains({ "5", "7" }));
      REQUIRE(nextTTable.contains({ "5", "8" }));
      REQUIRE(nextTTable.contains({ "6", "1" }));
      REQUIRE(nextTTable.contains({ "6", "2" }));
      REQUIRE(nextTTable.contains({ "6", "4" }));
      REQUIRE(nextTTable.contains({ "6", "5" }));
      REQUIRE(nextTTable.contains({ "6", "6" }));
      REQUIRE(nextTTable.contains({ "6", "7" }));
      REQUIRE(nextTTable.contains({ "6", "8" }));
      REQUIRE(nextTTable.contains({ "7", "2" }));
      REQUIRE(nextTTable.contains({ "7", "3" }));
      REQUIRE(nextTTable.contains({ "7", "4" }));
      REQUIRE(nextTTable.contains({ "7", "5" }));
      REQUIRE(nextTTable.contains({ "7", "6" }));
      REQUIRE(nextTTable.contains({ "7", "7" }));
      REQUIRE(nextTTable.contains({ "7", "8" }));
      REQUIRE(nextTTable.size() == 56);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ "1", "a" }));
      REQUIRE(patternWhileTable.contains({ "1", "b" }));
      REQUIRE(patternWhileTable.contains({ "3", "c" }));
      REQUIRE(patternWhileTable.contains({ "3", "d" }));
      REQUIRE(patternWhileTable.contains({ "4", "e" }));
      REQUIRE(patternWhileTable.contains({ "4", "f" }));
      REQUIRE(patternWhileTable.size() == 6);
      REQUIRE(usesPTable.contains({ "i", "a" }));
      REQUIRE(usesPTable.contains({ "i", "b" }));
      REQUIRE(usesPTable.contains({ "i", "c" }));
      REQUIRE(usesPTable.contains({ "i", "d" }));
      REQUIRE(usesPTable.contains({ "i", "e" }));
      REQUIRE(usesPTable.contains({ "i", "f" }));
      REQUIRE(usesPTable.contains({ "i", "print" }));
      REQUIRE(usesPTable.contains({ "i", "read" }));
      REQUIRE(usesPTable.contains({ "i", "if" }));
      REQUIRE(usesPTable.contains({ "i", "while" }));
      REQUIRE(usesPTable.contains({ "i", "call" }));
      REQUIRE(usesPTable.size() == 11);
    }
  }
}

/////////////////////////////////
// Combination of if and while //
/////////////////////////////////

TEST_CASE("[TestSimpleParser] Loops - Nested/follows if/while loops", "[SimpleParser][Assign][If][Print][Read][While]") {
  std::string string("procedure proc{read D33z;while(D33z>6){life=42;if(life==bad)then{print = read;}else{while(D33z!=life){D33z=life-1;}}while(bad==2){print nu7z;}}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table ifTable = pkb.getIfTable();
  Table whileTable = pkb.getWhileTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString1(" 42 ");
  std::string expectedPostFixString2(" read ");
  std::string expectedPostFixString3(" life 1 - ");

  REQUIRE(procTable.contains({ "proc" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(ifTable.contains({ "4" }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(whileTable.contains({ "2" }));
  REQUIRE(whileTable.contains({ "6" }));
  REQUIRE(whileTable.contains({ "8" }));
  REQUIRE(whileTable.size() == 3);
  REQUIRE(printTable.contains({ "9" }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ "1" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "D33z" }));
  REQUIRE(varTable.contains({ "nu7z" }));
  REQUIRE(varTable.contains({ "life" }));
  REQUIRE(varTable.contains({ "bad" }));
  REQUIRE(varTable.contains({ "print" }));
  REQUIRE(varTable.contains({ "read" }));
  REQUIRE(varTable.size() == 6);
  REQUIRE(constTable.contains({ "1" }));
  REQUIRE(constTable.contains({ "2" }));
  REQUIRE(constTable.contains({ "6" }));
  REQUIRE(constTable.contains({ "42" }));
  REQUIRE(constTable.size() == 4);
  REQUIRE(stmtTable.size() == 9);
  REQUIRE(usesSTable.contains({ "2", "D33z" }));
  REQUIRE(usesSTable.contains({ "4", "life" }));
  REQUIRE(usesSTable.contains({ "4", "bad" }));
  REQUIRE(usesSTable.contains({ "5", "read" }));
  REQUIRE(usesSTable.contains({ "6", "D33z" }));
  REQUIRE(usesSTable.contains({ "6", "life" }));
  REQUIRE(usesSTable.contains({ "7", "life" }));
  REQUIRE(usesSTable.contains({ "8", "bad" }));
  REQUIRE(usesSTable.contains({ "9", "nu7z" }));
  REQUIRE(usesSTable.size() == 9);
  REQUIRE(modifiesSTable.contains({ "1", "D33z" }));
  REQUIRE(modifiesSTable.contains({ "3", "life" }));
  REQUIRE(modifiesSTable.contains({ "5", "print" }));
  REQUIRE(modifiesSTable.contains({ "7", "D33z" }));
  REQUIRE(modifiesSTable.size() == 4);
  REQUIRE(followsTable.contains({ "1", "2" }));
  REQUIRE(followsTable.contains({ "3", "4" }));
  REQUIRE(followsTable.contains({ "4", "8" }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ "2", "3" }));
  REQUIRE(parentTable.contains({ "2", "4" }));
  REQUIRE(parentTable.contains({ "2", "8" }));
  REQUIRE(parentTable.contains({ "4", "5" }));
  REQUIRE(parentTable.contains({ "4", "6" }));
  REQUIRE(parentTable.contains({ "6", "7" }));
  REQUIRE(parentTable.contains({ "8", "9" }));
  REQUIRE(parentTable.size() == 7);
  REQUIRE(patternAssignTable.contains({ "3", "life", expectedPostFixString1 }));
  REQUIRE(patternAssignTable.contains({ "5", "print", expectedPostFixString2 }));
  REQUIRE(patternAssignTable.contains({ "7", "D33z", expectedPostFixString3 }));
  REQUIRE(patternAssignTable.size() == 3);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ "3", "8" }));
    REQUIRE(followsTTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ "2", "life" }));
    REQUIRE(modifiesSTable.contains({ "2", "print" }));
    REQUIRE(modifiesSTable.contains({ "2", "D33z" }));
    REQUIRE(modifiesSTable.contains({ "4", "print" }));
    REQUIRE(modifiesSTable.contains({ "4", "D33z" }));
    REQUIRE(modifiesSTable.contains({ "6", "D33z" }));
    REQUIRE(modifiesSTable.size() == 10);
    REQUIRE(parentTTable.contains({ "2", "5" }));
    REQUIRE(parentTTable.contains({ "2", "6" }));
    REQUIRE(parentTTable.contains({ "2", "7" }));
    REQUIRE(parentTTable.contains({ "2", "9" }));
    REQUIRE(parentTTable.contains({ "4", "7" }));
    REQUIRE(parentTTable.size() == 12);
    REQUIRE(usesSTable.contains({ "2", "life" }));
    REQUIRE(usesSTable.contains({ "2", "bad" }));
    REQUIRE(usesSTable.contains({ "2", "read" }));
    REQUIRE(usesSTable.contains({ "2", "nu7z" }));
    REQUIRE(usesSTable.contains({ "4", "read" }));
    REQUIRE(usesSTable.contains({ "4", "D33z" }));
    REQUIRE(usesSTable.contains({ "8", "nu7z" }));
    REQUIRE(usesSTable.size() == 16);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.contains({ "3", "7" }));
      REQUIRE(affectsTable.size() == 1);
      REQUIRE(affectsTTable.size() == 1);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "proc", "life" }));
      REQUIRE(modifiesPTable.contains({ "proc", "print" }));
      REQUIRE(modifiesPTable.contains({ "proc", "D33z" }));
      REQUIRE(modifiesPTable.size() == 3);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "2", "3" }));
      REQUIRE(nextTable.contains({ "3", "4" }));
      REQUIRE(nextTable.contains({ "4", "5" }));
      REQUIRE(nextTable.contains({ "4", "6" }));
      REQUIRE(nextTable.contains({ "5", "8" }));
      REQUIRE(nextTable.contains({ "6", "7" }));
      REQUIRE(nextTable.contains({ "6", "8" }));
      REQUIRE(nextTable.contains({ "7", "6" }));
      REQUIRE(nextTable.contains({ "8", "9" }));
      REQUIRE(nextTable.contains({ "8", "2" }));
      REQUIRE(nextTable.contains({ "9", "8" }));
      REQUIRE(nextTable.size() == 12);
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "5" }));
      REQUIRE(nextTTable.contains({ "1", "6" }));
      REQUIRE(nextTTable.contains({ "1", "7" }));
      REQUIRE(nextTTable.contains({ "1", "8" }));
      REQUIRE(nextTTable.contains({ "1", "9" }));
      REQUIRE(nextTTable.contains({ "2", "2" }));
      REQUIRE(nextTTable.contains({ "2", "4" }));
      REQUIRE(nextTTable.contains({ "2", "5" }));
      REQUIRE(nextTTable.contains({ "2", "6" }));
      REQUIRE(nextTTable.contains({ "2", "7" }));
      REQUIRE(nextTTable.contains({ "2", "8" }));
      REQUIRE(nextTTable.contains({ "2", "9" }));
      REQUIRE(nextTTable.contains({ "3", "2" }));
      REQUIRE(nextTTable.contains({ "3", "3" }));
      REQUIRE(nextTTable.contains({ "3", "5" }));
      REQUIRE(nextTTable.contains({ "3", "6" }));
      REQUIRE(nextTTable.contains({ "3", "7" }));
      REQUIRE(nextTTable.contains({ "3", "8" }));
      REQUIRE(nextTTable.contains({ "3", "9" }));
      REQUIRE(nextTTable.contains({ "4", "2" }));
      REQUIRE(nextTTable.contains({ "4", "3" }));
      REQUIRE(nextTTable.contains({ "4", "4" }));
      REQUIRE(nextTTable.contains({ "4", "7" }));
      REQUIRE(nextTTable.contains({ "4", "8" }));
      REQUIRE(nextTTable.contains({ "4", "9" }));
      REQUIRE(nextTTable.contains({ "5", "2" }));
      REQUIRE(nextTTable.contains({ "5", "3" }));
      REQUIRE(nextTTable.contains({ "5", "4" }));
      REQUIRE(nextTTable.contains({ "5", "5" }));
      REQUIRE(nextTTable.contains({ "5", "6" }));
      REQUIRE(nextTTable.contains({ "5", "7" }));
      REQUIRE(nextTTable.contains({ "5", "9" }));
      REQUIRE(nextTTable.contains({ "6", "2" }));
      REQUIRE(nextTTable.contains({ "6", "3" }));
      REQUIRE(nextTTable.contains({ "6", "4" }));
      REQUIRE(nextTTable.contains({ "6", "5" }));
      REQUIRE(nextTTable.contains({ "6", "6" }));
      REQUIRE(nextTTable.contains({ "6", "9" }));
      REQUIRE(nextTTable.contains({ "7", "2" }));
      REQUIRE(nextTTable.contains({ "7", "3" }));
      REQUIRE(nextTTable.contains({ "7", "4" }));
      REQUIRE(nextTTable.contains({ "7", "5" }));
      REQUIRE(nextTTable.contains({ "7", "7" }));
      REQUIRE(nextTTable.contains({ "7", "8" }));
      REQUIRE(nextTTable.contains({ "7", "9" }));
      REQUIRE(nextTTable.contains({ "8", "3" }));
      REQUIRE(nextTTable.contains({ "8", "4" }));
      REQUIRE(nextTTable.contains({ "8", "5" }));
      REQUIRE(nextTTable.contains({ "8", "6" }));
      REQUIRE(nextTTable.contains({ "8", "7" }));
      REQUIRE(nextTTable.contains({ "8", "8" }));
      REQUIRE(nextTTable.contains({ "9", "2" }));
      REQUIRE(nextTTable.contains({ "9", "3" }));
      REQUIRE(nextTTable.contains({ "9", "4" }));
      REQUIRE(nextTTable.contains({ "9", "5" }));
      REQUIRE(nextTTable.contains({ "9", "6" }));
      REQUIRE(nextTTable.contains({ "9", "7" }));
      REQUIRE(nextTTable.contains({ "9", "9" }));
      REQUIRE(nextTTable.size() == 72);
      REQUIRE(patternIfTable.contains({ "4", "life" }));
      REQUIRE(patternIfTable.contains({ "4", "bad" }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.contains({ "2", "D33z" }));
      REQUIRE(patternWhileTable.contains({ "6", "D33z" }));
      REQUIRE(patternWhileTable.contains({ "6", "life" }));
      REQUIRE(patternWhileTable.contains({ "8", "bad" }));
      REQUIRE(patternWhileTable.size() == 4);
      REQUIRE(usesPTable.contains({ "proc", "D33z" }));
      REQUIRE(usesPTable.contains({ "proc", "life" }));
      REQUIRE(usesPTable.contains({ "proc", "bad" }));
      REQUIRE(usesPTable.contains({ "proc", "read" }));
      REQUIRE(usesPTable.contains({ "proc", "nu7z" }));
      REQUIRE(usesPTable.size() == 5);
    }
  }
}

TEST_CASE("[TestSimpleParser] Loops - Random example", "[SimpleParser][Assign][If][Print][Read][While]") {
  std::string string(R"(
    procedure main {while (((while + (print)) != read) || ((if) <= else)) {if (!((if > then) && (print < while))) then {
    read read;} else {while (print >= (((call) + ((read))) % else)) {print = call + procedure;}}}})");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  Table procTable = pkb.getProcTable();
  Table ifTable = pkb.getIfTable();
  Table whileTable = pkb.getWhileTable();
  Table printTable = pkb.getPrintTable();
  Table readTable = pkb.getReadTable();
  Table varTable = pkb.getVarTable();
  Table constTable = pkb.getConstTable();
  Table stmtTable = pkb.getStmtTable();
  Table usesSTable = pkb.getUsesSTable();
  Table modifiesSTable = pkb.getModifiesSTable();
  Table followsTable = pkb.getFollowsTable();
  Table parentTable = pkb.getParentTable();
  Table patternAssignTable = pkb.getPatternAssignTable();

  std::string expectedPostFixString(" call procedure + ");

  REQUIRE(procTable.contains({ "main" }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(ifTable.contains({ "2" }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(whileTable.contains({ "1" }));
  REQUIRE(whileTable.contains({ "4" }));
  REQUIRE(whileTable.size() == 2);
  REQUIRE(printTable.size() == 0);
  REQUIRE(readTable.contains({ "3" }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ "while" }));
  REQUIRE(varTable.contains({ "print" }));
  REQUIRE(varTable.contains({ "read" }));
  REQUIRE(varTable.contains({ "if" }));
  REQUIRE(varTable.contains({ "else" }));
  REQUIRE(varTable.contains({ "then" }));
  REQUIRE(varTable.contains({ "call" }));
  REQUIRE(varTable.contains({ "procedure" }));
  REQUIRE(varTable.size() == 8);
  REQUIRE(constTable.size() == 0);
  REQUIRE(stmtTable.size() == 5);
  REQUIRE(usesSTable.contains({ "1", "while" }));
  REQUIRE(usesSTable.contains({ "1", "print" }));
  REQUIRE(usesSTable.contains({ "1", "read" }));
  REQUIRE(usesSTable.contains({ "1", "if" }));
  REQUIRE(usesSTable.contains({ "1", "else" }));
  REQUIRE(usesSTable.contains({ "2", "if" }));
  REQUIRE(usesSTable.contains({ "2", "then" }));
  REQUIRE(usesSTable.contains({ "2", "print" }));
  REQUIRE(usesSTable.contains({ "2", "while" }));
  REQUIRE(usesSTable.contains({ "4", "print" }));
  REQUIRE(usesSTable.contains({ "4", "call" }));
  REQUIRE(usesSTable.contains({ "4", "read" }));
  REQUIRE(usesSTable.contains({ "4", "else" }));
  REQUIRE(usesSTable.contains({ "5", "call" }));
  REQUIRE(usesSTable.contains({ "5", "procedure" }));
  REQUIRE(usesSTable.size() == 15);
  REQUIRE(modifiesSTable.contains({ "3", "read" }));
  REQUIRE(modifiesSTable.contains({ "5", "print" }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ "1", "2" }));
  REQUIRE(parentTable.contains({ "2", "3" }));
  REQUIRE(parentTable.contains({ "2", "4" }));
  REQUIRE(parentTable.contains({ "4", "5" }));
  REQUIRE(parentTable.size() == 4);
  REQUIRE(patternAssignTable.contains({ "5", "print", expectedPostFixString }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ "1", "read" }));
    REQUIRE(modifiesSTable.contains({ "1", "print" }));
    REQUIRE(modifiesSTable.contains({ "2", "read" }));
    REQUIRE(modifiesSTable.contains({ "2", "print" }));
    REQUIRE(modifiesSTable.contains({ "4", "print" }));
    REQUIRE(modifiesSTable.size() == 7);
    REQUIRE(parentTTable.contains({ "1", "3" }));
    REQUIRE(parentTTable.contains({ "1", "4" }));
    REQUIRE(parentTTable.contains({ "1", "5" }));
    REQUIRE(parentTTable.contains({ "2", "5" }));
    REQUIRE(parentTTable.size() == 8);
    REQUIRE(usesSTable.contains({ "1", "then" }));
    REQUIRE(usesSTable.contains({ "1", "call" }));
    REQUIRE(usesSTable.contains({ "1", "procedure" }));
    REQUIRE(usesSTable.contains({ "2", "call" }));
    REQUIRE(usesSTable.contains({ "2", "read" }));
    REQUIRE(usesSTable.contains({ "2", "else" }));
    REQUIRE(usesSTable.contains({ "2", "procedure" }));
    REQUIRE(usesSTable.contains({ "4", "procedure" }));
    REQUIRE(usesSTable.size() == 23);

    SECTION("Advanced SPA relations") {
      Table affectsTable = pkb.getAffectsTable();
      Table affectsTTable = pkb.getAffectsTTable();
      Table callsTable = pkb.getCallsTable();
      Table callsTTable = pkb.getCallsTTable();
      Table modifiesPTable = pkb.getModifiesPTable();
      Table nextTable = pkb.getNextTable();
      Table nextTTable = pkb.getNextTTable();
      Table patternIfTable = pkb.getPatternIfTable();
      Table patternWhileTable = pkb.getPatternWhileTable();
      Table usesPTable = pkb.getUsesPTable();

      REQUIRE(affectsTable.size() == 0);
      REQUIRE(affectsTTable.size() == 0);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ "main", "read" }));
      REQUIRE(modifiesPTable.contains({ "main", "print" }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ "1", "2" }));
      REQUIRE(nextTable.contains({ "2", "3" }));
      REQUIRE(nextTable.contains({ "2", "4" }));
      REQUIRE(nextTable.contains({ "3", "1" }));
      REQUIRE(nextTable.contains({ "4", "5" }));
      REQUIRE(nextTable.contains({ "4", "1" }));
      REQUIRE(nextTable.contains({ "5", "4" }));
      REQUIRE(nextTable.size() == 7);
      REQUIRE(nextTTable.contains({ "1", "1" }));
      REQUIRE(nextTTable.contains({ "1", "3" }));
      REQUIRE(nextTTable.contains({ "1", "4" }));
      REQUIRE(nextTTable.contains({ "1", "5" }));
      REQUIRE(nextTTable.contains({ "2", "1" }));
      REQUIRE(nextTTable.contains({ "2", "2" }));
      REQUIRE(nextTTable.contains({ "2", "5" }));
      REQUIRE(nextTTable.contains({ "3", "2" }));
      REQUIRE(nextTTable.contains({ "3", "3" }));
      REQUIRE(nextTTable.contains({ "3", "4" }));
      REQUIRE(nextTTable.contains({ "3", "5" }));
      REQUIRE(nextTTable.contains({ "4", "2" }));
      REQUIRE(nextTTable.contains({ "4", "3" }));
      REQUIRE(nextTTable.contains({ "4", "4" }));
      REQUIRE(nextTTable.contains({ "5", "1" }));
      REQUIRE(nextTTable.contains({ "5", "2" }));
      REQUIRE(nextTTable.contains({ "5", "3" }));
      REQUIRE(nextTTable.contains({ "5", "5" }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.contains({ "2", "if" }));
      REQUIRE(patternIfTable.contains({ "2", "then" }));
      REQUIRE(patternIfTable.contains({ "2", "print" }));
      REQUIRE(patternIfTable.contains({ "2", "while" }));
      REQUIRE(patternIfTable.size() == 4);
      REQUIRE(patternWhileTable.contains({ "1", "while" }));
      REQUIRE(patternWhileTable.contains({ "1", "print" }));
      REQUIRE(patternWhileTable.contains({ "1", "read" }));
      REQUIRE(patternWhileTable.contains({ "1", "if" }));
      REQUIRE(patternWhileTable.contains({ "1", "else" }));
      REQUIRE(patternWhileTable.contains({ "4", "print" }));
      REQUIRE(patternWhileTable.contains({ "4", "call" }));
      REQUIRE(patternWhileTable.contains({ "4", "read" }));
      REQUIRE(patternWhileTable.contains({ "4", "else" }));
      REQUIRE(patternWhileTable.size() == 9);
      REQUIRE(usesPTable.contains({ "main", "while" }));
      REQUIRE(usesPTable.contains({ "main", "print" }));
      REQUIRE(usesPTable.contains({ "main", "read" }));
      REQUIRE(usesPTable.contains({ "main", "if" }));
      REQUIRE(usesPTable.contains({ "main", "else" }));
      REQUIRE(usesPTable.contains({ "main", "then" }));
      REQUIRE(usesPTable.contains({ "main", "call" }));
      REQUIRE(usesPTable.contains({ "main", "procedure" }));
      REQUIRE(usesPTable.size() == 8);
    }
  }
}

TEST_CASE("[TestSimpleParser] Loops - Random example, but with extra parenthesis around cond expr", "[SimpleParser][Assign][If][Print][Read][While]") {
  std::string string(R"(
    procedure main {while ((((while + (print)) != read) || ((if) <= else))) {if (!((if > then) && (print < while))) then {
    read read;} else {while (print >= (((call) + ((read))) % else)) {print = call + procedure;}}}})");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  REQUIRE_THROWS(parser.parse());
}

/////////////////////////////
// Next and NextT relation //
/////////////////////////////

TEST_CASE("[TestSimpleParser] Next relation - No loops", "[SimpleParser][Next]") {
  std::string string("procedure a{x=0;read y;print z;call b;}procedure b{a=1;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "3", "4" }));
    REQUIRE(nextTable.size() == 3);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "2", "4" }));
    REQUIRE(nextTTable.size() == 6);
  }
}

TEST_CASE("[TestSimpleParser] Next relation - Example CFG (Figure 5) - Advanced SPA requirements", "[SimpleParser][Next]") {
  std::string string("procedure Second{x=0;i=5;while(i!=0){x=x+2*y;print Third;i=i-1;}if(x==1)then{x=x+1;}else{z=1;}z=z+x+i;y=z+2;x=x*y+z;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "3", "4" }));
    REQUIRE(nextTable.contains({ "3", "7" }));
    REQUIRE(nextTable.contains({ "4", "5" }));
    REQUIRE(nextTable.contains({ "5", "6" }));
    REQUIRE(nextTable.contains({ "6", "3" }));
    REQUIRE(nextTable.contains({ "7", "8" }));
    REQUIRE(nextTable.contains({ "7", "9" }));
    REQUIRE(nextTable.contains({ "8", "10" }));
    REQUIRE(nextTable.contains({ "9", "10" }));
    REQUIRE(nextTable.contains({ "10", "11" }));
    REQUIRE(nextTable.contains({ "11", "12" }));
    REQUIRE(nextTable.size() == 13);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "1", "5" }));
    REQUIRE(nextTTable.contains({ "1", "6" }));
    REQUIRE(nextTTable.contains({ "1", "7" }));
    REQUIRE(nextTTable.contains({ "1", "8" }));
    REQUIRE(nextTTable.contains({ "1", "9" }));
    REQUIRE(nextTTable.contains({ "1", "10" }));
    REQUIRE(nextTTable.contains({ "1", "11" }));
    REQUIRE(nextTTable.contains({ "1", "12" }));
    REQUIRE(nextTTable.contains({ "2", "4" }));
    REQUIRE(nextTTable.contains({ "2", "5" }));
    REQUIRE(nextTTable.contains({ "2", "6" }));
    REQUIRE(nextTTable.contains({ "2", "7" }));
    REQUIRE(nextTTable.contains({ "2", "8" }));
    REQUIRE(nextTTable.contains({ "2", "9" }));
    REQUIRE(nextTTable.contains({ "2", "10" }));
    REQUIRE(nextTTable.contains({ "2", "11" }));
    REQUIRE(nextTTable.contains({ "2", "12" }));
    REQUIRE(nextTTable.contains({ "3", "3" }));
    REQUIRE(nextTTable.contains({ "3", "5" }));
    REQUIRE(nextTTable.contains({ "3", "6" }));
    REQUIRE(nextTTable.contains({ "3", "8" }));
    REQUIRE(nextTTable.contains({ "3", "9" }));
    REQUIRE(nextTTable.contains({ "3", "10" }));
    REQUIRE(nextTTable.contains({ "3", "11" }));
    REQUIRE(nextTTable.contains({ "3", "12" }));
    REQUIRE(nextTTable.contains({ "4", "3" }));
    REQUIRE(nextTTable.contains({ "4", "4" }));
    REQUIRE(nextTTable.contains({ "4", "6" }));
    REQUIRE(nextTTable.contains({ "4", "7" }));
    REQUIRE(nextTTable.contains({ "4", "8" }));
    REQUIRE(nextTTable.contains({ "4", "9" }));
    REQUIRE(nextTTable.contains({ "4", "10" }));
    REQUIRE(nextTTable.contains({ "4", "11" }));
    REQUIRE(nextTTable.contains({ "4", "12" }));
    REQUIRE(nextTTable.contains({ "5", "3" }));
    REQUIRE(nextTTable.contains({ "5", "4" }));
    REQUIRE(nextTTable.contains({ "5", "5" }));
    REQUIRE(nextTTable.contains({ "5", "7" }));
    REQUIRE(nextTTable.contains({ "5", "8" }));
    REQUIRE(nextTTable.contains({ "5", "9" }));
    REQUIRE(nextTTable.contains({ "5", "10" }));
    REQUIRE(nextTTable.contains({ "5", "11" }));
    REQUIRE(nextTTable.contains({ "5", "12" }));
    REQUIRE(nextTTable.contains({ "6", "4" }));
    REQUIRE(nextTTable.contains({ "6", "5" }));
    REQUIRE(nextTTable.contains({ "6", "6" }));
    REQUIRE(nextTTable.contains({ "6", "7" }));
    REQUIRE(nextTTable.contains({ "6", "8" }));
    REQUIRE(nextTTable.contains({ "6", "9" }));
    REQUIRE(nextTTable.contains({ "6", "10" }));
    REQUIRE(nextTTable.contains({ "6", "11" }));
    REQUIRE(nextTTable.contains({ "6", "12" }));
    REQUIRE(nextTTable.contains({ "7", "10" }));
    REQUIRE(nextTTable.contains({ "7", "11" }));
    REQUIRE(nextTTable.contains({ "7", "12" }));
    REQUIRE(nextTTable.contains({ "8", "11" }));
    REQUIRE(nextTTable.contains({ "8", "12" }));
    REQUIRE(nextTTable.contains({ "9", "11" }));
    REQUIRE(nextTTable.contains({ "9", "12" }));
    REQUIRE(nextTTable.contains({ "10", "12" }));
    REQUIRE(nextTTable.size() == 75);
  }
}

TEST_CASE("[TestSimpleParser] Next relation - If within while", "[SimpleParser][Next]") {
  std::string string("procedure a{x=0;while(c<d){print z;if(3==3)then{call=assign;}else{while=read;}}read f;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "2", "7" }));
    REQUIRE(nextTable.contains({ "3", "4" }));
    REQUIRE(nextTable.contains({ "4", "5" }));
    REQUIRE(nextTable.contains({ "4", "6" }));
    REQUIRE(nextTable.contains({ "5", "2" }));
    REQUIRE(nextTable.contains({ "6", "2" }));
    REQUIRE(nextTable.size() == 8);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "1", "5" }));
    REQUIRE(nextTTable.contains({ "1", "6" }));
    REQUIRE(nextTTable.contains({ "1", "7" }));
    REQUIRE(nextTTable.contains({ "2", "2" }));
    REQUIRE(nextTTable.contains({ "2", "4" }));
    REQUIRE(nextTTable.contains({ "2", "5" }));
    REQUIRE(nextTTable.contains({ "2", "6" }));
    REQUIRE(nextTTable.contains({ "3", "2" }));
    REQUIRE(nextTTable.contains({ "3", "3" }));
    REQUIRE(nextTTable.contains({ "3", "5" }));
    REQUIRE(nextTTable.contains({ "3", "6" }));
    REQUIRE(nextTTable.contains({ "3", "7" }));
    REQUIRE(nextTTable.contains({ "4", "2" }));
    REQUIRE(nextTTable.contains({ "4", "3" }));
    REQUIRE(nextTTable.contains({ "4", "4" }));
    REQUIRE(nextTTable.contains({ "4", "7" }));
    REQUIRE(nextTTable.contains({ "5", "3" }));
    REQUIRE(nextTTable.contains({ "5", "4" }));
    REQUIRE(nextTTable.contains({ "5", "5" }));
    REQUIRE(nextTTable.contains({ "5", "6" }));
    REQUIRE(nextTTable.contains({ "5", "7" }));
    REQUIRE(nextTTable.contains({ "6", "3" }));
    REQUIRE(nextTTable.contains({ "6", "4" }));
    REQUIRE(nextTTable.contains({ "6", "5" }));
    REQUIRE(nextTTable.contains({ "6", "6" }));
    REQUIRE(nextTTable.contains({ "6", "7" }));
    REQUIRE(nextTTable.size() == 36);
  }
}

TEST_CASE("[TestSimpleParser] Next relation - While within while", "[SimpleParser][Next]") {
  std::string string("procedure a{while(b<b){while(c<d){read while;print read;}}e=1;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "1", "5" }));
    REQUIRE(nextTable.contains({ "2", "1" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "3", "4" }));
    REQUIRE(nextTable.contains({ "4", "2" }));
    REQUIRE(nextTable.size() == 6);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "1" }));
    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "2", "2" }));
    REQUIRE(nextTTable.contains({ "2", "4" }));
    REQUIRE(nextTTable.contains({ "2", "5" }));
    REQUIRE(nextTTable.contains({ "3", "1" }));
    REQUIRE(nextTTable.contains({ "3", "2" }));
    REQUIRE(nextTTable.contains({ "3", "3" }));
    REQUIRE(nextTTable.contains({ "3", "5" }));
    REQUIRE(nextTTable.contains({ "4", "1" }));
    REQUIRE(nextTTable.contains({ "4", "3" }));
    REQUIRE(nextTTable.contains({ "4", "4" }));
    REQUIRE(nextTTable.contains({ "4", "5" }));
    REQUIRE(nextTTable.size() == 20);
  }
}

TEST_CASE("[TestSimpleParser] Next relation - If within if", "[SimpleParser][Next]") {
  std::string string("procedure a{if(b>c)then{if(d==e)then{read f;}else{print g;}}else{if(h!=i)then{j=3;}else{k=l+m;}}a=1;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "1", "5" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "2", "4" }));
    REQUIRE(nextTable.contains({ "3", "8" }));
    REQUIRE(nextTable.contains({ "4", "8" }));
    REQUIRE(nextTable.contains({ "5", "6" }));
    REQUIRE(nextTable.contains({ "5", "7" }));
    REQUIRE(nextTable.contains({ "6", "8" }));
    REQUIRE(nextTable.contains({ "7", "8" }));
    REQUIRE(nextTable.size() == 10);
  }

  SECTION("NextT") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "1", "6" }));
    REQUIRE(nextTTable.contains({ "1", "7" }));
    REQUIRE(nextTTable.contains({ "1", "8" }));
    REQUIRE(nextTTable.contains({ "2", "8" }));
    REQUIRE(nextTTable.contains({ "5", "8" }));
    REQUIRE(nextTTable.size() == 17);
  }
}

TEST_CASE("[TestSimpleParser] Next relation - While within if", "[SimpleParser][Next]") {
  std::string string("procedure a{if(2>e)then{while(c>=3){z=1;}}else{while(f>=3){k=1;}}a=1;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("Next") {
    Table nextTable = pkb.getNextTable();

    REQUIRE(nextTable.contains({ "1", "2" }));
    REQUIRE(nextTable.contains({ "1", "4" }));
    REQUIRE(nextTable.contains({ "2", "3" }));
    REQUIRE(nextTable.contains({ "2", "6" }));
    REQUIRE(nextTable.contains({ "3", "2" }));
    REQUIRE(nextTable.contains({ "4", "5" }));
    REQUIRE(nextTable.contains({ "4", "6" }));
    REQUIRE(nextTable.contains({ "5", "4" }));
    REQUIRE(nextTable.size() == 8);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ "1", "3" }));
    REQUIRE(nextTTable.contains({ "1", "4" }));
    REQUIRE(nextTTable.contains({ "1", "5" }));
    REQUIRE(nextTTable.contains({ "1", "6" }));
    REQUIRE(nextTTable.contains({ "2", "2" }));
    REQUIRE(nextTTable.contains({ "3", "6" }));
    REQUIRE(nextTTable.contains({ "4", "4" }));
    REQUIRE(nextTTable.contains({ "5", "5" }));
    REQUIRE(nextTTable.contains({ "5", "6" }));
    REQUIRE(nextTTable.size() == 17);
  }
}

////////////////////
// CFG and CFGBip //
////////////////////

TEST_CASE("[TestSimpleParser] CFG - Example CFG (Figure 5) - Advanced SPA requirements", "[SimpleParser][Cfg][CfgBip]") {
  std::string string("procedure Second{x=0;i=5;while(i!=0){x=x+2*y;call Third;i=i-1;}if(x==1)then{x=x+1;}else{z=1;}z=z+x+i;y=z+2;x=x*y+z;}procedure Third{z=5;v=z;print v;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("CFG") {
    std::vector<int> stmt1 = pkb.getNextStmtsFromCfg(1);
    std::vector<int> stmt2 = pkb.getNextStmtsFromCfg(2);
    std::vector<int> stmt3 = pkb.getNextStmtsFromCfg(3);
    std::vector<int> stmt4 = pkb.getNextStmtsFromCfg(4);
    std::vector<int> stmt5 = pkb.getNextStmtsFromCfg(5);
    std::vector<int> stmt6 = pkb.getNextStmtsFromCfg(6);
    std::vector<int> stmt7 = pkb.getNextStmtsFromCfg(7);
    std::vector<int> stmt8 = pkb.getNextStmtsFromCfg(8);
    std::vector<int> stmt9 = pkb.getNextStmtsFromCfg(9);
    std::vector<int> stmt10 = pkb.getNextStmtsFromCfg(10);
    std::vector<int> stmt11 = pkb.getNextStmtsFromCfg(11);
    std::vector<int> stmt12 = pkb.getNextStmtsFromCfg(12);

    REQUIRE(std::find(stmt1.begin(), stmt1.end(), 2) != stmt1.end());
    REQUIRE(stmt1.size() == 1);
    REQUIRE(std::find(stmt2.begin(), stmt2.end(), 3) != stmt2.end());
    REQUIRE(stmt2.size() == 1);
    REQUIRE(std::find(stmt3.begin(), stmt3.end(), 4) != stmt3.end());
    REQUIRE(std::find(stmt3.begin(), stmt3.end(), 7) != stmt3.end());
    REQUIRE(stmt3.size() == 2);
    REQUIRE(std::find(stmt4.begin(), stmt4.end(), 5) != stmt4.end());
    REQUIRE(stmt4.size() == 1);
    REQUIRE(std::find(stmt5.begin(), stmt5.end(), 6) != stmt5.end());
    REQUIRE(stmt5.size() == 1);
    REQUIRE(std::find(stmt6.begin(), stmt6.end(), 3) != stmt6.end());
    REQUIRE(stmt6.size() == 1);
    REQUIRE(std::find(stmt7.begin(), stmt7.end(), 8) != stmt7.end());
    REQUIRE(std::find(stmt7.begin(), stmt7.end(), 9) != stmt7.end());
    REQUIRE(stmt7.size() == 2);
    REQUIRE(std::find(stmt8.begin(), stmt8.end(), 10) != stmt8.end());
    REQUIRE(stmt8.size() == 1);
    REQUIRE(std::find(stmt9.begin(), stmt9.end(), 10) != stmt9.end());
    REQUIRE(stmt9.size() == 1);
    REQUIRE(std::find(stmt10.begin(), stmt10.end(), 11) != stmt10.end());
    REQUIRE(stmt10.size() == 1);
    REQUIRE(std::find(stmt11.begin(), stmt11.end(), 12) != stmt11.end());
    REQUIRE(stmt11.size() == 1);
    REQUIRE(stmt12.size() == 0);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("CFGBip") {
    std::vector<Cfg::BipNode> bipStmt1 = pkb.getNextStmtsFromCfgBip(1);
    std::vector<Cfg::BipNode> bipStmt2 = pkb.getNextStmtsFromCfgBip(2);
    std::vector<Cfg::BipNode> bipStmt3 = pkb.getNextStmtsFromCfgBip(3);
    std::vector<Cfg::BipNode> bipStmt4 = pkb.getNextStmtsFromCfgBip(4);
    std::vector<Cfg::BipNode> bipStmt5 = pkb.getNextStmtsFromCfgBip(5);
    std::vector<Cfg::BipNode> bipStmt6 = pkb.getNextStmtsFromCfgBip(6);
    std::vector<Cfg::BipNode> bipStmt7 = pkb.getNextStmtsFromCfgBip(7);
    std::vector<Cfg::BipNode> bipStmt8 = pkb.getNextStmtsFromCfgBip(8);
    std::vector<Cfg::BipNode> bipStmt9 = pkb.getNextStmtsFromCfgBip(9);
    std::vector<Cfg::BipNode> bipStmt10 = pkb.getNextStmtsFromCfgBip(10);
    std::vector<Cfg::BipNode> bipStmt11 = pkb.getNextStmtsFromCfgBip(11);
    std::vector<Cfg::BipNode> bipStmt12 = pkb.getNextStmtsFromCfgBip(12);
    std::vector<Cfg::BipNode> bipStmt13 = pkb.getNextStmtsFromCfgBip(13);
    std::vector<Cfg::BipNode> bipStmt14 = pkb.getNextStmtsFromCfgBip(14);
    std::vector<Cfg::BipNode> bipStmt15 = pkb.getNextStmtsFromCfgBip(15);
    std::vector<Cfg::BipNode> dummy1 = pkb.getNextStmtsFromCfgBip(-1);
    std::vector<Cfg::BipNode> dummy13 = pkb.getNextStmtsFromCfgBip(-13);

    REQUIRE(bipStmt1[0].node == 2);
    REQUIRE(bipStmt1[0].label == 0);
    REQUIRE(bipStmt1[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt1.size() == 1);
    REQUIRE(bipStmt2[0].node == 3);
    REQUIRE(bipStmt2[0].label == 0);
    REQUIRE(bipStmt2[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt2.size() == 1);
    REQUIRE(bipStmt3[0].node == 4);
    REQUIRE(bipStmt3[0].label == 0);
    REQUIRE(bipStmt3[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt3[1].node == 7);
    REQUIRE(bipStmt3[1].label == 0);
    REQUIRE(bipStmt3[1].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt3.size() == 2);
    REQUIRE(bipStmt4[0].node == 5);
    REQUIRE(bipStmt4[0].label == 0);
    REQUIRE(bipStmt4[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt4.size() == 1);
    REQUIRE(bipStmt5[0].node == 13);
    REQUIRE(bipStmt5[0].label == 5);
    REQUIRE(bipStmt5[0].type == Cfg::NodeType::BRANCH_IN);
    REQUIRE(bipStmt5.size() == 1);
    REQUIRE(bipStmt6[0].node == 3);
    REQUIRE(bipStmt6[0].label == 0);
    REQUIRE(bipStmt6[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt6.size() == 1);
    REQUIRE(bipStmt7[0].node == 8);
    REQUIRE(bipStmt7[0].label == 0);
    REQUIRE(bipStmt7[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt7[1].node == 9);
    REQUIRE(bipStmt7[1].label == 0);
    REQUIRE(bipStmt7[1].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt7.size() == 2);
    REQUIRE(bipStmt8[0].node == 10);
    REQUIRE(bipStmt8[0].label == 0);
    REQUIRE(bipStmt8[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt8.size() == 1);
    REQUIRE(bipStmt9[0].node == 10);
    REQUIRE(bipStmt9[0].label == 0);
    REQUIRE(bipStmt9[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt9.size() == 1);
    REQUIRE(bipStmt10[0].node == 11);
    REQUIRE(bipStmt10[0].label == 0);
    REQUIRE(bipStmt10[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt10.size() == 1);
    REQUIRE(bipStmt11[0].node == 12);
    REQUIRE(bipStmt11[0].label == 0);
    REQUIRE(bipStmt11[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt11.size() == 1);
    REQUIRE(bipStmt12[0].node == -1);
    REQUIRE(bipStmt12[0].label == 0);
    REQUIRE(bipStmt12[0].type == Cfg::NodeType::DUMMY);
    REQUIRE(bipStmt12.size() == 1);
    REQUIRE(bipStmt13[0].node == 14);
    REQUIRE(bipStmt13[0].label == 0);
    REQUIRE(bipStmt13[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt13.size() == 1);
    REQUIRE(bipStmt14[0].node == 15);
    REQUIRE(bipStmt14[0].label == 0);
    REQUIRE(bipStmt14[0].type == Cfg::NodeType::NORMAL);
    REQUIRE(bipStmt14.size() == 1);
    REQUIRE(bipStmt15[0].node == -13);
    REQUIRE(bipStmt15[0].label == 0);
    REQUIRE(bipStmt15[0].type == Cfg::NodeType::DUMMY);
    REQUIRE(bipStmt15.size() == 1);
    REQUIRE(dummy1.size() == 0);
    REQUIRE(dummy13[0].node == 6);
    REQUIRE(dummy13[0].label == 5);
    REQUIRE(dummy13[0].type == Cfg::NodeType::BRANCH_BACK);
    REQUIRE(dummy13.size() == 1);
  }
}

///////////////////////////////////
// Affects and AffectsT relation //
///////////////////////////////////

TEST_CASE("[TestSimpleParser] Affects relation - Example CFG (Figure 5) - Advanced SPA requirements", "[SimpleParser][Affects]") {
  std::string string("procedure Second{x=0;i=5;while(i!=0){x=x+2*y;call Third;i=i-1;}if(x==1)then{x=x+1;}else{z=1;}z=z+x+i;y=z+2;x=x*y+z;}procedure Third{z=5;v=z;print v;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.contains({ "1", "4" }));
    REQUIRE(affectsTable.contains({ "1", "8" }));
    REQUIRE(affectsTable.contains({ "1", "10" }));
    REQUIRE(affectsTable.contains({ "1", "12" }));
    REQUIRE(affectsTable.contains({ "2", "6" }));
    REQUIRE(affectsTable.contains({ "2", "10" }));
    REQUIRE(affectsTable.contains({ "4", "4" }));
    REQUIRE(affectsTable.contains({ "4", "8" }));
    REQUIRE(affectsTable.contains({ "4", "10" }));
    REQUIRE(affectsTable.contains({ "4", "12" }));
    REQUIRE(affectsTable.contains({ "6", "6" }));
    REQUIRE(affectsTable.contains({ "6", "10" }));
    REQUIRE(affectsTable.contains({ "8", "10" }));
    REQUIRE(affectsTable.contains({ "8", "12" }));
    REQUIRE(affectsTable.contains({ "9", "10" }));
    REQUIRE(affectsTable.contains({ "10", "11" }));
    REQUIRE(affectsTable.contains({ "10", "12" }));
    REQUIRE(affectsTable.contains({ "11", "12" }));
    REQUIRE(affectsTable.contains({ "13", "14" }));
    REQUIRE(affectsTable.size() == 19);
  }

  SECTION("AffectsT") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.contains({ "1", "11" }));
    REQUIRE(affectsTTable.contains({ "2", "11" }));
    REQUIRE(affectsTTable.contains({ "2", "12" }));
    REQUIRE(affectsTTable.contains({ "4", "11" }));
    REQUIRE(affectsTTable.contains({ "6", "11" }));
    REQUIRE(affectsTTable.contains({ "6", "12" }));
    REQUIRE(affectsTTable.contains({ "8", "11" }));
    REQUIRE(affectsTTable.contains({ "9", "11" }));
    REQUIRE(affectsTTable.contains({ "9", "12" }));
    REQUIRE(affectsTTable.size() == 28);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Call proc directly modifies target var", "[SimpleParser][Affects]") {
  std::string string("procedure first{x=0;call second;i=x-1;}procedure second{read x;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects", "[SimpleParser][Affects]") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.size() == 0);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.size() == 0);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Call proc indirectly modifies target var", "[SimpleParser][Affects]") {
  std::string string("procedure first{x=0;call second;i=x-1;}procedure second{print y;call third;}procedure third{x=3;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects", "[SimpleParser][Affects]") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.size() == 0);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.size() == 0);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Container statement - Advanced SPA requirements", "[SimpleParser][Affects]") {
  std::string string("procedure alpha{x=1;if(i!=2)then{x=a+1;}else{a=b;}a=x;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.contains({ "1", "5" }));
    REQUIRE(affectsTable.contains({ "3", "5" }));
    REQUIRE(affectsTable.size() == 2);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.size() == 2);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Procedure call - Advanced SPA requirements", "[SimpleParser][Affects]") {
  std::string string("procedure alpha{x=1;call beta;a=x;}procedure beta{if(i!=2)then{x=a+1;}else{a=b;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.size() == 0);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.size() == 0);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Advanced SPA in class quiz", "[SimpleParser][Affects]") {
  std::string string(R"(
      procedure Bumblebee{read x;y=2*z;call Megatron;z=p-y;
      if(q!=1)then{z=x+24;call Ironhide;}else{while(z>0){y=x*q-5;z=z-1;}y=x+y*z+p*q;call Ironhide;}i=x+j+z;call Barricade;}
      procedure Megatron{p=x*10;q=y+y*10;while(t==0){a=5*p-5;print y;}q=p*5;}
      procedure Ironhide{i=x*10;j=x+y*10;if(i>j)then{read x;call Barricade;}else{y=i+x+y*z;}}
      procedure Barricade{q=i*x*j+y+p*q;print q;})"); 
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.contains({ "2", "4" }));
    REQUIRE(affectsTable.contains({ "2", "11" }));
    REQUIRE(affectsTable.contains({ "4", "10" }));
    REQUIRE(affectsTable.contains({ "4", "11" }));
    REQUIRE(affectsTable.contains({ "4", "13" }));
    REQUIRE(affectsTable.contains({ "6", "13" }));
    REQUIRE(affectsTable.contains({ "9", "11" }));
    REQUIRE(affectsTable.contains({ "10", "10" }));
    REQUIRE(affectsTable.contains({ "10", "11" }));
    REQUIRE(affectsTable.contains({ "10", "13" }));
    REQUIRE(affectsTable.contains({ "15", "18" }));
    REQUIRE(affectsTable.contains({ "15", "20" }));
    REQUIRE(affectsTable.contains({ "21", "26" }));
    REQUIRE(affectsTable.size() == 13);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.contains({ "2", "10" }));
    REQUIRE(affectsTTable.contains({ "2", "13" }));
    REQUIRE(affectsTTable.size() == 15);
  }
}

TEST_CASE("[TestSimpleParser] Affects relation - Week 7 lecture quiz", "[SimpleParser][Affects]") {
  std::string string(R"(
      procedure Gandalf{read shine;ring=shine*2;f=ring+3;read lost;call Frodo;
      if(lost==shine)then{call Aragon;d=(shine+12)*(lost+13);}else{while(f>10){read g;f=ring+4+g*a*2*1+shine;call Frodo;}d=2;}print d;}
      procedure Frodo{a=f+shine;if(a<2)then{call Aragon;}else{f=f+19;}}
      procedure Aragon{while(shine<3){a=1*2+lost-f;while(lost==a){lost=lost-1;w=a+lost;shine=1+ring;}if(a!=shine)then{a=lost+1;w=shine*2;}else {w=1;}}})");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();
  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("Affects") {
    Table affectsTable = pkb.getAffectsTable();

    REQUIRE(affectsTable.contains({ "2", "3" }));
    REQUIRE(affectsTable.contains({ "2", "11" }));
    REQUIRE(affectsTable.contains({ "20", "23" }));
    REQUIRE(affectsTable.contains({ "22", "20" }));
    REQUIRE(affectsTable.contains({ "22", "22" }));
    REQUIRE(affectsTable.contains({ "22", "23" }));
    REQUIRE(affectsTable.contains({ "22", "26" }));
    REQUIRE(affectsTable.contains({ "24", "27" }));
    REQUIRE(affectsTable.size() == 8);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.size() == 8);
  }
}

///////////////////////////////////
// NextBip and NextBipT relation //
///////////////////////////////////

TEST_CASE("[TestSimpleParser] NextBip relation - Example 1 - Project iteration 2 and 3", "[SimpleParser][NextBip]") {
  std::string string("procedure Bill{x=5;call Mary;y=x+6;x=5;z=x*y+2;}procedure Mary{y=x*3;call John;z=x+y;}procedure John{if(i>0)then{x=x+z;}else{y=x*y;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("NextBip - Before Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ "1", "2" }));
    REQUIRE(nextBipTable.contains({ "3", "4" }));
    REQUIRE(nextBipTable.contains({ "4", "5" }));
    REQUIRE(nextBipTable.contains({ "6", "7" }));
    REQUIRE(nextBipTable.contains({ "9", "10" }));
    REQUIRE(nextBipTable.contains({ "9", "11" }));
    REQUIRE(nextBipTable.size() == 6);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextBip - After Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ "2", "6" }));
    REQUIRE(nextBipTable.contains({ "7", "9" }));
    REQUIRE(nextBipTable.contains({ "8", "3" }));
    REQUIRE(nextBipTable.contains({ "10", "8" }));
    REQUIRE(nextBipTable.contains({ "11", "8" }));
    REQUIRE(nextBipTable.size() == 11);
  }

  SECTION("NextBipT") {
    Table nextBipTTable = pkb.getNextBipTTable();

    REQUIRE(nextBipTTable.contains({ "1", "2" }));
    REQUIRE(nextBipTTable.contains({ "1", "6" }));
    REQUIRE(nextBipTTable.contains({ "1", "7" }));
    REQUIRE(nextBipTTable.contains({ "1", "9" }));
    REQUIRE(nextBipTTable.contains({ "1", "10" }));
    REQUIRE(nextBipTTable.contains({ "1", "11" }));
    REQUIRE(nextBipTTable.contains({ "1", "8" }));
    REQUIRE(nextBipTTable.contains({ "1", "3" }));
    REQUIRE(nextBipTTable.contains({ "1", "4" }));
    REQUIRE(nextBipTTable.contains({ "1", "5" }));
    REQUIRE(nextBipTTable.contains({ "2", "6" }));
    REQUIRE(nextBipTTable.contains({ "2", "7" }));
    REQUIRE(nextBipTTable.contains({ "2", "9" }));
    REQUIRE(nextBipTTable.contains({ "2", "10" }));
    REQUIRE(nextBipTTable.contains({ "2", "11" }));
    REQUIRE(nextBipTTable.contains({ "2", "8" }));
    REQUIRE(nextBipTTable.contains({ "2", "3" }));
    REQUIRE(nextBipTTable.contains({ "2", "4" }));
    REQUIRE(nextBipTTable.contains({ "2", "5" }));
    REQUIRE(nextBipTTable.contains({ "3", "4" }));
    REQUIRE(nextBipTTable.contains({ "3", "5" }));
    REQUIRE(nextBipTTable.contains({ "4", "5" }));
    REQUIRE(nextBipTTable.contains({ "6", "7" }));
    REQUIRE(nextBipTTable.contains({ "6", "9" }));
    REQUIRE(nextBipTTable.contains({ "6", "10" }));
    REQUIRE(nextBipTTable.contains({ "6", "11" }));
    REQUIRE(nextBipTTable.contains({ "6", "8" }));
    REQUIRE(nextBipTTable.contains({ "6", "3" }));
    REQUIRE(nextBipTTable.contains({ "6", "4" }));
    REQUIRE(nextBipTTable.contains({ "6", "5" }));
    REQUIRE(nextBipTTable.contains({ "7", "9" }));
    REQUIRE(nextBipTTable.contains({ "7", "10" }));
    REQUIRE(nextBipTTable.contains({ "7", "11" }));
    REQUIRE(nextBipTTable.contains({ "7", "8" }));
    REQUIRE(nextBipTTable.contains({ "7", "3" }));
    REQUIRE(nextBipTTable.contains({ "7", "4" }));
    REQUIRE(nextBipTTable.contains({ "7", "5" }));
    REQUIRE(nextBipTTable.contains({ "8", "3" }));
    REQUIRE(nextBipTTable.contains({ "8", "4" }));
    REQUIRE(nextBipTTable.contains({ "8", "5" }));
    REQUIRE(nextBipTTable.contains({ "9", "10" }));
    REQUIRE(nextBipTTable.contains({ "9", "11" }));
    REQUIRE(nextBipTTable.contains({ "9", "8" }));
    REQUIRE(nextBipTTable.contains({ "9", "3" }));
    REQUIRE(nextBipTTable.contains({ "9", "4" }));
    REQUIRE(nextBipTTable.contains({ "9", "5" }));
    REQUIRE(nextBipTTable.contains({ "10", "8" }));
    REQUIRE(nextBipTTable.contains({ "10", "3" }));
    REQUIRE(nextBipTTable.contains({ "10", "4" }));
    REQUIRE(nextBipTTable.contains({ "10", "5" }));
    REQUIRE(nextBipTTable.contains({ "11", "8" }));
    REQUIRE(nextBipTTable.contains({ "11", "3" }));
    REQUIRE(nextBipTTable.contains({ "11", "4" }));
    REQUIRE(nextBipTTable.contains({ "11", "5" }));
    REQUIRE(nextBipTTable.size() == 54);
  }
}

TEST_CASE("[TestSimpleParser] NextBip relation - Example 2 - Project iteration 2 and 3", "[SimpleParser][NextBip]") {
  std::string string("procedure Bill{x=5;call Mary;y=x+6;call John;z=x*y+2;}procedure Mary{y=x*3;call John;z=x+y;}procedure John{if(i>0)then{x=x+z;}else{y=x*y;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SECTION("NextBip - Before Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ "1", "2" }));
    REQUIRE(nextBipTable.contains({ "3", "4" }));
    REQUIRE(nextBipTable.contains({ "6", "7" }));
    REQUIRE(nextBipTable.contains({ "9", "10" }));
    REQUIRE(nextBipTable.contains({ "9", "11" }));
    REQUIRE(nextBipTable.size() == 5);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextBip - After Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ "2", "6" }));
    REQUIRE(nextBipTable.contains({ "4", "9" }));
    REQUIRE(nextBipTable.contains({ "7", "9" }));
    REQUIRE(nextBipTable.contains({ "8", "3" }));
    REQUIRE(nextBipTable.contains({ "10", "5" }));
    REQUIRE(nextBipTable.contains({ "11", "5" }));
    REQUIRE(nextBipTable.contains({ "10", "8" }));
    REQUIRE(nextBipTable.contains({ "11", "8" }));
    REQUIRE(nextBipTable.size() == 13);
  }

  SECTION("NextBipT") {
    Table nextBipTTable = pkb.getNextBipTTable();

    REQUIRE(nextBipTTable.contains({ "1", "2" }));
    REQUIRE(nextBipTTable.contains({ "1", "6" }));
    REQUIRE(nextBipTTable.contains({ "1", "7" }));
    REQUIRE(nextBipTTable.contains({ "1", "9" }));
    REQUIRE(nextBipTTable.contains({ "1", "10" }));
    REQUIRE(nextBipTTable.contains({ "1", "11" }));
    REQUIRE(nextBipTTable.contains({ "1", "8" }));
    REQUIRE(nextBipTTable.contains({ "1", "3" }));
    REQUIRE(nextBipTTable.contains({ "1", "4" }));
    REQUIRE(nextBipTTable.contains({ "1", "5" }));
    REQUIRE(nextBipTTable.contains({ "2", "6" }));
    REQUIRE(nextBipTTable.contains({ "2", "7" }));
    REQUIRE(nextBipTTable.contains({ "2", "9" }));
    REQUIRE(nextBipTTable.contains({ "2", "10" }));
    REQUIRE(nextBipTTable.contains({ "2", "11" }));
    REQUIRE(nextBipTTable.contains({ "2", "8" }));
    REQUIRE(nextBipTTable.contains({ "2", "3" }));
    REQUIRE(nextBipTTable.contains({ "2", "4" }));
    REQUIRE(nextBipTTable.contains({ "2", "5" }));
    REQUIRE(nextBipTTable.contains({ "3", "4" }));
    REQUIRE(nextBipTTable.contains({ "3", "9" }));
    REQUIRE(nextBipTTable.contains({ "3", "10" }));
    REQUIRE(nextBipTTable.contains({ "3", "11" }));
    REQUIRE(nextBipTTable.contains({ "3", "5" }));
    REQUIRE(nextBipTTable.contains({ "4", "9" }));
    REQUIRE(nextBipTTable.contains({ "4", "10" }));
    REQUIRE(nextBipTTable.contains({ "4", "11" }));
    REQUIRE(nextBipTTable.contains({ "4", "5" }));
    REQUIRE(nextBipTTable.contains({ "6", "7" }));
    REQUIRE(nextBipTTable.contains({ "6", "9" }));
    REQUIRE(nextBipTTable.contains({ "6", "10" }));
    REQUIRE(nextBipTTable.contains({ "6", "11" }));
    REQUIRE(nextBipTTable.contains({ "6", "8" }));
    REQUIRE(nextBipTTable.contains({ "6", "3" }));
    REQUIRE(nextBipTTable.contains({ "6", "4" }));
    REQUIRE(nextBipTTable.contains({ "6", "5" }));
    REQUIRE(nextBipTTable.contains({ "7", "9" }));
    REQUIRE(nextBipTTable.contains({ "7", "10" }));
    REQUIRE(nextBipTTable.contains({ "7", "11" }));
    REQUIRE(nextBipTTable.contains({ "7", "8" }));
    REQUIRE(nextBipTTable.contains({ "7", "3" }));
    REQUIRE(nextBipTTable.contains({ "7", "4" }));
    REQUIRE(nextBipTTable.contains({ "7", "5" }));
    REQUIRE(nextBipTTable.contains({ "8", "3" }));
    REQUIRE(nextBipTTable.contains({ "8", "4" }));
    REQUIRE(nextBipTTable.contains({ "8", "9" }));
    REQUIRE(nextBipTTable.contains({ "8", "10" }));
    REQUIRE(nextBipTTable.contains({ "8", "11" }));
    REQUIRE(nextBipTTable.contains({ "8", "5" }));
    REQUIRE(nextBipTTable.contains({ "9", "10" }));
    REQUIRE(nextBipTTable.contains({ "9", "11" }));
    REQUIRE(nextBipTTable.contains({ "9", "8" }));
    REQUIRE(nextBipTTable.contains({ "9", "3" }));
    REQUIRE(nextBipTTable.contains({ "9", "4" }));
    REQUIRE(nextBipTTable.contains({ "9", "9" }));
    REQUIRE(nextBipTTable.contains({ "9", "5" }));
    REQUIRE(nextBipTTable.contains({ "10", "8" }));
    REQUIRE(nextBipTTable.contains({ "10", "3" }));
    REQUIRE(nextBipTTable.contains({ "10", "4" }));
    REQUIRE(nextBipTTable.contains({ "10", "9" }));
    REQUIRE(nextBipTTable.contains({ "10", "10" }));
    REQUIRE(nextBipTTable.contains({ "10", "11" }));
    REQUIRE(nextBipTTable.contains({ "10", "5" }));
    REQUIRE(nextBipTTable.contains({ "11", "8" }));
    REQUIRE(nextBipTTable.contains({ "11", "3" }));
    REQUIRE(nextBipTTable.contains({ "11", "4" }));
    REQUIRE(nextBipTTable.contains({ "11", "9" }));
    REQUIRE(nextBipTTable.contains({ "11", "10" }));
    REQUIRE(nextBipTTable.contains({ "11", "11" }));
    REQUIRE(nextBipTTable.contains({ "11", "5" }));
    REQUIRE(nextBipTTable.size() == 70);
  }
}

/////////////////////////
// AffectsBip relation //
/////////////////////////

TEST_CASE("[TestSimpleParser] AffectsBip relation - Call proc directly modifies target var", "[SimpleParser][AffectsBip]") {
  std::string string("procedure first{x=0;call second;i=x-1;}procedure second{read x;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.size() == 0);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Call proc indirectly modifies target var", "[SimpleParser][AffectsBip]") {
  std::string string("procedure first{x=0;call second;i=x-1;}procedure second{print y;call third;}procedure third{x=3;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "6", "3" }));
  REQUIRE(affectsBipTable.size() == 1);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Container statement - Advanced SPA requirements", "[SimpleParser][AffectsBip]") {
  std::string string("procedure alpha{x=1;if(i!=2)then{x=a+1;}else{a=b;}a=x;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "1", "5" }));
  REQUIRE(affectsBipTable.contains({ "3", "5" }));
  REQUIRE(affectsBipTable.size() == 2);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Procedure call - Advanced SPA requirements", "[SimpleParser][AffectsBip]") {
  std::string string("procedure alpha{x=1;call beta;a=x+a;}procedure beta{if(i!=2)then{x=a+1;}else{a=b;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "1", "3" }));
  REQUIRE(affectsBipTable.contains({ "5", "3" }));
  REQUIRE(affectsBipTable.contains({ "6", "3" }));
  REQUIRE(affectsBipTable.size() == 3);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Example CFG (Figure 5) - Advanced SPA requirements", "[SimpleParser][AffectsBip]") {
  std::string string("procedure Second{x=0;i=5;while(i!=0){x=x+2*y;call Third;i=i-1;}if(x==1)then{x=x+1;}else{z=1;}z=z+x+i;y=z+2;x=x*y+z;}procedure Third{z=5;v=z;print v;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "1", "4" }));
  REQUIRE(affectsBipTable.contains({ "1", "8" }));
  REQUIRE(affectsBipTable.contains({ "1", "10" }));
  REQUIRE(affectsBipTable.contains({ "1", "12" }));
  REQUIRE(affectsBipTable.contains({ "2", "6" }));
  REQUIRE(affectsBipTable.contains({ "2", "10" }));
  REQUIRE(affectsBipTable.contains({ "4", "4" }));
  REQUIRE(affectsBipTable.contains({ "4", "8" }));
  REQUIRE(affectsBipTable.contains({ "4", "10" }));
  REQUIRE(affectsBipTable.contains({ "4", "12" }));
  REQUIRE(affectsBipTable.contains({ "6", "6" }));
  REQUIRE(affectsBipTable.contains({ "6", "10" }));
  REQUIRE(affectsBipTable.contains({ "8", "10" }));
  REQUIRE(affectsBipTable.contains({ "8", "12" }));
  REQUIRE(affectsBipTable.contains({ "9", "10" }));
  REQUIRE(affectsBipTable.contains({ "10", "11" }));
  REQUIRE(affectsBipTable.contains({ "10", "12" }));
  REQUIRE(affectsBipTable.contains({ "11", "12" }));
  REQUIRE(affectsBipTable.contains({ "13", "10" }));
  REQUIRE(affectsBipTable.contains({ "13", "14" }));
  REQUIRE(affectsBipTable.size() == 20);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Advanced SPA in class quiz", "[SimpleParser][AffectsBip]") {
  std::string string(R"(
    procedure Bumblebee{read x;y=2*z;call Megatron;z=p-y;
    if(q!=1)then{z=x+24;call Ironhide;}else{while(z>0){y=x*q-5;z=z-1;}y=x+y*z+p*q;call Ironhide;}i=x+j+z;call Barricade;}
    procedure Megatron{p=x*10;q=y+y*10;while(t==0){a=5*p-5;print y;}q=p*5;}
    procedure Ironhide{i=x*10;j=x+y*10;if(i>j)then{read x;call Barricade;}else{y=i+x+y*z;}}
    procedure Barricade{q=i*x*j+y+p*q;print q;})");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "2", "4" }));
  REQUIRE(affectsBipTable.contains({ "2", "11" }));
  REQUIRE(affectsBipTable.contains({ "2", "16" }));
  REQUIRE(affectsBipTable.contains({ "2", "22" }));
  REQUIRE(affectsBipTable.contains({ "2", "26" }));
  REQUIRE(affectsBipTable.contains({ "2", "27" }));
  REQUIRE(affectsBipTable.contains({ "4", "10" }));
  REQUIRE(affectsBipTable.contains({ "4", "11" }));
  REQUIRE(affectsBipTable.contains({ "4", "13" }));
  REQUIRE(affectsBipTable.contains({ "4", "26" }));
  REQUIRE(affectsBipTable.contains({ "6", "13" }));
  REQUIRE(affectsBipTable.contains({ "6", "26" }));
  REQUIRE(affectsBipTable.contains({ "9", "11" }));
  REQUIRE(affectsBipTable.contains({ "10", "10" }));
  REQUIRE(affectsBipTable.contains({ "10", "11" }));
  REQUIRE(affectsBipTable.contains({ "10", "13" }));
  REQUIRE(affectsBipTable.contains({ "10", "26" }));
  REQUIRE(affectsBipTable.contains({ "11", "22" }));
  REQUIRE(affectsBipTable.contains({ "11", "26" }));
  REQUIRE(affectsBipTable.contains({ "11", "27" }));
  REQUIRE(affectsBipTable.contains({ "13", "27" }));
  REQUIRE(affectsBipTable.contains({ "15", "18" }));
  REQUIRE(affectsBipTable.contains({ "15", "20" }));
  REQUIRE(affectsBipTable.contains({ "15", "4" }));
  REQUIRE(affectsBipTable.contains({ "15", "11" }));
  REQUIRE(affectsBipTable.contains({ "15", "27" }));
  REQUIRE(affectsBipTable.contains({ "20", "9" }));
  REQUIRE(affectsBipTable.contains({ "20", "11" }));
  REQUIRE(affectsBipTable.contains({ "20", "27" }));
  REQUIRE(affectsBipTable.contains({ "21", "26" }));
  REQUIRE(affectsBipTable.contains({ "21", "27" }));
  REQUIRE(affectsBipTable.contains({ "22", "13" }));
  REQUIRE(affectsBipTable.contains({ "22", "27" }));
  REQUIRE(affectsBipTable.contains({ "26", "27" }));
  REQUIRE(affectsBipTable.contains({ "27", "27" }));
  REQUIRE(affectsBipTable.size() == 35);
}

TEST_CASE("[TestSimpleParser] AffectsBip relation - Week 7 lecture quiz", "[SimpleParser][AffectsBip]") {
  std::string string(R"(
    procedure Gandalf{read shine;ring=shine*2;f=ring+3;read lost;call Frodo;
    if(lost==shine)then{call Aragon;d=(shine+12)*(lost+13);}else{while(f>10){read g;f=ring+4+g*a*2*1+shine;call Frodo;}d=2;}print d;}
    procedure Frodo{a=f+shine;if(a<2)then{call Aragon;}else{f=f+19;}}
    procedure Aragon{while(shine<3){a=1*2+lost-f;while(lost==a){lost=lost-1;w=a+lost;shine=1+ring;}if(a!=shine)then{a=lost+1;w=shine*2;}else {w=1;}}})");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTable = pkb.getAffectsBipTable();

  REQUIRE(affectsBipTable.contains({ "2", "3" }));
  REQUIRE(affectsBipTable.contains({ "2", "11" }));
  REQUIRE(affectsBipTable.contains({ "2", "24" }));
  REQUIRE(affectsBipTable.contains({ "3", "15" }));
  REQUIRE(affectsBipTable.contains({ "3", "18" }));
  REQUIRE(affectsBipTable.contains({ "3", "20" }));
  REQUIRE(affectsBipTable.contains({ "11", "15" }));
  REQUIRE(affectsBipTable.contains({ "11", "18" }));
  REQUIRE(affectsBipTable.contains({ "11", "20" }));
  REQUIRE(affectsBipTable.contains({ "15", "11" }));
  REQUIRE(affectsBipTable.contains({ "18", "20" }));
  REQUIRE(affectsBipTable.contains({ "20", "11" }));
  REQUIRE(affectsBipTable.contains({ "20", "23" }));
  REQUIRE(affectsBipTable.contains({ "22", "8" }));
  REQUIRE(affectsBipTable.contains({ "22", "20" }));
  REQUIRE(affectsBipTable.contains({ "22", "22" }));
  REQUIRE(affectsBipTable.contains({ "22", "23" }));
  REQUIRE(affectsBipTable.contains({ "22", "26" }));
  REQUIRE(affectsBipTable.contains({ "24", "8" }));
  REQUIRE(affectsBipTable.contains({ "24", "11" }));
  REQUIRE(affectsBipTable.contains({ "24", "15" }));
  REQUIRE(affectsBipTable.contains({ "24", "27" }));
  REQUIRE(affectsBipTable.contains({ "26", "11" }));
  REQUIRE(affectsBipTable.size() == 23);
}
