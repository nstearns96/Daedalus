#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct Head
{
	std::string state = "0";
	int position = 0;
};

struct Table
{
	unsigned int numStates;
	std::vector<char> write;
	std::vector<char> move;
	std::vector<std::string> nextState;
};

struct TuringMachine
{
	Table table;
	Head head;
	std::vector<char> tape;
	std::vector<char> alphabet;
	char blankSymbol;

	//Does a single step of the turing machine
	void step();

	//Load .tm file into turing machine
	int load(std::string filePath);

	int loadTape(std::string filePath);

	void run(bool isStepLimited, int stepLimit, bool isTapeShown);

	void printTape();
};

bool isNum(std::string s);

#endif