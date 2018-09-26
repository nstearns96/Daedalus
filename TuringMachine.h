#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct head
{
	std::string State = "0";
	int Position = 0;
};

struct table
{
	unsigned int NumStates;
	std::vector<char> Write;
	std::vector<char> Move;
	std::vector<std::string> NextState;
};

struct turing_machine
{
	table Table;
	head Head;
	std::vector<char> Tape;
	std::vector<char> Alphabet;
	char BlankSymbol;

	//Does a single step of the turing machine
	void Step();

	//Load .tm file into turing machine
	int Load(std::string FilePath);
};

#endif