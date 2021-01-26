#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#include "PKB.h"
#include "TNode.h"

class Parser {
private:
	// Pattern for names
	const std::regex namePattern = regex("[A-Za-z](A-Za-z0-9)*");
	const std::regex expPattern = regex("");

	std::stack <std::string> braces;
	std::vector<TNode> building;
	std::unordered_map<std::string, TNode> names;
	std::unordered_set<std::string> procedures;
	TNode root;

	int removeBrace(std::string s) {
		if (braces.empty()) {
			return 1;
		}
		std::string removed = braces.top();
		if (s.compare(")") && removed.compare("(")) {
			braces.pop();
			return 0;
		}
		if (s.compare("}") && removed.compare("{")) {
			braces.pop();
			return 0;
		}
		return 1;
	}

	void addBrace(string s) {
		braces.push(s);
	}

	std::deque<std::string> tokenize(ifstream& stream) {
		// TODO
		std::deque<std::string> tokens;
		std::string str;
		char c;
		while (stream.get(c)) {
			if () {
			}
		}
		return tokens;
	}

	bool isValidName(std::string& str) {
		return std::regex_match(str, namePattern);
	}

	// TODO: Add var to VarTable after successful parsing.
	std::string parseName(std::string& name) {
		if (isValidName(name)) {
			return name;
		}
		throw std::string("Given name [" + name + "] is invalid!");
	}

	// To implement parsing for conditions in if/while
	TNode parseCond(std::deque<std::string>& tokens) {

	}

	// To implement parsing for expression
	TNode parseExp(std::deque<std::string>& tokens) {
		std::string front = tokens.front();
		while (!front.compare(";")) {
			tokens.pop_front();
			// start matching to exp?
			front = tokens.front();
		std::regex_search
		}
		return;
	}

	// change TNode to IfNode
	TNode parseIf(std::deque<std::string>& tokens) {
		tokens.pop_front();
		TNode ifNode = TNode();
		while (!tokens.front().compare("then")) {
			TNode cond = parseCond(tokens);
			// missing: add conds to ifNode
		}
		tokens.pop_front();
		StmtLst ifStmtLst = parseStmtLst(tokens);
		// missing: add stmtLst to ifStmtLst of ifNode
		if (tokens.front().compare("else")) {
			tokens.pop_front();
			StmtLst elseStmtLst = parseStmtLst(tokens);
			// missing: add stmtLst to elseStmtLst of ifNode
		}
		else {
			throw "Missing \"else\" for if statement loop!";
		}
		return ifNode;
	}

	// change TNode to WhileNode
	TNode parseWhile(std::deque<std::string>& tokens) {
		tokens.pop_front();
		TNode whileNode = TNode();
		// recursively call parseCond + parseStmtLst + add stmtLst to whileNode
		while (!tokens.front().compare("{")) {
			TNode cond = parseCond(tokens);
			// missing: add conds to ifNode
		}
		StmtLst stmtLst = parseStmtLst(tokens);
		// missing: add stmtLst to WhileNode
		return whileNode;
	}

	// TODO: change TNode to the respective class for read
	TNode parseRead(std::deque<std::string>& tokens) {
		tokens.pop_front();
		TNode read = TNode(parseName(tokens.front()));
		tokens.pop_front();
		if (tokens.front().compare(";")) {
			tokens.pop_front();
		}
		else {
			throw "Missing \";\"!";
		}
		return read;
	}

	// TODO: change TNode to the respective class for print
	TNode parsePrint(std::deque<std::string>& tokens) {
		tokens.pop_front();
		TNode print = TNode(parseName(tokens.front()));
		tokens.pop_front();
		if (tokens.front().compare(";")) {
			tokens.pop_front();
		}
		else {
			throw "Missing \";\"!";
		}
		return print;
	}

	// Not needed so far - but prob can check aginst a proc list or smth
	TNode parseCall(std::deque<std::string>& tokens) {
		tokens.pop_front();
		return;
	}

	// By concept of abstraction, is done. However still need to implement parseName and parseExp to work
	TNode parseAssign(std::deque<std::string>& tokens) {
		std::string varName = parseName(tokens.front());
		tokens.pop_front();
		TNode assign = TNode();
		if (tokens.front().compare("=")) {
			tokens.pop_front();
			TNode exp = parseExp(tokens);
			if (tokens.front().compare(";")) {
				tokens.pop_front();
				// missing: set lhs of TNode to varName, rhs to exp
				return assign;
			}
			else {
				throw "Missing \";\"!";
			}
		}
		else {
			throw "Not a valid assignment!";
		}
	}

	// Done
	TNode parseStmt(std::deque<std::string>& tokens) {
		std::string keyword = tokens.front();
		TNode stmt;
		if (keyword.compare("if")) {
			TNode stmt = parseIf(tokens);
		}
		else if (keyword.compare("while")) {
			TNode stmt = parseWhile(tokens);
		}
		else if (keyword.compare("read")) {
			TNode stmt = parseRead(tokens);
		}
		else if (keyword.compare("print")) {
			TNode stmt = parsePrint(tokens);
		}
		else if (keyword.compare("call")) {
			TNode stmt = parseCall(tokens);
		}
		else {
			TNode stmt = parseAssign(tokens);
		}
		return stmt;
	}

	// TODO: Create StmtLst classes to contain stmts
	StmtLst parseStmtLst(std::deque<std::string>& tokens) {
		std::string keyword = tokens.front();
		tokens.pop_front();
		StmtLst stmtLst;
		if (keyword.compare("{")) {
			while (!tokens.front().compare("}")) {
				TNode stmt = parseStmt(tokens);
				stmtLst.addStmt(stmt);
			}
			tokens.pop_front();
		}
		else {
			throw "Missing \"{\"!";
		}
	}

	// Done?? - might need to think of classes to encapsulate all the TNode variants
	TNode parseProcedure(std::deque<std::string>& tokens) {
		std::string keyword = tokens.front();
		tokens.pop_front();
		TNode proc;
		if (keyword.compare("procedure")) {
			proc = TNode(parseName(tokens.front()));
			tokens.pop_front();
			StmtLst stmtsLst = parseStmtLst(tokens);
			proc.addStmtLst(stmtsLst);
			if (!tokens.front().compare("}")) {
				throw "Missing \"}\"!";
			}
			tokens.pop_front();
		}
		else {
			throw "Must contain keyword \"procedure\"!";
		}
		return proc;
	}

public:
	// Done by concept of abstraction. However individual methods might not be complete
	TNode parse(ifstream& stream) {
		std::deque<std::string> tokens = tokenize(stream);
		TNode root;
		while (!tokens.empty()) {
			root = parseProcedure(tokens);
		}
		return root;
	}

	int checkCurlyBrace() {
		if (braces.empty()) {
			return 0;
		}
		return 1;
	}
};
