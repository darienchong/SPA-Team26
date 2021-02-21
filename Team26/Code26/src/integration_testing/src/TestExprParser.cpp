#include "catch.hpp"

#include <string>
#include <list>
#include <unordered_set>
#include <sstream>

#include "Tokeniser.h"
#include "Token.h"
#include "ExprParser.h"

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

//==================//
//  CondExprParser  //
//==================//

TEST_CASE("Valid conditional expressions", "[ExprParser][CondExprParser]") {
  SECTION("Basic conditional expression") {
    std::string string(R"( x == y )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
    std::unordered_set<std::string> expectedConstantsUsed = { };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Conditional expression with single additional parenthesis") {
    std::string string(R"( (1) != (count) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "count" };
    std::unordered_set<std::string> expectedConstantsUsed = { "1" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Conditional expression with nested additional parenthesis") {
    std::string string(R"( (((x))) <= (((2))) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "x" };
    std::unordered_set<std::string> expectedConstantsUsed = { "2" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Conditional expression with logical operators") {
    std::string string(R"( (!(0 >= x)) || (2 != result) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "result", "x" };
    std::unordered_set<std::string> expectedConstantsUsed = { "0", "2" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Conditional expression with logical operators in different order") {
    std::string string(R"( !((0 >= x) || (2 != result)) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "result", "x" };
    std::unordered_set<std::string> expectedConstantsUsed = { "0", "2" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Advanced conditional expression 1") {
    std::string string(R"( ((7 * (2 + 3) / (var - temp % x * y) + 2) >= z) || ((7 / 3 + N < temp) && (2 != result + sum * 2)) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "x", "y", "z", "var", "temp", "result", "sum", "N" };
    std::unordered_set<std::string> expectedConstantsUsed = { "2", "7", "3" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }

  SECTION("Advanced conditional expression 2") {
    std::string string(R"( ((7 * (2 + 3) / (var - temp % x * y) + 2) >= z) || ((7 / 3 + N < temp) && (!(2 != result + sum * 2))) )");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::unordered_set<std::string> expectedVariableUsed = { "x", "y", "z", "var", "temp", "result", "sum", "N" };
    std::unordered_set<std::string> expectedConstantsUsed = { "2", "7", "3" };
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
  }
}

TEST_CASE("Invalid conditional expressions", "[ExprParser][CondExprParser]") {
  SECTION("Empty conditional expression") {
    std::string string("");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Empty parenthesis") {
    std::string string("()");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(var_name) cond_op (const)") {
    std::string string("(a) || (3)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  // should be the same as prev, but write just in case
  SECTION("(rel_factor) cond_op (rel_factor)") {
    std::string string("(a) && (z+2)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(rel_expr) cond_op (rel_factor)") {
    std::string string("(b1G==b4l75) || (D33zNu75)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("! (rel_factor)") {
    std::string string("!(h1D4D)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(rel_expr) cond_op (rel_expr) cond_op (rel_expr)") {
    std::string string("(a<=b) || (c>d) && (e!=f)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(rel_expr) ! (rel_expr)") {
    std::string string("(a<=b) ! (c>d)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("! (rel_expr) cond_op (rel_expr)") {
    std::string string("! (a<=b) && (c>d)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("assign_expr") {
    std::string string("x=3");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(rel_expr) cond_op (assign_expr)") {
    std::string string("(a<=b) && (x=3)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("rel_factor rel_op rel_factor rel_op rel_factor") {
    std::string string("a == b == c");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("(rel_factor rel_op rel_factor) rel_op rel_factor") {
    std::string string("(a == b) == c");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("rel_factor rel_op rel_expr") {
    std::string string("a == (b+3+d/3<4)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("rel_expr rel_op rel_expr") {
    std::string string("(b+3+d/3<4) == (b+3+d/3<4)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Invalid rel_expr 1") {
    std::string string("a == b+3/-d * f");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Invalid rel_expr 2") {
    std::string string("a == b+3/(-d * f)");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Invalid token") {
    std::string string("(!(a*3%1==(3/d-4))) || ((A;B) && (!(5<=0)))");
    std::list<Token> condExprTokens = expressionStringToTokens(string);
    ExprProcessor::CondExprParser parser(condExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Mismatched parenthesis") {
    SECTION("Extra '('") {
      std::string string("(!(a*3%4>(3/d-4))) || ((A!=B) && (!(5<=0))");
      std::list<Token> condExprTokens = expressionStringToTokens(string);
      ExprProcessor::CondExprParser parser(condExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Extra ')'") {
      std::string string("(!(a*3%4>(3/d-4))) || ((A!=B) && (!(5<=0))))");
      std::list<Token> condExprTokens = expressionStringToTokens(string);
      ExprProcessor::CondExprParser parser(condExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

//==================//
// AssignExprParser //
//==================//

// Invalid variable names eg. "x3)3_5!$" would never occur since the tokens would be split by tokeniser
TEST_CASE("Assignment expressions - single variable", "[ExprParser][AssignExprParser]") {
  SECTION("Valid single length variable") {
    std::string string("x");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed = { "x" };
    std::unordered_set<std::string> expectedConstantsUsed;
    std::string expectedPostFixString(" x ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid multi-length lowercase letter variable") {
    std::string string("abcdefghijklmnopqrstuvwxyz");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed = { "abcdefghijklmnopqrstuvwxyz" };
    std::unordered_set<std::string> expectedConstantsUsed;
    std::string expectedPostFixString(" abcdefghijklmnopqrstuvwxyz ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid multi-length uppercase letter variable") {
    std::string string("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
    std::unordered_set<std::string> expectedConstantsUsed;
    std::string expectedPostFixString(" ABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid multi-length mixcase letter variable") {
    std::string string("jksJFeijKSJlmcJLezZnkEPx");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed = { "jksJFeijKSJlmcJLezZnkEPx" };
    std::unordered_set<std::string> expectedConstantsUsed;
    std::string expectedPostFixString(" jksJFeijKSJlmcJLezZnkEPx ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid multi-length letter-integer-mix variable") {
    std::string string("a1b23c567fsf98n0y040");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed = { "a1b23c567fsf98n0y040" };
    std::unordered_set<std::string> expectedConstantsUsed;
    std::string expectedPostFixString(" a1b23c567fsf98n0y040 ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Invalid variable with operators") {
    SECTION("variable operator") {
      std::string string("a+");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("operator variable") {
      std::string string("% a");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

// Invalid consts, eg "0314" would be handled by tokeniser and will not reach AssignExprParser
TEST_CASE("Assignment expressions - single constant", "[ExprParser][AssignExprParser]") {
  SECTION("Valid single length non-zero constant") {
    std::string string("1");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed;
    std::unordered_set<std::string> expectedConstantsUsed = { "1" };
    std::string expectedPostFixString(" 1 ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid single length zero constant") {
    std::string string("0");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed;
    std::unordered_set<std::string> expectedConstantsUsed = { "0" };
    std::string expectedPostFixString(" 0 ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Valid multi-length constant") {
    std::string string("1234567890");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    parser.parse();
    std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
    std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
    std::string resultPostFixString = parser.getPostfixExprString();
    std::unordered_set<std::string> expectedVariableUsed;
    std::unordered_set<std::string> expectedConstantsUsed = { "1234567890" };
    std::string expectedPostFixString(" 1234567890 ");
    REQUIRE(resultVariablesUsed == expectedVariableUsed);
    REQUIRE(resultConstantsUsed == expectedConstantsUsed);
    REQUIRE(resultPostFixString == expectedPostFixString);
  }

  SECTION("Invalid constant with operators") {
    SECTION("constant operator") {
      std::string string("1 *");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("operator constant") {
      std::string string("+ 1");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

TEST_CASE("Assignment expressions - double variables", "[ExprParser][AssignExprParser]") {
  SECTION("Invalid double variables (no operators between)") {
    std::string string("x y");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Invalid symbols") {
    SECTION("Invalid symbols before variables") {
      std::string string("|| x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid symbols between variables") {
      std::string string("x < y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid symbols after variables") {
      std::string string("x y =");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Addition") {
    SECTION("Valid double variable addition") {
      std::string string("x + y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
      std::unordered_set<std::string> expectedConstantsUsed;
      std::string expectedPostFixString(" x y + ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double variable addition, + before 2 variables") {
      std::string string("+ x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double variable addition, + after 2 variables") {
      std::string string("x y +");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Subtraction") {
    SECTION("Valid double variable subtraction") {
      std::string string("x - y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
      std::unordered_set<std::string> expectedConstantsUsed;
      std::string expectedPostFixString(" x y - ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double variable subtraction, - before 2 variables") {
      std::string string("- x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double variable subtraction, - after 2 variables") {
      std::string string("x y -");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Multiplication") {
    SECTION("Valid double variable multiplication") {
      std::string string("x * y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
      std::unordered_set<std::string> expectedConstantsUsed;
      std::string expectedPostFixString(" x y * ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double variable multiplication, * before 2 variables") {
      std::string string("* x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double variable multiplication, * after 2 variables") {
      std::string string("x y *");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Division") {
    SECTION("Valid double variable division") {
      std::string string("x / y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
      std::unordered_set<std::string> expectedConstantsUsed;
      std::string expectedPostFixString(" x y / ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double variable division, / before 2 variables") {
      std::string string("/ x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double variable division, / after 2 variables") {
      std::string string("x y /");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Modulo") {
    SECTION("Valid double variable modulo") {
      std::string string("x % y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
      std::unordered_set<std::string> expectedConstantsUsed;
      std::string expectedPostFixString(" x y % ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double variable modulo, % before 2 variables") {
      std::string string("% x y");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double variable modulo, % after 2 variables") {
      std::string string("x y %");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

TEST_CASE("Assignment expressions - double constants", "[ExprParser][AssignExprParser]") {
  SECTION("Invalid double constants (no operators between)") {
    std::string string("1 2");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Invalid symbols") {
    SECTION("Invalid symbols before constants") {
      std::string string("! 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid symbols between constants") {
      std::string string("1 == 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid symbols after constants") {
      std::string string("1 2 &&");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Addition") {
    SECTION("Valid double constants addition") {
      std::string string("1 + 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed;
      std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
      std::string expectedPostFixString(" 1 2 + ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double constants addition, + before 2 constants") {
      std::string string("+ 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double constants addition, + after 2 constants") {
      std::string string("1 2 +");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Subtraction") {
    SECTION("Valid double constants subtraction") {
      std::string string("1 - 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed;
      std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
      std::string expectedPostFixString(" 1 2 - ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double constants subtraction, - before 2 constants") {
      std::string string("- 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double constants subtraction, - after 2 constants") {
      std::string string("1 2 -");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Multiplication") {
    SECTION("Valid double constants multiplication") {
      std::string string("1 * 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed;
      std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
      std::string expectedPostFixString(" 1 2 * ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double constants multiplication, * before 2 constants") {
      std::string string("* 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double constants multiplication, * after 2 constants") {
      std::string string("1 2 *");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Division") {
    SECTION("Valid double constants division") {
      std::string string("1 / 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed;
      std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
      std::string expectedPostFixString(" 1 2 / ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double constants division, / before 2 constants") {
      std::string string("/ 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double constants division, / after 2 constants") {
      std::string string("1 2 /");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Modulo") {
    SECTION("Valid double constants modulo") {
      std::string string("1 % 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed;
      std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
      std::string expectedPostFixString(" 1 2 % ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid double constants modulo, % before 2 constants") {
      std::string string("% 1 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid double constants modulo, % after 2 constants") {
      std::string string("1 2 %");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

TEST_CASE("Assignment expressions - single variable single constant", "[ExprParser][AssignExprParser]") {
  SECTION("Invalid single variable single constant (no operators between)") {
    std::string string("x 1");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Addition") {
    SECTION("Valid single variable single constant addition") {
      std::string string("x + 1");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x" };
      std::unordered_set<std::string> expectedConstantsUsed = { "1" };
      std::string expectedPostFixString(" x 1 + ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid single variable single constant addition, + before single variable single constant") {
      std::string string("+ x 1");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid single variable single constant addition, + after single variable single constant") {
      std::string string("x 1 +");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Subtraction") {
    SECTION("Valid single variable single constant subtraction") {
      std::string string("1 - x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x" };
      std::unordered_set<std::string> expectedConstantsUsed = { "1" };
      std::string expectedPostFixString(" 1 x - ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid single variable single constant subtraction, - before single variable single constant") {
      std::string string("- 1 x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid single variable single constant, - after single variable single constant") {
      std::string string("1 x -");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Multiplication") {
    SECTION("Valid single variable single constant multiplication") {
      std::string string("x * 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x" };
      std::unordered_set<std::string> expectedConstantsUsed = { "2" };
      std::string expectedPostFixString(" x 2 * ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid single variable single constant multiplication, * before single variable single constant") {
      std::string string("* x 2");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid single variable single constant multiplication, * after single variable single constant") {
      std::string string("x 2 *");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Division") {
    SECTION("Valid single variable single constant division") {
      std::string string("1 / x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x" };
      std::unordered_set<std::string> expectedConstantsUsed = { "1" };
      std::string expectedPostFixString(" 1 x / ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid single variable single constant division, / before single variable single constant") {
      std::string string("/ 1 x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid single variable single constant, / after single variable single constant") {
      std::string string("1 x /");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }

  SECTION("Modulo") {
    SECTION("Valid single variable single constant modulo") {
      std::string string("1 % x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      parser.parse();
      std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
      std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
      std::string resultPostFixString = parser.getPostfixExprString();
      std::unordered_set<std::string> expectedVariableUsed = { "x" };
      std::unordered_set<std::string> expectedConstantsUsed = { "1" };
      std::string expectedPostFixString(" 1 x % ");
      REQUIRE(resultVariablesUsed == expectedVariableUsed);
      REQUIRE(resultConstantsUsed == expectedConstantsUsed);
      REQUIRE(resultPostFixString == expectedPostFixString);
    }

    SECTION("Invalid single variable single constant modulo, % before single variable single constant") {
      std::string string("% 1 x");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }

    SECTION("Invalid single variable single constant modulo, % after single variable single constant") {
      std::string string("1 x %");
      std::list<Token> assignExprTokens = expressionStringToTokens(string);
      ExprProcessor::AssignExprParser parser(assignExprTokens);
      REQUIRE_THROWS(parser.parse());
    }
  }
}

TEST_CASE("Assignment expressions - variables/constants with ()", "[ExprParser][AssignExprParser]") {
  SECTION("Mismatched ()") {
    SECTION("Extra (") {
      SECTION("Single (") {
        std::string string("(");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("Multiple (") {
        std::string string("(((((())))");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }
    }

    SECTION("Extra )") {
      SECTION("Single )") {
        std::string string(")");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("Multiple )") {
        std::string string("(((())))))");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }
    }
  }

  SECTION("Matching ()") {
    SECTION("Basics") {
      SECTION("Empty ()") {
        std::string string("()");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("(Variable)") {
        std::string string("(x)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x" };
        std::unordered_set<std::string> expectedConstantsUsed;
        std::string expectedPostFixString(" x ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("(Constant)") {
        std::string string("((((1))))");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed;
        std::unordered_set<std::string> expectedConstantsUsed = { "1" };
        std::string expectedPostFixString(" 1 ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("(Variable and constant)") {
        std::string string("((((1 + (x)))))");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1" };
        std::string expectedPostFixString(" 1 x + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }
    }

    SECTION("Operator-operand-bracket-interactions") {
      SECTION("1") {
        std::string string("x + 1 (+ z)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("2") {
        std::string string("x + 1 (+) z");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("3") {
        std::string string("x + (1 + z)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "z" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1" };
        std::string expectedPostFixString(" x 1 z + + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("4") {
        std::string string("x + 1 + (z)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "z" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1" };
        std::string expectedPostFixString(" x 1 + z + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("5") {
        std::string string("x * (5) z");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("6") {
        std::string string("x (5 + z)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }

      SECTION("7") {
        std::string string("(x * (5 + z) + ) + x");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        REQUIRE_THROWS(parser.parse());
      }
    }

    SECTION("Precedence of operators") {
      SECTION("No brackets") {
        std::string string("x * 1 + y * 2");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 * y 2 * + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 1") {
        std::string string("x * ( 1 + y ) * 2");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 y + * 2 * ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 2") {
        std::string string("( x * 1 ) + y * 2");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 * y 2 * + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 3") {
        std::string string("( x * 1 ) + ( y * 2 )");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 * y 2 * + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 4") {
        std::string string("x * 1 + ( y * 2 )");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 * y 2 * + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 5") {
        std::string string("x * ( 1 + y * 2 )");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 y 2 * + * ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Brackets 6") {
        std::string string("( x * 1 + y ) * 2");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "x", "y" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2" };
        std::string expectedPostFixString(" x 1 * y + 2 * ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }
    }

    SECTION("Advanced") {
      SECTION("Geekforgeeks example") {
        std::string string("a + b * (c * d - e) * (f + g * h) - i");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "a", "b", "c", "d", "e", "f", "g", "h", "i" };
        std::unordered_set<std::string> expectedConstantsUsed;
        std::string expectedPostFixString(" a b c d * e - * f g h * + * + i - ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Geeksforgeeks comment example") {
        std::string string("96*9+var1340*4+2-76*(8-varY)*2/8*6+45/(1+varZ*1/4+(57/2/9*29+6)-v13dj3d0a3kd/4)+3-4+74*2");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "var1340", "varY", "varZ", "v13dj3d0a3kd" };
        std::unordered_set<std::string> expectedConstantsUsed = { "1", "2", "3", "4", "6", "8", "9", "29", "45", "57", "74", "76", "96" };
        std::string expectedPostFixString(" 96 9 * var1340 4 * + 2 + 76 8 varY - * 2 * 8 / 6 * - 45 1 varZ 1 * 4 / + 57 2 / 9 / 29 * 6 + + v13dj3d0a3kd 4 / - / + 3 + 4 - 74 2 * + ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }

      SECTION("Runestone academy example") {
        std::string string("(A + B) * C - (D - E) * (F + G)");
        std::list<Token> assignExprTokens = expressionStringToTokens(string);
        ExprProcessor::AssignExprParser parser(assignExprTokens);
        parser.parse();
        std::unordered_set<std::string> resultVariablesUsed = parser.getVariables();
        std::unordered_set<std::string> resultConstantsUsed = parser.getConstants();
        std::string resultPostFixString = parser.getPostfixExprString();
        std::unordered_set<std::string> expectedVariableUsed = { "A", "B", "C", "D", "E", "F", "G" };
        std::unordered_set<std::string> expectedConstantsUsed;
        std::string expectedPostFixString(" A B + C * D E - F G + * - ");
        REQUIRE(resultVariablesUsed == expectedVariableUsed);
        REQUIRE(resultConstantsUsed == expectedConstantsUsed);
        REQUIRE(resultPostFixString == expectedPostFixString);
      }
    }
  }
}

// Fill in other examples for assign expr parsing
TEST_CASE("Assignment expressions - others", "[ExprParser][AssignExprParser]") {
  SECTION("Empty assignment") {
    std::string string("");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Empty parenthesis") {
    std::string string("()");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }

  SECTION("Operator only") {
    std::string string("+");
    std::list<Token> assignExprTokens = expressionStringToTokens(string);
    ExprProcessor::AssignExprParser parser(assignExprTokens);
    REQUIRE_THROWS(parser.parse());
  }
}
