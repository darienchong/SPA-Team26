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

/*
* Integration testing of SimpleParser with Tokeniser, ExprParser and Pkb
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

// Procedure
TEST_CASE("Invalid parsing procedure", "[SimpleParser][Procedure]") {
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

  SECTION("Extra procedure") {
    std::string string("procedure computeAverage {x=1;} procedure b{x=2;}");
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

// Read statement
TEST_CASE("Read statement", "[SimpleParser][Read]") {
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

  SECTION("Valid read statement") {
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
}

// Print statement
TEST_CASE("Print statement", "[SimpleParser][Print]") {
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

  SECTION("Valid print statement") {
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
}

// Assign statement - invalid cases
TEST_CASE("Invalid assign statement", "[TestSimpleParser][Assign]") {
  SECTION("LHS constant") {
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

  SECTION("LHS variable") {
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
}

// Assign statement - valid cases
// TODO: uncomment constant checking after pkb side has implemented the methods
TEST_CASE("Valid assign statement", "[SimpleParser][Assign]") {
  SECTION("Basic") {
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

      std::string expectedPostFixString("y");

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

      std::string expectedPostFixString("0");

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

      std::string expectedPostFixString1("0");
      std::string expectedPostFixString2("x");

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

  SECTION("Advanced") {
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

      std::string expectedPostFixString("a b c d * e - * f g h * + * + i -");

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

      std::string expectedPostFixString("96 9 * var1340 4 * + 2 + 76 8 varY - * 2 * 8 / 6 * - 45 1 varZ 1 * 4 / + 57 2 / 9 / 29 * 6 + + v13dj3d0a3kd 4 / - / + 3 + 4 - 74 2 * +");

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
}

// If statement only
TEST_CASE("Invalid if statement", "[SimpleParser][If]") {
  SECTION("Parenthesis") {
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

// Note for loops, testing relations have to be direct, not transitive
TEST_CASE("Valid if statement", "[SimpleParser][If]") {
  SECTION("Basic") {
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

    std::string expectedPostFixString("1");

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
      DesignExtractor designExtractor;
      designExtractor.extractDesignAbstractions(pkb);
      Table parentTTable = pkb.getParentTTable();
      Table usesSTable = pkb.getUsesSTable();
      Table modifiesSTable = pkb.getModifiesSTable();

      REQUIRE(usesSTable.size() == 2);
      REQUIRE(modifiesSTable.contains({ "1", "x" }));
      REQUIRE(modifiesSTable.contains({ "1", "y" }));
      REQUIRE(modifiesSTable.size() == 4);
      REQUIRE(parentTTable.size() == 2);
    }
  }

  SECTION("Advanced") {
    SECTION("Singly nested if") {
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

      std::string expectedPostFixString1("x");
      std::string expectedPostFixString2("y");

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
        DesignExtractor designExtractor;
        designExtractor.extractDesignAbstractions(pkb);
        Table parentTTable = pkb.getParentTTable();
        Table usesSTable = pkb.getUsesSTable();
        Table modifiesSTable = pkb.getModifiesSTable();

        REQUIRE(usesSTable.contains({ "1", "while" }));
        REQUIRE(usesSTable.contains({ "1", "x" }));
        REQUIRE(usesSTable.contains({ "1", "if" }));
        REQUIRE(usesSTable.contains({ "1", "read" }));
        REQUIRE(usesSTable.contains({ "1", "y" }));
        REQUIRE(usesSTable.contains({ "2", "x" }));
        REQUIRE(usesSTable.contains({ "5", "x" }));
        REQUIRE(usesSTable.contains({ "5", "y" }));
        REQUIRE(usesSTable.size() == 17);
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
      }
    }

    SECTION("Doubly nested if") {
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

      std::string expectedPostFixString1("y1");
      std::string expectedPostFixString2("3");
      std::string expectedPostFixString3("6");
      std::string expectedPostFixString4("f g 5 / +");

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
        DesignExtractor designExtractor;
        designExtractor.extractDesignAbstractions(pkb);
        Table parentTTable = pkb.getParentTTable();
        Table usesSTable = pkb.getUsesSTable();
        Table modifiesSTable = pkb.getModifiesSTable();

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
      }
    }
  }
}

// While statement only
TEST_CASE("Invalid while statement", "[SimpleParser][While]") {
  SECTION("Parenthesis") {
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

// Note for loops, testing relations have to be direct, not transitive
TEST_CASE("Valid while statement", "[SimpleParser][While]") {
  SECTION("Basic") {
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
      DesignExtractor designExtractor;
      designExtractor.extractDesignAbstractions(pkb);
      Table parentTTable = pkb.getParentTTable();
      Table usesSTable = pkb.getUsesSTable();
      Table modifiesSTable = pkb.getModifiesSTable();

      REQUIRE(usesSTable.size() == 4);
      REQUIRE(modifiesSTable.contains({ "1", "read" }));
      REQUIRE(modifiesSTable.size() == 2);
      REQUIRE(parentTTable.size() == 1);
    }
  }

  SECTION("Advanced") {
    SECTION("Singly nested while") {
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

      std::string expectedPostFixString("i 1 +");

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
        DesignExtractor designExtractor;
        designExtractor.extractDesignAbstractions(pkb);
        Table parentTTable = pkb.getParentTTable();
        Table usesSTable = pkb.getUsesSTable();
        Table followsTTable = pkb.getFollowsTTable();
        Table modifiesSTable = pkb.getModifiesSTable();

        REQUIRE(usesSTable.contains({ "2", "i" }));
        REQUIRE(usesSTable.size() == 7);
        REQUIRE(modifiesSTable.contains({ "2", "i" }));
        REQUIRE(modifiesSTable.contains({ "3", "i" }));
        REQUIRE(modifiesSTable.size() == 4);
        REQUIRE(followsTTable.contains({ "1", "6" }));
        REQUIRE(followsTTable.size() == 4);
        REQUIRE(parentTTable.contains({ "2", "4" }));
        REQUIRE(parentTTable.size() == 4);
      }
    }

    SECTION("Doubly nested while") {
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

      std::string expectedPostFixString1("print 1 +");
      std::string expectedPostFixString2("read 3 /");
      std::string expectedPostFixString3("if 2 *");
      std::string expectedPostFixString4("while 0 %");
      std::string expectedPostFixString5("call call -");

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
        DesignExtractor designExtractor;
        designExtractor.extractDesignAbstractions(pkb);
        Table parentTTable = pkb.getParentTTable();
        Table usesSTable = pkb.getUsesSTable();
        Table followsTTable = pkb.getFollowsTTable();
        Table modifiesSTable = pkb.getModifiesSTable();

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
        REQUIRE(modifiesSTable.contains({ "1", "print" }));
        REQUIRE(modifiesSTable.contains({ "1", "read" }));
        REQUIRE(modifiesSTable.contains({ "1", "if" }));
        REQUIRE(modifiesSTable.contains({ "1", "while" }));
        REQUIRE(modifiesSTable.contains({ "3", "read" }));
        REQUIRE(modifiesSTable.contains({ "3", "if" }));
        REQUIRE(modifiesSTable.contains({ "4", "read" }));
        REQUIRE(modifiesSTable.size() == 12);
        REQUIRE(followsTTable.contains({ "2", "7" }));
        REQUIRE(followsTTable.size() == 5);
        REQUIRE(parentTTable.contains({ "1", "4" }));
        REQUIRE(parentTTable.contains({ "1", "5" }));
        REQUIRE(parentTTable.contains({ "1", "6" }));
        REQUIRE(parentTTable.contains({ "3", "5" }));
        REQUIRE(parentTTable.size() == 10);
      }
    }
  }
}

// Combination of if and while statements
TEST_CASE("Loops", "[SimpleParser][Assign][If][Print][Read][While]") {
  SECTION("Nested/follows if/while loops") {
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

    std::string expectedPostFixString1("42");
    std::string expectedPostFixString2("read");
    std::string expectedPostFixString3("life 1 -");

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
    REQUIRE(stmtTable.size() == 9); // maybe can also check the statement num? maybe not necessary
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
      DesignExtractor designExtractor;
      designExtractor.extractDesignAbstractions(pkb);
      Table parentTTable = pkb.getParentTTable();
      Table usesSTable = pkb.getUsesSTable();
      Table followsTTable = pkb.getFollowsTTable();
      Table modifiesSTable = pkb.getModifiesSTable();

      REQUIRE(usesSTable.contains({ "2", "life" }));
      REQUIRE(usesSTable.contains({ "2", "bad" }));
      REQUIRE(usesSTable.contains({ "2", "read" }));
      REQUIRE(usesSTable.contains({ "2", "nu7z" }));
      REQUIRE(usesSTable.contains({ "4", "read" }));
      REQUIRE(usesSTable.contains({ "4", "D33z" }));
      REQUIRE(usesSTable.contains({ "8", "nu7z" }));
      REQUIRE(usesSTable.size() == 16);
      REQUIRE(modifiesSTable.contains({ "2", "life" }));
      REQUIRE(modifiesSTable.contains({ "2", "print" }));
      REQUIRE(modifiesSTable.contains({ "2", "D33z" }));
      REQUIRE(modifiesSTable.contains({ "4", "print" }));
      REQUIRE(modifiesSTable.contains({ "4", "D33z" }));
      REQUIRE(modifiesSTable.contains({ "6", "D33z" }));
      REQUIRE(modifiesSTable.size() == 10);
      REQUIRE(followsTTable.contains({ "3", "8" }));
      REQUIRE(followsTTable.size() == 4);
      REQUIRE(parentTTable.contains({ "2", "5" }));
      REQUIRE(parentTTable.contains({ "2", "6" }));
      REQUIRE(parentTTable.contains({ "2", "7" }));
      REQUIRE(parentTTable.contains({ "2", "9" }));
      REQUIRE(parentTTable.contains({ "4", "7" }));
      REQUIRE(parentTTable.size() == 12);
    }
  }

  SECTION("Random example") {
    std::string string("procedure main {while (((while + (print)) != read) || ((if) <= else)) {if (!((if > then) && (print < while))) then {read read;} else {while (print >= (((call) + ((read))) % else)) {print = call + procedure;}}}}");
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

    std::string expectedPostFixString("call procedure +");

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
    REQUIRE(stmtTable.size() == 5); // maybe can also check the statement num? maybe not necessary
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
      DesignExtractor designExtractor;
      designExtractor.extractDesignAbstractions(pkb);
      Table parentTTable = pkb.getParentTTable();
      Table usesSTable = pkb.getUsesSTable();
      Table followsTTable = pkb.getFollowsTTable();
      Table modifiesSTable = pkb.getModifiesSTable();

      REQUIRE(usesSTable.contains({ "1", "then" }));
      REQUIRE(usesSTable.contains({ "1", "call" }));
      REQUIRE(usesSTable.contains({ "1", "procedure" }));
      REQUIRE(usesSTable.contains({ "2", "call" }));
      REQUIRE(usesSTable.contains({ "2", "read" }));
      REQUIRE(usesSTable.contains({ "2", "else" }));
      REQUIRE(usesSTable.contains({ "2", "procedure" }));
      REQUIRE(usesSTable.contains({ "4", "procedure" }));
      REQUIRE(usesSTable.size() == 23);
      REQUIRE(modifiesSTable.contains({ "1", "read" }));
      REQUIRE(modifiesSTable.contains({ "1", "print" }));
      REQUIRE(modifiesSTable.contains({ "2", "read" }));
      REQUIRE(modifiesSTable.contains({ "2", "print" }));
      REQUIRE(modifiesSTable.contains({ "4", "print" }));
      REQUIRE(modifiesSTable.size() == 7);
      REQUIRE(followsTTable.size() == 0);
      REQUIRE(parentTTable.contains({ "1", "3" }));
      REQUIRE(parentTTable.contains({ "1", "4" }));
      REQUIRE(parentTTable.contains({ "1", "5" }));
      REQUIRE(parentTTable.contains({ "2", "5" }));
      REQUIRE(parentTTable.size() == 8);
    }
  }

  SECTION("Random example, but with extra parenthesis around cond expr") {
    std::string string("procedure main {while ((((while + (print)) != read) || ((if) <= else))) {if (!((if > then) && (print < while))) then {read read;} else {while (print >= (((call) + ((read))) % else)) {print = call + procedure;}}}}");
    std::list<Token> simpleProg = expressionStringToTokens(string);
    Pkb pkb;
    SourceProcessor::SimpleParser parser(pkb, simpleProg);
    REQUIRE_THROWS(parser.parse());
  }
}
