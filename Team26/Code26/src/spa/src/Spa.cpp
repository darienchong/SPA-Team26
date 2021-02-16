#include "Spa.h"

#include <iostream>
#include <fstream>

Spa::Spa()
  : pkb(Pkb()) {}

void Spa::parseSourceFile(const std::string& filename) {
  std::ifstream sourceFile(filename);
  if (!sourceFile.is_open()) {
    std::cerr << "Unable to open source file";
    exit(EXIT_FAILURE);
  }
  // SIMPLE Parsing code here
  sourceFile.close();
}

void Spa::evaluateQuery(const std::string& query, std::list<std::string>& results) {
  // PQL evaluation code here
}