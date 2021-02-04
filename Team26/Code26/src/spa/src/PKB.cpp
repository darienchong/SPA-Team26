#include <stdio.h>
#include <iostream>
#include <string>

#include "PKB.h"

PKB::PKB() {}

void PKB::addVar(std::string var) {
	std::vector<std::string> vect{ var };
	varTable.insertRow(vect);
}

void PKB::addStmt(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	stmtTable.insertRow(vect);
}

void PKB::addProc(std::string proc) {
	std::vector<std::string> vect{ proc };
	procTable.insertRow(vect);
}

void PKB::addIf(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	ifTable.insertRow(vect);
}

void PKB::addWhile(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	whileTable.insertRow(vect);
}

void PKB::addRead(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	readTable.insertRow(vect);
}

void PKB::addPrint(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	printTable.insertRow(vect);
}

void PKB::addAssign(int stmtNo) {
	std::vector<std::string> vect{ std::to_string(stmtNo) };
	assignTable.insertRow(vect);
}

void PKB::addFollows(int follower, int followed) {
	std::vector<std::string> vect{ std::to_string(follower), std::to_string(followed) };
	followsTable.insertRow(vect);
}

void PKB::addParent(int parent, int child) {
	std::vector<std::string> vect{ std::to_string(parent), std::to_string(child) };
	parentTable.insertRow(vect);
}

void PKB::addUses(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	usesTable.insertRow(vect);
}

void PKB::addUses(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	usesTable.insertRow(vect);
}

void PKB::addModifies(int stmtNo, std::string var) {
	std::vector<std::string> vect{ std::to_string(stmtNo), var };
	modifiesTable.insertRow(vect);
}

void PKB::addModifies(std::string proc, std::string var) {
	std::vector<std::string> vect{ proc , var };
	modifiesTable.insertRow(vect);
}