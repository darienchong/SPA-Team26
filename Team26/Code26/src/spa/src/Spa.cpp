#include "Spa.h"

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <sstream>
#include <exception>

#include "PqlEvaluator.h"
#include "PqlParser.h"
#include "PqlQuery.h"
#include "SimpleParser.h"
#include "DesignExtractor.h"
#include "Token.h"
#include "Tokeniser.h"
#include "SpaException.h"

Spa::Spa()
  : pkb(Pkb()) {
}

void Spa::parseSourceFile(const std::string& filename) {
  std::ifstream sourceFile(filename);
  if (!sourceFile.is_open()) {
    std::cout << "Unable to open source file" << std::endl;
    exit(EXIT_FAILURE);
  }

  try {
    std::list<Token> tokens = Tokeniser()
      .notAllowingLeadingZeroes()
      .consumingWhitespace()
      .tokenise(sourceFile);
    sourceFile.close();

    SourceProcessor::SimpleParser parser(pkb, tokens);
    parser.parse();
    SourceProcessor::DesignExtractor(pkb).extractDesignAbstractions();

  } catch (const std::exception& e) {
    pkb = Pkb();
    std::cout << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    pkb = Pkb();
    std::cout << "OOPS! An unexpected error occured!";
    exit(EXIT_FAILURE);
  }
}

void Spa::evaluateQuery(const std::string& queryString, std::list<std::string>& results) {
  try {
    std::stringstream ss(queryString);
    std::list<Token> tokens = Tokeniser()
      .allowingLeadingZeroes()
      .notConsumingWhitespace()
      .tokenise(ss);
    Pql::PqlParser parser(tokens);
    Pql::Query queryObject = parser.parseQuery();
    if (queryObject.hasSemanticError()) {
      if (queryObject.isBoolean()) {
        results.push_back("FALSE");
      }
      throw Pql::SemanticError(queryObject.getSemanticErrorMessage());
    }
    Pql::PqlEvaluator evaluator(pkb, queryObject, results);
    evaluator.evaluateQuery();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "OOPS! An unexpected error occured!";
  }
}
