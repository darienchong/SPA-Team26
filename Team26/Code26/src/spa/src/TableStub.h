#pragma once

#include <vector>
#include <set>

class TableStub {
public:
	TableStub();
	void insertHeader(std::vector<std::string> header);
	void insertData(std::vector<std::string> data);
	std::vector<std::string> getHeader();
	std::set<std::vector<std::string>> getData();

private:
	std::vector<std::string> header;
	std::set<std::vector<std::string>> data;
};