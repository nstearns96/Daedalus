//Version 0.3

#include <iterator>
#include <map>
#include "TuringProgramming.h"


//Converts input file to .wb1
int lexWB1(std::string filePath)
{
	std::ifstream file;
	file.exceptions(std::ios::badbit);
	file.open(filePath);

	if (file)
	{
		std::ofstream output;
		output.open("output.wb1");
		if (output)
		{
			//Write WB1 header
			std::string line;
			output << "WB1\n";
			getline(file, line);

			//Read in alphabet
			std::vector<char> alphabet;
			for (int I = 0; I < line.length(); I += 2)
			{
				alphabet.push_back(line[I]);
			}

			//Write alphabet
			output << line + '\n';

			//Parse labels
			std::map<std::string, unsigned int> labelMap;
			unsigned int lineNum = 0;
			while (std::getline(file, line))
			{
				std::istringstream lineStream(line);
				std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, std::istream_iterator<std::string>());
				if (args[0][args[0].length() - 1] == ':')
				{
					if (args[0] != ":")
					{
						std::string label = args[0].substr(0, args[0].length() - 1);
						if (labelMap.find(label) == labelMap.end())
						{
							labelMap[label] = lineNum;
						}
					}
					else
					{
						printf(("Invalid label at " + std::to_string(lineNum + labelMap.size()+1)).c_str());
						return -1;
					}
				}
				else
				{
					++lineNum;
				}
			}

			file.clear();
			file.seekg(0, file.beg);
			lineNum = 1;

			while (std::getline(file, line))
			{
				std::istringstream lineStream(line);
				std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, std::istream_iterator<std::string>());
				if (args[0] == "accept") // accept
				{
					output << "0\n";
				}
				else if (args[0] == "reject") //reject
				{
					output << "1\n";
				}
				else if (args[0] == "if") //if-goto
				{
					if (labelMap.find(args[3]) != labelMap.end())
					{
						output << "2 " << std::to_string(std::find(alphabet.begin(), alphabet.end(), args[1][0]) - alphabet.begin())
							<< " " << labelMap[args[3]] << "\n";
					}
					else
					{
						printf(("Invalid label reference at " + std::to_string(lineNum)).c_str());
						return -1;
					}
				}
				else if (args[0] == "write") //write
				{
					output << "3 " 
						<< std::to_string(std::find(alphabet.begin(), alphabet.end(), args[1][0]) - alphabet.begin())
						<< "\n";
				}
				else if (args[0] == "goto") //goto
				{
					if (labelMap.find(args[1]) != labelMap.end())
					{
						output << "4 " << labelMap[args[1]] << "\n";
					}
				}
				else if (args[0] == "move") //move
				{
					output << "5 " << args[1] << "\n";
				}
				++lineNum;
			}
			output.close();
		}

		file.close();
	}
	else
	{
		printf("Error: Failed to open file!");
		return -1;
	}

	return 0;
}

//Converts .wb1 to .tm
int wb1toTM(std::string FilePath)
{
	std::ifstream file;
	file.exceptions(std::ios::badbit);
	file.open(FilePath);

	if (file)
	{
		std::ofstream output;
		output.open("output.tm");
		if (output)
		{
			//Check for valid header
			std::string line;
			getline(file, line);

			if (line == "WB1")
			{
				//Write header to file
				output << "TM\n";

				//Read in alphabet
				getline(file, line);
				std::vector<char> alphabet;
				for (int I = 0; I < line.length(); I += 2)
				{
					alphabet.push_back(line[I]);
				}

				//Write alphabet
				output << line + '\n';

				//Maps .wb1 lines to .tm states
				unsigned int lineNum = 0;
				std::map<unsigned int, unsigned int> lineMap;

				//TODO: Add functionality for string labels
				//Queue for labels to be process
				std::vector<unsigned int> labelQueue;
				unsigned int labelIter = 0;

				//template of turing machine table to be filled out
				unsigned int numStates = 0;
				std::vector<char> write;
				std::vector<char> move;
				std::vector<std::string> nextState;

				while (getline(file, line))
				{					
					std::istringstream lineStream(line);
					std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, 
						std::istream_iterator<std::string>());
					if (args[0] == "0") // Accept
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('r');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back("a");
						}
						lineMap[lineNum++] = numStates;
						++numStates;
					}
					else if (args[0] == "1") //Reject
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('r');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back("r");
						}
						lineMap[lineNum++] = numStates;
						++numStates;
					}
					else if (args[0] == "2") // If-goto
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('r');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('l');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('l');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							if (c == stoi(args[1]))
								nextState.push_back(std::to_string(numStates + 1));
							else
								nextState.push_back(std::to_string(numStates + 2));
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back("temp");
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back(std::to_string(numStates+3));
						}
						labelQueue.push_back(stoi(args[2]));
						lineMap[lineNum++] = numStates;
						numStates += 3;
					}
					else if (args[0] == "3") // write
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[stoi(args[1])]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('r');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('l');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back(std::to_string(numStates + 1));
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back(std::to_string(numStates + 2));
						}
						lineMap[lineNum++] = numStates;
						numStates += 2;
					}
					else if (args[0] == "4") //goto
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('r');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back('l');
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back(std::to_string(numStates + 1));
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back("temp");
						}
						labelQueue.push_back(stoi(args[1]));
						lineMap[lineNum++] = numStates;
						numStates += 2;
					}
					else if (args[0] == "5") // Move
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							write.push_back(alphabet[c]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							move.push_back(args[1][0]);
						}

						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState.push_back(std::to_string(numStates+1));
						}
						lineMap[lineNum++] = numStates;
						++numStates;
					}
				}

				//Replace labels with mapped states
				while (!labelQueue.empty())
				{
					if (nextState[labelIter] == "temp")
					{
						for (int c = 0; c < alphabet.size(); ++c)
						{
							nextState[labelIter+c] = std::to_string(lineMap[labelQueue[0]]);
						}
						labelQueue.erase(labelQueue.begin());
					}
					++labelIter;
				}
				//Write table template to .tm
				output << numStates << "\n";
				for (int c = 0; c < write.size(); ++c)
				{
					output << write[c] << " ";
				}
				output << "\n";
				for (int c = 0; c < move.size(); ++c)
				{
					output << move[c] << " ";
				}
				output << "\n";
				for (int c = 0; c < nextState.size(); ++c)
				{
					output << nextState[c] << " ";
				}
				output << "\n";
			}
			else
			{
				printf("Error: Invalid Header Filed.\nExpected: \"WB1\".\nRead: %s\n", line.c_str());
				output.close();
				file.close();
				return -1;
			}

			output.close();
		}

		file.close();
	}
	else
	{
		printf("Error: Failed to open file!");
		return -1;
	}

	return 0;
}

void main() 
{
	TuringMachine machine;
	machine.tape = { '1','1','0','1','1','1','1','1' };
	if (lexWB1("input.txt") == 0)
	{
		if (wb1toTM("output.wb1") == 0)
		{
			if (machine.load("output.tm") == 0)
			{
				while (machine.head.state != "r" && machine.head.state != "a")
				{
					machine.step();
				}
				printf("%s\n", machine.head.state == "r" ? "Rejected" : "Approved");
			}
		}
	}
}