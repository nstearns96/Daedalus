#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "TuringMachine.h"

#include <string>
#include <vector>

enum OpCodes { opAcc = 0, opRej, opIfGoto, opWrite, opGoto, opMove };

class Instruction
{
private:
	int instructionCode;
	std::vector<std::string> arguments;

public:
	Instruction(int code, std::vector<std::string> arguments);
	int getCode() const;
	std::vector<std::string> getArgs() const;
};

bool validOptimization(const std::vector<Instruction> &ins, std::string arg);

Table getOptimizedStates(const std::vector<Instruction> &ins, const std::vector<char> &alphabet, unsigned int numStates);

void lookAheadOptimize(Table &table, const std::vector<char> &alphabet);

#endif