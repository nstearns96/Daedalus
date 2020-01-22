#include "TuringMachine.h"

#include <iostream>

bool isNum(std::string s)
{
	for (size_t c = 0; c < s.length(); ++c)
	{
		if (s[c] != '0' && s[c] != '1' && s[c] != '2' && s[c] != '3' && s[c] != '4' &&
			s[c] != '5' &&  s[c] != '6' && s[c] != '7' && s[c] != '8' && s[c] != '9')
		{
			return false;
		}
	}
	return true;
}

void TuringMachine::step()
{
	int TableOffset = alphabet.size() * stoi(head.state) +
		(std::find(alphabet.begin(), alphabet.end(), tape[head.position]) - alphabet.begin());
	tape[head.position] = table.write[TableOffset];
	head.position += (table.move[TableOffset] == 'r') ? 1 : -1;
	head.state = table.nextState[TableOffset];

	//Extend tape on side if out of bounds
	if (head.position == -1)
	{
		tape.insert(tape.begin(), blankSymbol);
		++head.position;
	}
	else if (head.position == (int)tape.size())
	{
		tape.push_back(blankSymbol);
	}
}

int TuringMachine::load(std::string filePath)
{
	std::ifstream file;
	file.exceptions(std::ios::badbit);

	std::string inputFilePath = "tm/" + filePath + ".tm";
	file.open(inputFilePath);
	std::string line;
	if (file)
	{
		//Check for valid header
		getline(file, line);
		if (line == "TM")
		{
			//Read in alphabet
			getline(file, line);
			blankSymbol = line[0];
			for (size_t i = 0; i < line.length(); i += 2)
			{
				alphabet.push_back(line[i]);
			}

			//Get number of states
			getline(file, line);
			table.numStates = stoi(line);

			//Get Table
			getline(file, line);
			for (size_t i = 0; i < 2 * table.numStates * alphabet.size(); i += 2)
			{
				if (std::find(alphabet.begin(), alphabet.end(), line[i]) != alphabet.end())
				{
					table.write.push_back(line[i]);
				}
				else
				{
					std::cout << "Error: Character not in alphabet.\nRead: " << line[i] << std::endl;
					file.close();
					return -1;
				}
			}

			getline(file, line);
			for (size_t i = 0; i < 2 * table.numStates * alphabet.size(); i += 2)
			{
				if (line[i] == 'r' || line[i] == 'l')
				{
					table.move.push_back(line[i]);
				}
				else
				{
					std::cout << "Error: Invalid move direction.\nRead: " << line[i] << std::endl;
					file.close();
					return -1;
				}
			}

			for (size_t i = 0; i < table.numStates * alphabet.size(); ++i)
			{
				getline(file, line, ' ');
				if (line == "a" || line == "r")
				{
					table.nextState.push_back(line);
				}
				else
				{
					if (isNum(line) && 0U <= stoi(line) && (unsigned int)stoi(line) <= table.numStates)
					{
						table.nextState.push_back(line);
					}
					else
					{
						std::cout << "Error: Invalid State Transition.\nRead: " << line << std::endl;
						file.close();
						return -1;
					}
				}
			}
		}
		else
		{
			std::cout << "Error: Invalid Header Filed.\nExpected: \"TM\".\nRead: " << line << std::endl;
			file.close();
			return -1;
		}

		file.close();
	}
	else
	{
		std::cout << "Error: Failed to open file: " << filePath << ".txt" << std::endl;
		return -1;
	}

	return 0;
}

int TuringMachine::loadTape(std::string filePath)
{
	std::ifstream fin;
	fin.exceptions(std::ios::badbit);

	std::string inputFilePath = "tape/" + filePath + ".txt";
	fin.open(inputFilePath);
	std::string line;
	if (fin)
	{
		std::getline(fin, line);
		for (size_t c = 0; c < line.size(); c += 2)
		{
			tape.push_back(line[c]);
		}
		fin.close();
		return 0;
	}
	else
	{
		std::cout << "Error: Failed to open file: " << filePath << ".txt" << std::endl;
		return -1;
	}
}

void TuringMachine::run(bool isStepsLimited, unsigned int stepLimit, bool isTapeShown)
{
	if (isStepsLimited)
	{
		unsigned int steps = 0;
		while (steps < stepLimit && (head.state != "r" && head.state != "a"))
		{
			++steps;
			step();
			if (isTapeShown)
			{
				printTape();
			}
		}
	}
	else
	{
		while (head.state != "r" && head.state != "a")
		{
			step();
			if (isTapeShown)
			{
				printTape();
			}
		}
	}
}

void TuringMachine::printTape()
{
	std::string tapeString = "";
	for (size_t s = 0; s < tape.size(); ++s)
	{
		tapeString += (std::string{ tape[s] }); 
	}
	std::string headString = "";
	for (int s = 0; s < head.position; ++s)
	{
		headString += " ";
	}
	headString += "^";

	std::cout << tapeString << '\n' << headString << std::endl;
}