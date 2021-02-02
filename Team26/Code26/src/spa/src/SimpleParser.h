#pragma once
#include <fstream>
#include <TNode.h>

class SimpleParser {
public:
  SimpleParser();
  ~SimpleParser();

  TNode parse(ifstream& stream);
};