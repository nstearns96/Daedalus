//Version 0.6

#include <iterator>
#include <map>
#include <iostream>
#include <algorithm>

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
				if (!args.empty())
				{
					if (args[0][args[0].length() - 1] == ':')
					{
						if (args[0] != ":")
						{
							std::string label = args[0].substr(0, args[0].length() - 1);
							if (labelMap.find(label) == labelMap.end())
							{
								labelMap[label] = lineNum;
							}
							else
							{
								std::cout << "Repeated label: " << label << " at " << lineNum << " and " << labelMap[label];
								output.close();
								file.close();
								return -1;
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
			}

			file.clear();
			file.seekg(0, file.beg);
			lineNum = 1;

			while (std::getline(file, line))
			{
				std::istringstream lineStream(line);
				std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, std::istream_iterator<std::string>());
				if (!args.empty())
				{
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
int wb1toTM(std::string filePath, int optimizationLevel)
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

				if (optimizationLevel > 0)
				{
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
							temp.move.insert(temp.move.end(), optimizedStates.move.begin(), optimizedStates.move.end());
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

					for (int c = 0; c < optimizedStates.nextState.size(); ++c)
					{
						if (optimizedStates.nextState[c] != "r" && optimizedStates.nextState[c] != "a")
						{
							if (std::stoi(optimizedStates.nextState[c]) >= optimizedStates.numStates)
							{
								optimizedStates.nextState[c] = "a";
							}
						}
					}

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

					if (optimizationLevel == 2)
					{
						lookAheadOptimize(temp, alphabet);
					}
				}
				else
				{

					while (getline(file, line))
					{
						std::istringstream lineStream(line);
						std::vector<std::string> args(std::istream_iterator<std::string>{lineStream},
							std::istream_iterator<std::string>());

						switch (std::stoi(args[0]))
						{
							case opAcc:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back("a");
								}
								temp.numStates += 2;
								break;
							}
							case opRej:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back("r");
								}
								temp.numStates += 2;
								break;
							}
							case opIfGoto:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									if (args[1][0] == alphabet[c])
									{
										temp.nextState.push_back("line" + args[2]);
									}
									temp.nextState.push_back(std::to_string(temp.numStates + 2));
								}
								temp.numStates += 2;
								break;
							}
							case opWrite:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(args[1][0]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back(std::to_string(temp.numStates + 2));
								}
								temp.numStates += 2;
								break;
							}
							case opGoto:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back("line" + args[1]);
								}
								temp.numStates += 2;
								break;
							}
							case opMove:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('r');
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								temp.numStates += 1;
								break;
							}
						}
						lineMap["line" + std::to_string(lineNum + 1)] = temp.numStates;
						++lineNum;
					}

					for (int i = 0; i < temp.nextState.size(); ++i)
					{
						if (lineMap.find(temp.nextState[i]) != lineMap.end())
						{
							temp.nextState[i] = std::to_string(lineMap[temp.nextState[i]]);
						}
					}
				}

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

int interpretCommandLineArgs(int argc, char** args)
{
	if (argc >= 2)
	{
		std::string command{ args[1] };
		if (command == "run")
		{
			if (argc >= 4)
			{
				if (args[2][0] == '-' || args[3][0] == '-')
				{
					std::cout << "Error: Invalid path variable" << (args[2][0] == '-' ? args[3] : args[2]) << std::endl;
					return -1;
				}

				TuringMachine machine;

				if (machine.load(args[2]) == 0)
				{
					if (machine.loadTape(args[3]) == 0)
					{
						if (argc >= 5)
						{
							if (argc == 6)
							{
								if (args[4][0] == '-' || args[5][0] == '-')
								{
									if (std::string{ args[4] } == "-S" || std::string{ args[5] } == "-S")
									{
										std::string stepLimitString = std::string{ args[4] } == "-S" ? std::string{ args[5] } : std::string{ args[4] };
										if (isNum(stepLimitString.substr(1, stepLimitString.size() - 1)))
										{
											unsigned int stepLimit = std::stoi(stepLimitString.substr(1, stepLimitString.size() - 1));
											machine.run(true, stepLimit, true);

											if (machine.head.state != "r" && machine.head.state != "a")
											{
												std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
											}
										}
										else
										{
											std::cout << "Error: Invalid step limit flag " << stepLimitString << std::endl;
										}
									}
									else
									{
										std::cout << "Error: Invalid flag " << (std::string{ args[4] } == "-S" ? std::string{ args[4] } : std::string{ args[5] }) << std::endl;
									}
								}
							}
							else if (argc == 5)
							{
								std::string flagString = std::string{ args[4] };
								if (flagString == "-S")
								{
									machine.run(false, 0, true);
										
									if (machine.head.state != "r" && machine.head.state != "a")
									{
										std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
									}
								}
								else if (isNum(flagString.substr(1, flagString.size() - 1)))
								{
									unsigned int stepLimit = std::stoi(flagString.substr(1, flagString.size() - 1));
									machine.run(true, stepLimit, false);
										
									if (machine.head.state != "r" && machine.head.state != "a")
									{
										std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
									}
								}
								else
								{
									std::cout << "Invalid flag " << flagString << std::endl;
								}
								}
							else
							{
								std::cout << "Too many arguments!" << std::endl;
							}
						}
					}	
					else
					{	
						return -1;
					}
				}
				else
				{
					return -1;
				}
			}
			else
			{
				if (argc == 2)
				{
					std::cout << "Error: Please specify a source path for the Turing Machine" << std::endl;
					return -1;
				}
				else if (argc == 3)
				{
					std::cout << "Error: Please specify a source path for the inital tape" << std::endl;
					return -1;
				}
			}
		}
		else if (command == "compile")
		{
			if (argc >= 5)
			{
				std::string sourceFileTypeString{ args[2] };
				std::string outputFileTypeString{ args[3] };
				if ((sourceFileTypeString == "ddls" || sourceFileTypeString == "wb1") && (outputFileTypeString == "wb1" || outputFileTypeString == "tm") &&
					(sourceFileTypeString != outputFileTypeString))
				{
					if (sourceFileTypeString == "ddls")
					{
						lexWB1(args[4]);
						if (outputFileTypeString == "tm")
						{
							int optimizationLevel = 0;
							if (argc == 6)
							{
								std::string flagString{ args[5] };
								if ((flagString == "-O0" || flagString == "-O1" || flagString == "-O2"))
								{
									optimizationLevel = std::stoi(flagString.substr(2, 1));
								}
								else
								{
									std::cout << "Error: Invalid flag " << flagString << std::endl;
									return -1;
								}
								wb1toTM(args[4], optimizationLevel);
							}
						}
					}
					else
					{
						int optimizationLevel = 0;
						if (argc == 6)
						{
							std::string flagString{ args[5] };
							if ((flagString == "-O0" || flagString == "-O1" || flagString == "-O2"))
							{
								optimizationLevel = std::stoi(flagString.substr(2, 1));
							}
							else
							{
								std::cout << "Error: Invalid flag " << flagString << std::endl;
								return -1;
							}
							wb1toTM(args[4], optimizationLevel);
						}
					}
				}
				else
				{
					std::cout << "Error: Invalid compile instruction" << std::endl;
				}
			}
		}
		else
		{
			std::cout << "Error: Invalid Command: " << command << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << "Please specify a command\n" <<
			"<run|compile>:\n" <<
			"\tcompile <ddls|wb1> <wb1|tm> <inputFile> -optimizationLevel(-Ox)\n" <<
			"\trun <inputFile> -stepLimit(-#) -showTape(-S)" << std::endl;
		return -1;
	}

	return 0;
}

int main(int argc, char** args)
{
	interpretCommandLineArgs(argc, args);
/*
	std::string filePath{ args[1] };

	if (lexWB1(filePath) == 0)
	{
		if (wb1toTM(filePath, 2) == 0)
		{
			if (machine.load(filePath) == 0)
			{
				if (machine.loadTape("initTape") == 0)
				{
					while (machine.head.state != "r" && machine.head.state != "a")
					{
						machine.step();
					}
					std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
				}
			}
		}
	}*/

	return 0;
}