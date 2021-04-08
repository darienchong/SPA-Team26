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
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("b") }));
  REQUIRE(procTable.size() == 2);
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("computeAverage"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
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

    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("computeAverage"), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("x") }));
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

  REQUIRE(callProcTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("b") }));
  REQUIRE(callProcTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("c") }));
  REQUIRE(callProcTable.size() == 2);
  REQUIRE(callsTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("b") }));
  REQUIRE(callsTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("c") }));
  REQUIRE(callsTable.size() == 2);
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("a") }));
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("b") }));
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("c") }));
  REQUIRE(procTable.size() == 3);
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesPTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("c"), pkb.getIntRefFromEntity("z") }));
  REQUIRE(usesPTable.size() == 1);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("z") }));
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
    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesPTable.size() == 3);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesSTable.size() == 3);
    REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("z") }));
    REQUIRE(usesPTable.size() == 2);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("z") }));
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

  REQUIRE(callProcTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("b") }));
  REQUIRE(callProcTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("c") }));
  REQUIRE(callProcTable.size() == 2);
  REQUIRE(callsTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("b") }));
  REQUIRE(callsTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("c") }));
  REQUIRE(callsTable.size() == 2);
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("a") }));
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("b") }));
  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("c") }));
  REQUIRE(procTable.size() == 3);
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("c"), pkb.getIntRefFromEntity("w") }));
  REQUIRE(modifiesPTable.size() == 3);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("w") }));
  REQUIRE(modifiesSTable.size() == 3);
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("v") }));
  REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("c"), pkb.getIntRefFromEntity("z") }));
  REQUIRE(usesPTable.size() == 2);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("v") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("z") }));
  REQUIRE(usesSTable.size() == 2);

  SECTION("Transitive") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table callsTTable = pkb.getCallsTTable();
    Table modifiesPTable = pkb.getModifiesPTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table usesPTable = pkb.getUsesPTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(callsTTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("c") }));
    REQUIRE(callsTTable.size() == 3);
    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("w") }));
    REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("w") }));
    REQUIRE(modifiesPTable.size() == 6);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("w") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("w") }));
    REQUIRE(modifiesSTable.size() == 6);
    REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("z") }));
    REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("v") }));
    REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("b"), pkb.getIntRefFromEntity("z") }));
    REQUIRE(usesPTable.size() == 5);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("v") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("z") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("z") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(1)}));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("s3s1kd03kd42d") }));
  REQUIRE(varTable.size() == 1);
  REQUIRE(stmtTable.size() == 1);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("s3s1kd03kd42d") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(1)}));
  REQUIRE(printTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("s3s1kd03kd42d") }));
  REQUIRE(varTable.size() == 1);
  REQUIRE(stmtTable.size() == 1);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("s3s1kd03kd42d") }));
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

    REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(1)}));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("y") }));
    REQUIRE(varTable.size() == 2);
    REQUIRE(stmtTable.size() == 1);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("y") }));
    REQUIRE(usesSTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x"), pkb.getIntRefFromEntity(expectedPostFixString) }));
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

    REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
    REQUIRE(varTable.size() == 1);
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("0") }));
    REQUIRE(constTable.size() == 1);
    REQUIRE(stmtTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x"), pkb.getIntRefFromEntity(expectedPostFixString) }));
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

    REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("computeAverage") }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(assignTable.size() == 2);
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("y") }));
    REQUIRE(varTable.size() == 2);
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("0") }));
    REQUIRE(constTable.size() == 1);
    REQUIRE(stmtTable.size() == 2); 
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.size() == 1);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesSTable.size() == 2);
    REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(followsTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x"), pkb.getIntRefFromEntity(expectedPostFixString1) }));
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("y"), pkb.getIntRefFromEntity(expectedPostFixString2) }));
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

    REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("geeks") }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("a") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("b") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("c") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("d") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("e") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("f") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("g") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("h") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("i") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("z") }));
    REQUIRE(varTable.size() == 10);
    REQUIRE(stmtTable.size() == 1); 
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("a") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("b") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("c") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("d") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("e") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("g") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("h") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("i") }));
    REQUIRE(usesSTable.size() == 9);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("z") }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("z"), pkb.getIntRefFromEntity(expectedPostFixString) }));
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

    REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("f") }));
    REQUIRE(procTable.size() == 1);
    REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(assignTable.size() == 1);
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("var1340") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("varY") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("varZ") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("v13dj3d0a3kd") }));
    REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("c") }));
    REQUIRE(varTable.size() == 5);
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("1") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("2") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("3") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("4") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("6") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("8") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("9") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("29") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("45") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("57") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("74") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("76") }));
    REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("96") }));
    REQUIRE(constTable.size() == 13);
    REQUIRE(stmtTable.size() == 1); 
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("var1340") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("varY") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("varZ") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("v13dj3d0a3kd") }));
    REQUIRE(usesSTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("c") }));
    REQUIRE(modifiesSTable.size() == 1);
    REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("c"), pkb.getIntRefFromEntity(expectedPostFixString) }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("then") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(assignTable.size() == 1);
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("y") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("then") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("else") }));
  REQUIRE(varTable.size() == 4);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("1") }));
  REQUIRE(constTable.size() == 1);
  REQUIRE(stmtTable.size() == 3); 
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("then") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.size() == 2);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.size() == 2);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x"), pkb.getIntRefFromEntity(expectedPostFixString) }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("y") }));
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
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("x") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("y") }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.size() == 2);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.size() == 2);
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("then") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("then") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("else") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("then") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(assignTable.size() == 2);
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(ifTable.size() == 3);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("y") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("then") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("else") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("if") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("read") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.size() == 7);
  REQUIRE(constTable.size() == 0);
  REQUIRE(stmtTable.size() == 7);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("then") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("if") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("y") }));
  REQUIRE(usesSTable.size() == 9);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("y") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.size() == 3);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(parentTable.size() == 6);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("y"), pkb.getIntRefFromEntity(expectedPostFixString1) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("x"), pkb.getIntRefFromEntity(expectedPostFixString2) }));
  REQUIRE(patternAssignTable.size() == 2);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("y") }));
    REQUIRE(modifiesSTable.size() == 8);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(parentTTable.size() == 10);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("y") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("y") }));
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
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("x") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("y") }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTable.size() == 6);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.size() == 10);
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("then") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("if") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read") }));
      REQUIRE(patternIfTable.size() == 6);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("then") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("else") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"),pkb.getIntRefFromEntity("while") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("x") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("y") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("if") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("then"), pkb.getIntRefFromEntity("read") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("a") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(assignTable.contains({ pkb.getIntRefFromStmtNum(9) }));
  REQUIRE(assignTable.size() == 4);
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(ifTable.size() == 3);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(readTable.size() == 2);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x1") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("y1") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("x") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("z") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("b") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("c") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("f") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("g") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("d") }));
  REQUIRE(varTable.size() == 10);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("2") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("3") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("5") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("6") }));
  REQUIRE(constTable.size() == 4);
  REQUIRE(stmtTable.size() == 10); 
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("y1") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("x") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("b") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromEntity("f") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromEntity("g") }));
  REQUIRE(usesSTable.size() == 6);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x1") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("x") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("z") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("z") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromEntity("c") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromEntity("d") }));
  REQUIRE(modifiesSTable.size() == 6);
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(parentTable.size() == 9);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("x1"), pkb.getIntRefFromEntity(expectedPostFixString1) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("z"), pkb.getIntRefFromEntity(expectedPostFixString2) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("z"), pkb.getIntRefFromEntity(expectedPostFixString3) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromEntity("c"), pkb.getIntRefFromEntity(expectedPostFixString4) }));
  REQUIRE(patternAssignTable.size() == 4);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 3);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x1") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("z") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("c") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("d") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("x") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("z") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("c") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("d") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("z") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("c") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("d") }));
    REQUIRE(modifiesSTable.size() == 18);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(parentTTable.size() == 19);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("y1") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("b") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("g") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("x") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("b") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("g") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("g") }));
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
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("x1") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("x") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("z") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("c") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("d") }));
      REQUIRE(modifiesPTable.size() == 5);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
      REQUIRE(nextTable.size() == 9);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(10) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(10) }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("b") }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.size() == 0);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("y1") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("x") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("b") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("f") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("a"), pkb.getIntRefFromEntity("g") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("procedure") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(whileTable.size() == 1);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("if") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("else") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("read") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("print") }));
  REQUIRE(varTable.size() == 5);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("43") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("3") }));
  REQUIRE(constTable.size() == 2);
  REQUIRE(stmtTable.size() == 3); 
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("print") }));
  REQUIRE(usesSTable.size() == 4);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("read") }));
  REQUIRE(modifiesSTable.size() == 1);
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(followsTable.size() == 1);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
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
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
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
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("procedure"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(modifiesPTable.size() == 1);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTable.size() == 3);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.size() == 6);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternWhileTable.size() == 3);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("procedure"), pkb.getIntRefFromEntity("if") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("procedure"), pkb.getIntRefFromEntity("else") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("procedure"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("procedure"), pkb.getIntRefFromEntity("print") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("i") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(whileTable.size() == 2);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(printTable.size() == 2);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("i") }));
  REQUIRE(varTable.size() == 2);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("1") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("2") }));
  REQUIRE(constTable.size() == 2);
  REQUIRE(stmtTable.size() == 6);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("i") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("i") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("i") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.size() == 6);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("i") }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.size() == 3);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity(expectedPostFixString) }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(followsTTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("i") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("i") }));
    REQUIRE(modifiesSTable.size() == 4);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(parentTTable.size() == 4);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("i") }));
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

      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(affectsTable.size() == 1);
      REQUIRE(affectsTTable.size() == 1);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("i") }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.size() == 7);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("i") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternWhileTable.size() == 3);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("i") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("while") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("i") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(whileTable.size() == 3);
  REQUIRE(printTable.size() == 0);
  REQUIRE(readTable.size() == 0);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("a") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("b") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("c") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("d") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("e") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("f") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("call") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("if") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("print") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("read") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.size() == 11);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("0") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("1") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("2") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("3") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("4") }));
  REQUIRE(constTable.size() == 5);
  REQUIRE(stmtTable.size() == 8); 
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("a") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("b") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("c") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("d") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("e") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("f") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("if") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("call") }));
  REQUIRE(usesSTable.size() == 11);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("if") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("while") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("call") }));
  REQUIRE(modifiesSTable.size() == 5);
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(followsTable.size() == 4);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.size() == 6);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print"), pkb.getIntRefFromEntity(expectedPostFixString1) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read"), pkb.getIntRefFromEntity(expectedPostFixString2) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("if"), pkb.getIntRefFromEntity(expectedPostFixString3) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("while"), pkb.getIntRefFromEntity(expectedPostFixString4) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("call"), pkb.getIntRefFromEntity(expectedPostFixString5) }));
  REQUIRE(patternAssignTable.size() == 5);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(followsTTable.size() == 5);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("read") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("if") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("read") }));
    REQUIRE(modifiesSTable.size() == 12);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.size() == 10);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("print") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("c") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("d") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("e") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("e") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("f") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("if") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("read") }));
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

      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(affectsTable.size() == 4);
      REQUIRE(affectsTTable.size() == 4);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("print") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("if") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("call") }));
      REQUIRE(modifiesPTable.size() == 5);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTable.size() == 10);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.size() == 56);
      REQUIRE(patternIfTable.size() == 0);
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("a") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("b") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("c") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("d") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("e") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("f") }));
      REQUIRE(patternWhileTable.size() == 6);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("a") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("b") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("c") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("d") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("e") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("f") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("print") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("if") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("i"), pkb.getIntRefFromEntity("call") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("proc") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(whileTable.size() == 3);
  REQUIRE(printTable.contains({ pkb.getIntRefFromStmtNum(9) }));
  REQUIRE(printTable.size() == 1);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("D33z") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("nu7z") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("life") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("bad") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("print") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("read") }));
  REQUIRE(varTable.size() == 6);
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("1") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("2") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("6") }));
  REQUIRE(constTable.contains({ pkb.getIntRefFromEntity("42") }));
  REQUIRE(constTable.size() == 4);
  REQUIRE(stmtTable.size() == 9);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("D33z") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("life") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("bad") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("read") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("D33z") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("life") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("life") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("bad") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromEntity("nu7z") }));
  REQUIRE(usesSTable.size() == 9);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("D33z") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("life") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("print") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("D33z") }));
  REQUIRE(modifiesSTable.size() == 4);
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(followsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(followsTable.size() == 3);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(9) }));
  REQUIRE(parentTable.size() == 7);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("life"), pkb.getIntRefFromEntity(expectedPostFixString1) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("print"), pkb.getIntRefFromEntity(expectedPostFixString2) }));
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromEntity("D33z"), pkb.getIntRefFromEntity(expectedPostFixString3) }));
  REQUIRE(patternAssignTable.size() == 3);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(followsTTable.size() == 4);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("life") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("D33z") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("D33z") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("D33z") }));
    REQUIRE(modifiesSTable.size() == 10);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(parentTTable.size() == 12);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("life") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("bad") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("nu7z") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("D33z") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("nu7z") }));
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

      REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(affectsTable.size() == 1);
      REQUIRE(affectsTTable.size() == 1);
      REQUIRE(callsTable.size() == 0);
      REQUIRE(callsTTable.size() == 0);
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("life") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("print") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("D33z") }));
      REQUIRE(modifiesPTable.size() == 3);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTable.size() == 12);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(8) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(6) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(7) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(9) }));
      REQUIRE(nextTTable.size() == 72);
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("life") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("bad") }));
      REQUIRE(patternIfTable.size() == 2);
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("D33z") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("D33z") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromEntity("life") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromEntity("bad") }));
      REQUIRE(patternWhileTable.size() == 4);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("D33z") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("life") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("bad") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("proc"), pkb.getIntRefFromEntity("nu7z") }));
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

  REQUIRE(procTable.contains({ pkb.getIntRefFromEntity("main") }));
  REQUIRE(procTable.size() == 1);
  REQUIRE(ifTable.contains({ pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(ifTable.size() == 1);
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(1) }));
  REQUIRE(whileTable.contains({ pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(whileTable.size() == 2);
  REQUIRE(printTable.size() == 0);
  REQUIRE(readTable.contains({ pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(readTable.size() == 1);
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("while") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("print") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("read") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("if") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("else") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("then") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("call") }));
  REQUIRE(varTable.contains({ pkb.getIntRefFromEntity("procedure") }));
  REQUIRE(varTable.size() == 8);
  REQUIRE(constTable.size() == 0);
  REQUIRE(stmtTable.size() == 5);
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("print") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("if") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("then") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("print") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("call") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("read") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("else") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("call") }));
  REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("procedure") }));
  REQUIRE(usesSTable.size() == 15);
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromEntity("read") }));
  REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("print") }));
  REQUIRE(modifiesSTable.size() == 2);
  REQUIRE(followsTable.size() == 0);
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(parentTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(parentTable.size() == 4);
  REQUIRE(patternAssignTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromEntity("print"), pkb.getIntRefFromEntity(expectedPostFixString) }));
  REQUIRE(patternAssignTable.size() == 1);

  SECTION("With DE relations") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table followsTTable = pkb.getFollowsTTable();
    Table modifiesSTable = pkb.getModifiesSTable();
    Table parentTTable = pkb.getParentTTable();
    Table usesSTable = pkb.getUsesSTable();

    REQUIRE(followsTTable.size() == 0);
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("read") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("print") }));
    REQUIRE(modifiesSTable.size() == 7);
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(parentTTable.size() == 8);
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("then") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("call") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("procedure") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("call") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("read") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("else") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("procedure") }));
    REQUIRE(usesSTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("procedure") }));
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
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(modifiesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("print") }));
      REQUIRE(modifiesPTable.size() == 2);
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTable.size() == 7);
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(1) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
      REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
      REQUIRE(nextTTable.size() == 25);
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("if") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("then") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("print") }));
      REQUIRE(patternIfTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternIfTable.size() == 4);
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("while") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("print") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("read") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("if") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("print") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("call") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("read") }));
      REQUIRE(patternWhileTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromEntity("else") }));
      REQUIRE(patternWhileTable.size() == 9);
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("while") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("print") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("read") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("if") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("else") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("then") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("call") }));
      REQUIRE(usesPTable.contains({ pkb.getIntRefFromEntity("main"), pkb.getIntRefFromEntity("procedure") }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.size() == 3);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTable.size() == 13);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(12) }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.size() == 8);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.size() == 6);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(1) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTable.size() == 10);
  }

  SECTION("NextT") {
    SourceProcessor::DesignExtractor designExtractor(pkb);
    designExtractor.extractDesignAbstractions();

    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(8) }));
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

    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTable.size() == 8);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextT") {
    Table nextTTable = pkb.getNextTTable();

    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
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

    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(13), pkb.getIntRefFromStmtNum(14) }));
    REQUIRE(affectsTable.size() == 19);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(12) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(12) }));
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

    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
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

    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(13) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(13) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(13) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(18) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(20) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(21), pkb.getIntRefFromStmtNum(26) }));
    REQUIRE(affectsTable.size() == 13);
  }

  SECTION("AffectsT") {
    Table affectsTTable = pkb.getAffectsTTable();

    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(affectsTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(13) }));
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

    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(23) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(20) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(22) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(23) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(26) }));
    REQUIRE(affectsTable.contains({ pkb.getIntRefFromStmtNum(24), pkb.getIntRefFromStmtNum(27) }));
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

    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTable.size() == 6);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextBip - After Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTable.size() == 11);
  }

  SECTION("NextBipT") {
    Table nextBipTTable = pkb.getNextBipTTable();

    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(5) }));
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

    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTable.size() == 5);
  }

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  SECTION("NextBip - After Extraction") {
    Table nextBipTable = pkb.getNextBipTable();

    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTable.size() == 13);
  }

  SECTION("NextBipT") {
    Table nextBipTTable = pkb.getNextBipTTable();

    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(2) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(5) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(8) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(3) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(4) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(9) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(10) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(11) }));
    REQUIRE(nextBipTTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(5) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(3) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(1), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(8), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(12) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(13), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(13), pkb.getIntRefFromStmtNum(14) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(16) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(22) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(13) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(13) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(9), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(10) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(13) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(10), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(22) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(13), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(18) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(20) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(9) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(21), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(21), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(13) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(26), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(27), pkb.getIntRefFromStmtNum(27) }));
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

  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(24) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(15) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(18) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(20) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(15) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(18) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(11), pkb.getIntRefFromStmtNum(20) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(15), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(18), pkb.getIntRefFromStmtNum(20) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(20), pkb.getIntRefFromStmtNum(23) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(20) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(22) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(23) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(22), pkb.getIntRefFromStmtNum(26) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(24), pkb.getIntRefFromStmtNum(8) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(24), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(24), pkb.getIntRefFromStmtNum(15) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(24), pkb.getIntRefFromStmtNum(27) }));
  REQUIRE(affectsBipTable.contains({ pkb.getIntRefFromStmtNum(26), pkb.getIntRefFromStmtNum(11) }));
  REQUIRE(affectsBipTable.size() == 23);
}

/////////////////////////////
// AffectsBipStar relation //
/////////////////////////////

TEST_CASE("[TestSimpleParser] AffectsBipStar relation - Call proc directly modifies target var", "[SimpleParser][AffectsBipStar]") {
  std::string string("procedure B{call C;call C;call C;}procedure C{d=a;a=b;b=c;c=d;}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTTable = pkb.getAffectsBipTTable();

  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(6), pkb.getIntRefFromStmtNum(7) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(6) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(7), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.size() == 11);
}

TEST_CASE("[TestSimpleParser] AffectsBipStar relation - While loop", "[SimpleParser][AffectsBipStar]") {
  std::string string("procedure loop{while(true==false){d=a;a=b;b=c;c=d;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTTable = pkb.getAffectsBipTTable();

  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(2), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(3), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(4), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(2) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(3) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(4) }));
  REQUIRE(affectsBipTTable.contains({ pkb.getIntRefFromStmtNum(5), pkb.getIntRefFromStmtNum(5) }));
  REQUIRE(affectsBipTTable.size() == 16);
}

TEST_CASE("[TestSimpleParser] AffectsBipStar relation - Call while while loop", "[SimpleParser][AffectsBipStar]") {
  std::string string("procedure call{while(1==2){call loop;}}procedure loop{while(true==false){d=a;a=b;b=c;c=d;}}");
  std::list<Token> simpleProg = expressionStringToTokens(string);
  Pkb pkb;
  SourceProcessor::SimpleParser parser(pkb, simpleProg);
  parser.parse();

  SourceProcessor::DesignExtractor designExtractor(pkb);
  designExtractor.extractDesignAbstractions();

  Table affectsBipTTable = pkb.getAffectsBipTTable();

  REQUIRE(affectsBipTTable.size() == 16);
}
