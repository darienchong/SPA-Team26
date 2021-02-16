#include "Spa.h"

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <sstream>

#include "PqlParser.h"
#include "PqlQuery.h"
#include "SimpleParser.h"
#include "Token.h"
#include "Tokeniser.h"

Spa::Spa()
  : pkb(Pkb()) {}

void Spa::parseSourceFile(const std::string& filename) {
  std::ifstream sourceFile(filename);
  if (!sourceFile.is_open()) {
    std::cerr << "Unable to open source file";
    exit(EXIT_FAILURE);
  }
  sourceFile.close();

  // SIMPLE Parsing code here
  Tokeniser t;
  std::list<Token> tokens = t.tokenise(sourceFile);
  SourceProcessor::SimpleParser parser(pkb, tokens);
  parser.parse();

  // DE to fill up pkb here
}

void Spa::evaluateQuery(const std::string& query, std::list<std::string>& results) {
  // PQL evaluation code here
}