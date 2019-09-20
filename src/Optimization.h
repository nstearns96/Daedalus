#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "TuringMachine.h"

#include <string>
#include <vector>

enum OpCodes { opAcc = 0, opRej, opIfGoto, opWrite, opGoto, opMove };

enum class OptimizationParameter {OP_IF_GOTO_NONE = 0, OP_IF_GOTO_ARG, OP_WRITE_NONE, OP_WRITE_ARG, OP_MOVE_NONE, OP_MOVE_ARG, OP_GOTO_NONE, OP_GOTO_ARG, OP_GOTO_ACC, OP_GOTO_REJ};

struct OptimizationTemplate
{
	OptimizationParameter ifGotoParam;
	std::vector<char> alphabetSplit;
	std::vector<int> ifGotoArgs;

	OptimizationParameter writeParam;
	char writeArg;

	OptimizationParameter moveParam;
	char moveArg;

	OptimizationParameter gotoParam;
	int gotoArg;
};

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