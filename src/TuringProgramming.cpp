//Version 0.5

#include <iterator>
#include <map>
#include <iostream>

#include "TuringProgramming.h"

//Converts input file to .wb1
int lexWB1(std::string filePath)
{
	std::ifstream file;
	file.exceptions(std::ios::badbit);
	
	std::string inputFilePath = "daedalus/" + filePath + ".ddls";

	file.open(inputFilePath);

	if (file)
	{
		std::ofstream output;

		std::string outputFilePath = "wb1/" + filePath + ".wb1";

		output.open(outputFilePath);
		if (output)
		{
			//Write WB1 header
			std::string line;
			output << "WB1\n";
			getline(file, line);

			//Read in alphabet
			std::vector<char> alphabet;
			for (int i = 0; i < line.length(); i += 2)
			{
				alphabet.push_back(line[i]);
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
						std::cout << "Invalid label at " << std::to_string(lineNum + labelMap.size() + 1) << std::endl;
						output.close();
						file.close();
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
						std::cout << "Invalid label reference at " << std::to_string(lineNum) << std::endl;
						output.close();
						file.close();
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
		std::cout << "Error: Failed to open file: " + filePath + ".ddls" << std::endl;
		return -1;
	}

	return 0;
}

//Converts .wb1 to .tm
int wb1toTM(std::string filePath)
{
	std::ifstream file;
	file.exceptions(std::ios::badbit);
	
	std::string inputFilePath = "wb1/" + filePath + ".wb1";

	file.open(inputFilePath);

	if (file)
	{
		std::ofstream output;

		std::string outputFilePath = "tm/" + filePath + ".tm";

		output.open(outputFilePath);
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

				std::vector<std::string> goTos;
				unsigned int lineNum = 0;

				fpos_t codePosition = file.tellg();

				while (getline(file, line))
				{
					std::istringstream lineStream(line);
					std::vector<std::string> args(std::istream_iterator<std::string>{lineStream},
						std::istream_iterator<std::string>());

					if (args[0] == "2")
					{
						goTos.push_back(args[2]);
					}
					else if (args[0] == "4")
					{
						goTos.push_back(args[1]);
					}
				}

				file.clear();
				file.seekg(codePosition);

				//Maps .wb1 lines to .tm states
				lineNum = 0;
				std::map<std::string, unsigned int> lineMap;

				lineMap["line0"] = 0;

				//template of turing machine table to be filled out
				Table temp = { 0, {}, {}, {} };

				//List of current instructions read in
				std::vector<Instruction> currentInstructions;

				while (getline(file, line))
				{					
					std::istringstream lineStream(line);
					std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, 
						std::istream_iterator<std::string>());

					if (currentInstructions.size() != 0 && 
						(!validOptimization(currentInstructions, args[0]) || 
						(std::find(goTos.begin(), goTos.end(), std::to_string(lineNum)) != goTos.end())))
					{
						//Optimized states
						Table optimizedStates = getOptimizedStates(currentInstructions, alphabet, temp.numStates);

						temp.numStates += optimizedStates.numStates;
						temp.write.insert(temp.write.end(), optimizedStates.write.begin(), optimizedStates.write.end());
						temp.move.insert(temp.move.end(),optimizedStates.move.begin(), optimizedStates.move.end());
						temp.nextState.insert(temp.nextState.end(), optimizedStates.nextState.begin(), optimizedStates.nextState.end());

						lineMap["line" + std::to_string(lineNum)] = temp.numStates;
						currentInstructions.clear();
					}
					switch (std::stoi(args[0]))
					{
						case opAcc:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {} });
							break;
						}
						case opRej:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {} });
							break;
						}
						case opIfGoto:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {args[1],args[2]} });
							break;
						}
						case opWrite:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {args[1]} });
							break;
						}
						case opGoto:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {args[1]} });
							break;
						}
						case opMove:
						{
							currentInstructions.push_back({ std::stoi(args[0]), {args[1]} });
							break;
						}
					}
					++lineNum;
				}

				Table optimizedStates = getOptimizedStates(currentInstructions, alphabet, temp.numStates);

				temp.numStates += optimizedStates.numStates;
				temp.write.insert(temp.write.end(), optimizedStates.write.begin(), optimizedStates.write.end());
				temp.move.insert(temp.move.end(), optimizedStates.move.begin(), optimizedStates.move.end());
				temp.nextState.insert(temp.nextState.end(), optimizedStates.nextState.begin(), optimizedStates.nextState.end());

				lineMap["line" + std::to_string(lineNum)] = temp.numStates;
				currentInstructions.clear();
	
				//Replace labels with mapped states
				for (int i = 0; i < temp.nextState.size(); ++i)
				{
					if (lineMap.find(temp.nextState[i]) != lineMap.end())
					{
						temp.nextState[i] = std::to_string(lineMap[temp.nextState[i]]);
					}
				}

				lookAheadOptimize(temp, alphabet);

				//Write table template to .tm
				output << temp.numStates << "\n";
				for (int c = 0; c < temp.write.size(); ++c)
				{
					output << temp.write[c] << " ";
				}
				output << "\n";
				for (int c = 0; c < temp.move.size(); ++c)
				{
					output << temp.move[c] << " ";
				}
				output << "\n";
				for (int c = 0; c < temp.nextState.size(); ++c)
				{
					output << temp.nextState[c] << " ";
				}
				output << "\n";
			}
			else
			{
				std::cout << "Invalid Header Found.\nExpected: \"WB1\".\nRead: \"" << line << "\"" << std::endl;
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
		std::cout << "Error: Failed to open file: " << filePath << ".wb1" << std::endl;
		return -1;
	}

	return 0;
}

int main(int argc, char** args)
{
	TuringMachine machine;
	machine.tape = { '1','1','1','1','1','0','1','1'};
#ifndef _DEBUG
	if (argc < 2)
	{
		//std::cout << "Please specify a file." << std::endl;
	}
	else
	{
		if (lexWB1(std::string(args[1])) == 0)
		{
			if (wb1toTM(std::string(args[1])) == 0)
			{
				if (machine.load(std::string(args[1])) == 0)
				{
					while (machine.head.state != "r" && machine.head.state != "a")
					{
						machine.step();
					}
					std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
				}
			}
		}
	}
#else
	if (lexWB1("input") == 0)
	{
		if (wb1toTM("input") == 0)
		{
			if (machine.load("input") == 0)
			{
				while (machine.head.state != "r" && machine.head.state != "a")
				{
					machine.step();
				}
				std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
			}
		}
	}
#endif
}