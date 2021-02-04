#include <stdio.h>
#include <iostream>
#include <string>

#include "TableStub.h"

TableStub::TableStub() {}

void TableStub::insertHeader(std::vector<std::string> h) {
	header = h;
}

void TableStub::insertData(std::vector<std::string> d){
	if (header.size() != d.size()) {
		throw "Header size does not match";
	}
	data.insert(d);
}

std::vector<std::string> TableStub::getHeader() {
	return header;
}

std::set<std::vector<std::string>> TableStub::getData() {
	return data;
}