#include "SPA.h"

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <sstream>

#include "PqlParser.h"
#include "PqlQuery.h"
#include "Token.h"
#include "Tokeniser.h"

SPA::SPA()
  : pkb(PKB()) {}

void SPA::parseSourceFile(const std::string& filename) {
  std::ifstream sourceFile(filename);
  if (!sourceFile.is_open()) {
    std::cerr << "Unable to open source file";
    exit(EXIT_FAILURE);
  }
  // SIMPLE Parsing code here
  sourceFile.close();
}

void SPA::evaluateQuery(const std::string& queryString, std::list<std::string>& results) {
  std::stringstream ss(queryString);
  Tokeniser t;
  std::list<Token> tokens = t.tokenise(ss);
  PqlParser p(tokens);
  Pql::Query queryStruct = p.parseQuery();
}