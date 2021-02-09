#include "SPA.h"

#include <iostream>
#include <fstream>

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

void SPA::evaluateQuery(const std::string& query, std::list<std::string>& results) {
  // PQL evaluation code here
}