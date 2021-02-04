#include <stdio.h>
#include <iostream>
#include <string>

#include "PKB.h"

PKB::PKB() {}

void PKB::addVar(std::string var) {
	std::vector<std::string> vect{ var };
	varTable.insertData(vect);
}

void PKB::addStmt(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	stmtTable.insertData(vect);
}

void PKB::addProc(std::string proc) {
	std::vector<std::string> vect{ proc };
	procTable.insertData(vect);
}

void PKB::addIf(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	ifTable.insertData(vect);
}

void PKB::addWhile(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	whileTable.insertData(vect);
}

void PKB::addRead(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	readTable.insertData(vect);
}

void PKB::addPrint(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	printTable.insertData(vect);
}

void PKB::addAssign(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	assignTable.insertData(vect);
}

void PKB::addFollows(int follower, int followed) {
	std::vector<std::string> vect{ std::to_string(follower), std::to_string(followed) };
	followsTable.insertData(vect);
}

void PKB::addParent(int parent, int child) {
	std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
	parentTable.insertData(vect);
}

void PKB::addUses(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	usesTable.insertData(vect);
}

void PKB::addUses(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	usesTable.insertData(vect);
}

void PKB::addModifies(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	modifiesTable.insertData(vect);
}

void PKB::addModifies(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	modifiesTable.insertData(vect);
}