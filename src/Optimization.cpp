#include "Optimization.h"

#include <set>

Instruction::Instruction(int code, std::vector<std::string> args)
{
	instructionCode = code;
	arguments = args;
}

int Instruction::getCode() const
{
	return instructionCode;
}

std::vector<std::string> Instruction::getArgs() const
{
	return arguments;
}

bool validOptimization(const std::vector<Instruction> &ins, std::string arg)
{
	if (ins.size() == 0)
		return true;
	switch (ins[ins.size() - 1].getCode())
	{
		case opAcc:
		{
			return false;
			break;
		}
		case opRej:
		{
			return false;
			break;
		}
		case opGoto:
		{
			return false;
			break;
		}
		case opIfGoto:
		{
			if (arg == "0" || arg == "1" || arg == "4" || arg == "2")
			{
				return true;
			}
			else
			{
				for (int i = 0; i < ins.size(); ++i)
				{
					if (ins[i].getCode() == std::stoi(arg))
					{
						return false;
					}
				}
				return true;
			}
			break;
		}
		case opWrite:
		{
			if (arg == "0" || arg == "1" || arg == "4" || arg == "5")
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		}
		case opMove:
		{
			if (arg == "0" || arg == "1" || arg == "4")
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		}
	}
}

Table createOptimizedStates(const OptimizationTemplate& temp, const std::vector<char> &alphabet, unsigned int numStates)
{
	Table result = { 0, {},{},{} };
	result.numStates = 2 + (temp.moveParam == OptimizationParameter::OP_MOVE_ARG ? -1 : 0) + (temp.ifGotoParam == OptimizationParameter::OP_IF_GOTO_ARG ? temp.alphabetSplit.size() : 0);
	int numStatesMade = (temp.moveParam == OptimizationParameter::OP_MOVE_ARG ? 0 : 1);
	for (int c = 0; c < alphabet.size(); ++c)
	{
		if (std::find(temp.alphabetSplit.begin(), temp.alphabetSplit.end(), alphabet[c]) == temp.alphabetSplit.end())
		{
			if (temp.writeParam == OptimizationParameter::OP_WRITE_NONE)
			{
				result.write.push_back(alphabet[c]);
			}
			else if(temp.writeParam == OptimizationParameter::OP_WRITE_ARG)
			{
				result.write.push_back(temp.writeArg);
			}

			if (temp.moveParam == OptimizationParameter::OP_MOVE_NONE)
			{
				result.move.push_back('r');
				result.nextState.push_back(std::to_string(numStates + 1));
			}
			else if (temp.moveParam == OptimizationParameter::OP_MOVE_ARG)
			{
				result.move.push_back(temp.moveArg);

				if (temp.gotoParam == OptimizationParameter::OP_GOTO_NONE)
				{
					result.nextState.push_back(std::to_string(numStates + result.numStates));
				}
				else if (temp.gotoParam == OptimizationParameter::OP_GOTO_ACC)
				{
					result.nextState.push_back("a");
				}
				else if (temp.gotoParam == OptimizationParameter::OP_GOTO_REJ)
				{
					result.nextState.push_back("r");
				}
				else if (temp.gotoParam == OptimizationParameter::OP_GOTO_ARG)
				{
					result.nextState.push_back("line" + std::to_string(temp.gotoArg));
				}
			}
		}
		else
		{
			result.write.push_back(alphabet[c]);
			result.move.push_back('r');
			result.nextState.push_back(std::to_string(numStates + ++numStatesMade));
		}
	}

	if (temp.moveParam == OptimizationParameter::OP_MOVE_NONE)
	{
		for (int c = 0; c < alphabet.size(); ++c)
		{
			result.write.push_back(alphabet[c]);
			result.move.push_back('l');
			if (temp.gotoParam == OptimizationParameter::OP_GOTO_NONE)
			{
				result.nextState.push_back(std::to_string(numStates + result.numStates));
			}
			else if (temp.gotoParam == OptimizationParameter::OP_GOTO_ACC)
			{
				result.nextState.push_back("a");
			}
			else if (temp.gotoParam == OptimizationParameter::OP_GOTO_REJ)
			{
				result.nextState.push_back("r");
			}
			else if (temp.gotoParam == OptimizationParameter::OP_GOTO_ARG)
			{
				result.nextState.push_back("line" + std::to_string(temp.gotoArg));
			}
		}
	}

	if (temp.ifGotoParam == OptimizationParameter::OP_IF_GOTO_ARG)
	{
		for (int s = 0; s < temp.alphabetSplit.size(); ++s)
		{
			for (int c = 0; c < alphabet.size(); ++c)
			{
				result.write.push_back(alphabet[c]);
				result.move.push_back('l');
				result.nextState.push_back("line" + std::to_string(temp.ifGotoArgs[s]));
			}
		}
	}

	return result;
}

Table getOptimizedStates(const std::vector<Instruction> &ins, const std::vector<char> &alphabet, unsigned int numStates)
{
	Table result = { 0, {}, {}, {} };
	if (ins.size() > 0)
	{

		OptimizationTemplate temp = { OptimizationParameter::OP_IF_GOTO_NONE, {}, {}, OptimizationParameter::OP_WRITE_NONE, 0, 
			OptimizationParameter::OP_MOVE_NONE, 'r', OptimizationParameter::OP_GOTO_NONE, 0};

		for (int i = 0; i < ins.size(); ++i)
		{
			switch (ins[i].getCode())
			{
				case opAcc:
				{
					temp.gotoParam = OptimizationParameter::OP_GOTO_ACC;
					break;
				}
				case opRej:
				{
					temp.gotoParam = OptimizationParameter::OP_GOTO_REJ;
					break;
				}
				case opIfGoto:
				{
					temp.ifGotoParam = OptimizationParameter::OP_IF_GOTO_ARG;
					temp.alphabetSplit.push_back(alphabet[std::stoi(ins[i].getArgs()[0])]);
					temp.ifGotoArgs.push_back(std::stoi(ins[i].getArgs()[1]));
					break;
				}
				case opWrite:
				{
					temp.writeParam = OptimizationParameter::OP_WRITE_ARG;
					temp.writeArg = alphabet[std::stoi(ins[i].getArgs()[0])];
					break;
				}
				case opMove:
				{
					temp.moveParam = OptimizationParameter::OP_MOVE_ARG;
					temp.moveArg = ins[i].getArgs()[0][0];
					break;
				}
				case opGoto:
				{
					temp.gotoParam = OptimizationParameter::OP_GOTO_ARG;
					temp.gotoArg = std::stoi(ins[i].getArgs()[0]);
					break;
				}
			}
		}
		result = createOptimizedStates(temp, alphabet, numStates);
	}

	return result;
}

void lookAheadOptimize(Table &table, const std::vector<char> &alphabet)
{
	std::vector<int> stateQueue{0};
	std::vector<int> completedStates;

	//Acellerate states
	while (!stateQueue.empty())
	{
		int currentState = stateQueue[0];
		completedStates.push_back(currentState);
		stateQueue.erase(stateQueue.begin());

		for (int currentChar = 0; currentChar < alphabet.size(); ++currentChar)
		{
			bool isCurrentStateIdenticalWrite = (table.write[alphabet.size()*currentState + currentChar] == alphabet[currentChar]);
			if ((table.nextState[alphabet.size()*currentState + currentChar] != "r") && 
				(table.nextState[alphabet.size()*currentState + currentChar] != "a"))
			{
				char moveDir = table.move[alphabet.size()*currentState + currentChar];
				int nextState = std::stoi(table.nextState[alphabet.size()*currentState + currentChar]);
				
				bool isIdenticalWrite = true;
				bool isMovingBack = true;
				bool isNextStateSame = true;

				std::string nextStateNextState = table.nextState[alphabet.size() * nextState];
				for (int nextStateChar = 0; nextStateChar < alphabet.size(); ++nextStateChar)
				{
					if (table.write[alphabet.size()*nextState + nextStateChar] != alphabet[nextStateChar])
					{
						isIdenticalWrite = false;
						break;
					}
					if (table.move[alphabet.size()*nextState + nextStateChar] != ((moveDir == 'r') ? 'l' : 'r'))
					{
						isMovingBack = false;
						break;
					}
					if (table.nextState[alphabet.size()*nextState + nextStateChar] != nextStateNextState)
					{
						isNextStateSame = false;
						break;
					}
				}

				if (isIdenticalWrite && isMovingBack && isNextStateSame)
				{
					if (nextStateNextState == "r" || nextStateNextState == "a")
					{
						table.nextState[alphabet.size()*currentState + currentChar] = nextStateNextState;
					}
					else// if(isCurrentStateIdenticalWrite)
					{
						bool madeChange = false;
						int stateWriteAlphabetOffset = std::find(alphabet.begin(), alphabet.end(), table.write[alphabet.size()*currentState + currentChar]) - alphabet.begin();
						if (table.write[alphabet.size()*currentState + currentChar] != table.write[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset] ||
							table.move[alphabet.size()*currentState + currentChar] != table.move[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset] ||
							table.nextState[alphabet.size()*currentState + currentChar] != table.nextState[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset])
						{
							madeChange = true;
						}
						table.write[alphabet.size()*currentState + currentChar] = table.write[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset];
						table.move[alphabet.size()*currentState + currentChar] = table.move[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset];
						table.nextState[alphabet.size()*currentState + currentChar] = table.nextState[alphabet.size()*std::stoi(nextStateNextState) + stateWriteAlphabetOffset];
						if (madeChange)
						{
							currentChar = -1;
							continue;
						}
					}
				}

				if (table.nextState[alphabet.size()*currentState + currentChar] != "a" && 
					table.nextState[alphabet.size()*currentState + currentChar] != "r" && 
					std::find(completedStates.begin(), completedStates.end(), std::stoi(table.nextState[alphabet.size()*currentState + currentChar])) == completedStates.end() &&
					std::find(stateQueue.begin(), stateQueue.end(), std::stoi(table.nextState[alphabet.size()*currentState + currentChar])) == stateQueue.end())
				{
					stateQueue.push_back(std::stoi(table.nextState[alphabet.size()*currentState + currentChar]));
				}
			}
		}
	}

	//condense repeated states
	std::vector<std::set<int>> identicalStates;
	for (int s = 0; s < table.numStates; ++s)
	{
		for (int n = s + 1; n < table.numStates; ++n)
		{
			bool areStatesSame = true;
			for (int c = 0; c < alphabet.size(); ++c)
			{
				if (table.write[alphabet.size()*s + c] != table.write[alphabet.size()*n + c] ||
					table.move[alphabet.size()*s + c] != table.move[alphabet.size()*n + c] ||
					table.nextState[alphabet.size()*s + c] != table.nextState[alphabet.size()*n + c])
				{
					areStatesSame = false;
					break;
				}
			}

			if (areStatesSame)
			{
				bool isStateEmplaced = false;
				for (int v = 0; v < identicalStates.size(); ++v)
				{
					if (std::find(identicalStates[v].begin(), identicalStates[v].end(), s) != identicalStates[v].end())
					{
						identicalStates[v].insert(n);
						isStateEmplaced = true;
						break;
					}
				}
				if (!isStateEmplaced)
				{
					identicalStates.push_back(std::set<int>{{s,n}});
				}
			}
		}
	}

	//Change references to identical state with the lowest state number
	for (int next = 0; next < table.nextState.size(); ++next)
	{
		if (table.nextState[next] != "r" && table.nextState[next] != "a")
		{
			for (int v = 0; v < identicalStates.size(); ++v)
			{
				if (identicalStates[v].find(std::stoi(table.nextState[next])) != identicalStates[v].end())
				{
					table.nextState[next] = std::to_string(*identicalStates[v].begin());
				}
			}
		}
	}

	//Eliminate all unreferenced states
	std::set<int> referencedStates;
	std::set<int> removedStates;
	std::vector<int> stateMapping;
	for (int s = 0; s < table.numStates; ++s)
	{
		stateMapping.push_back(s);
	}
	int currentNumStates = table.numStates;
	bool areAllStatesReferenced = false;
	while (!areAllStatesReferenced)
	{
		referencedStates.insert(0);
		areAllStatesReferenced = true;
		for (int next = 0; next < table.nextState.size(); ++next)
		{
			if (table.nextState[next] != "r" && 
				table.nextState[next] != "a")
			{
				referencedStates.insert(std::stoi(table.nextState[next]));
			}
		}
		for (int s = 1; s < currentNumStates; ++s)
		{
			if (referencedStates.find(s) == referencedStates.end() && (removedStates.find(s) == removedStates.end()))
			{
				areAllStatesReferenced = false;
				int mappedState = std::find(stateMapping.begin(), stateMapping.end(), s) - stateMapping.begin();
				for (int c = 0; c < alphabet.size(); ++c)
				{
					table.write.erase(table.write.begin() + alphabet.size()*(mappedState));
					table.move.erase(table.move.begin() + alphabet.size()*(mappedState));
					table.nextState.erase(table.nextState.begin() + alphabet.size()*(mappedState));
				}
				removedStates.insert(s);
				--table.numStates;
				stateMapping.erase(std::find(stateMapping.begin(), stateMapping.end(), s));
			}
		}
		referencedStates.clear();
	}

	//Remap states
	for (int next = 0; next < table.nextState.size(); ++next)
	{
		if (table.nextState[next] != "r" && table.nextState[next] != "a")
		{
			table.nextState[next] = std::to_string(std::find(stateMapping.begin(), stateMapping.end(), std::stoi(table.nextState[next])) - stateMapping.begin());
		}
	}
}