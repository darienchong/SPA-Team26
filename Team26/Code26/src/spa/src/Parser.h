#pragma once
#include <fstream>
#include <TNode.h>

class Parser {
public:
	Parser();
	~Parser();

	TNode parse(ifstream& stream);
};