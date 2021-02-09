#include <fstream>
#include <list>
#include <sstream>
#include <string>

#include "Token.h"
#include "Tokeniser.h"
#include "catch.hpp"

namespace {
  std::stringstream strToStream(std::string str) {
    std::stringstream stream;
    stream << str << "\n";
    return stream;
  }

  Tokeniser tokeniser;
}

TEST_CASE("[TestTokeniser] Sanity check") {
  REQUIRE(1 == 1);
}

TEST_CASE("[TestTokeniser] Delimiter, single") {
  std::stringstream stream = strToStream("{");
  std::list<Token> tokens = tokeniser.tokenise(stream);
  
  Token token = tokens.front();
  REQUIRE(token.type == TokenType::DELIMITER);
  REQUIRE(token.value == "{");
}

TEST_CASE("[TestTokeniser] Delimiter, multiple") {
  std::stringstream stream = strToStream("{}();_\",");
  std::list<Token> tokens = tokeniser.tokenise(stream);
  std::list<std::string> expectedValues = { "{", "}", "(", ")", ";", "_", "\"", "," };
  std::list<std::string>::const_iterator expectedValuesItr = expectedValues.begin();

  for (const Token token : tokens) {
    TokenType expectedType = TokenType::DELIMITER;
    std::string expectedValue = *expectedValuesItr;

    REQUIRE(token.type == expectedType);
    REQUIRE(token.value == expectedValue);

    ++expectedValuesItr;
  }
}

TEST_CASE("[TestTokeniser] Identifier, positive") {
  std::stringstream stream = strToStream("aName anotherName aName2");
  std::list<Token> tokens = tokeniser.tokenise(stream);
  std::list<std::string> expectedValues = { "aName", "anotherName", "aName2" };
  std::list<std::string>::const_iterator expectedValuesItr = expectedValues.begin();

  for (const Token token : tokens) {
    TokenType expectedType = TokenType::IDENTIFIER;
    std::string expectedValue = *expectedValuesItr;

    REQUIRE(token.type == expectedType);
    REQUIRE(token.value == expectedValue);

    ++expectedValuesItr;
  }
}

TEST_CASE("[TestTokeniser] Identifier, negative") {
  std::stringstream stream = strToStream("3name");
  REQUIRE_THROWS(tokeniser.tokenise(stream));
}

TEST_CASE("[TestTokeniser] Operator, single-char, single") {
  std::stringstream stream = strToStream("<");
  std::list<Token> tokens = tokeniser.tokenise(stream);
  
  Token token = tokens.front();
  REQUIRE(token.type == TokenType::OPERATOR);
  REQUIRE(token.value == "<");
}

TEST_CASE("[TestTokeniser] Operator, two-char, single, positive") {
  std::stringstream stream = strToStream("<=");
  std::list<Token> tokens = tokeniser.tokenise(stream);

  Token token = tokens.front();
  REQUIRE(token.type == TokenType::OPERATOR);
  REQUIRE(token.value == "<=");
}

TEST_CASE("[TestTokeniser] Operator, two-char, multiple, positive") {
  std::stringstream stream = strToStream("<= >= == != && ||");
  std::list<Token> tokens = tokeniser.tokenise(stream);
  std::list<std::string> expectedValues = { "<=", ">=", "==", "!=", "&&", "||" };
  std::list<std::string>::const_iterator expectedValuesItr = expectedValues.begin();

  for (const Token token : tokens) {
    REQUIRE(token.type == TokenType::OPERATOR);
    REQUIRE(token.value == *expectedValuesItr);

    ++expectedValuesItr;
  }
}

TEST_CASE("[TestTokeniser] Operator, two-char, single, negative") {
  std::stringstream stream = strToStream("!!");
  REQUIRE_THROWS(tokeniser.tokenise(stream));
}


