//Version 0.7

#include <iterator>
#include <map>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <iostream>

#include "TuringProgramming.h"

namespace fs = std::filesystem;

std::map<std::string, std::ifstream> loadDirectoryToMap(std::string filePath)
{
	std::map<std::string, std::ifstream> result;
	fs::directory_iterator directory("daedalus\\" + filePath);
	for (fs::directory_iterator file = fs::begin(directory); file != fs::end(directory); ++file)
	{
		if (file->is_regular_file())
		{
			result[file->path().string()] = std::ifstream(file->path());
		}
	}

	return result;
}

//Converts input file to .wb1
int lexWB1(std::string filePath)
{	
	std::map<std::string, std::ifstream> directoryMap = loadDirectoryToMap(filePath);

	bool areFilesOpen = true;
	std::string unopenedFiles = "";
	for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
	{
		if (!iter->second)
		{
			areFilesOpen = false;
			unopenedFiles += iter->first + " ";
		}
	}

	if (areFilesOpen)
	{

		std::map<std::string, std::string> lineIters;
		for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
		{
			lineIters[iter->first] = "";
		}

		bool isMainFailOpen = false;
		std::map<std::string, std::ifstream>::iterator mainFile;
		for (std::map<std::string, std::string>::iterator iter = lineIters.begin(); iter != lineIters.end(); ++iter)
		{
			if (iter->first.substr(iter->first.size() - 9, 9) == "main.ddls")
			{
				isMainFailOpen = true;
				mainFile = directoryMap.find(iter->first);
				break;
			}
		}

		if (isMainFailOpen)
		{
			std::vector<std::string> standardFileRefs;

			std::string labelLine;
			while (getline(mainFile->second, labelLine))
			{
				std::istringstream lineStream(labelLine);
				std::vector<std::string> args(std::istream_iterator<std::string>{lineStream},
					std::istream_iterator<std::string>());
				if (args[0] == "if")
				{
					if (args[3].size() >= 14 && args[3].substr(0, 14) == "call_standard_")
					{
						standardFileRefs.push_back(args[3].substr(5, args[3].size()));
					}
				}
				else if (args[0] == "goto")
				{
					if (args[1].size() >= 14 && args[1].substr(0, 14) == "call_standard_")
					{
						standardFileRefs.push_back(args[1].substr(5, args[1].size()));
					}
				}
			}

			mainFile->second.clear();
			mainFile->second.seekg(0, mainFile->second.beg);

			std::vector<std::map<std::string, std::ifstream>::iterator> standardFilesOpened;
			for (int r = 0; r < standardFileRefs.size(); ++r)
			{
				directoryMap[standardFileRefs[r]] = std::ifstream("standard\\" + standardFileRefs[r] + ".ddls");
				standardFilesOpened.push_back(directoryMap.find(standardFileRefs[r]));
			}

			bool areStandardFilesOpened = true;
			for (int f = 0; f < standardFilesOpened.size(); ++f)
			{
				if (!standardFilesOpened[f]->second)
				{
					areStandardFilesOpened = false;
					break;
				}
				else
				{
					lineIters[standardFilesOpened[f]->first] = "";
				}
			}

			if (areStandardFilesOpened)
			{

				bool isValidAlphabet = true;

				std::string alphabetString;
				std::string invalidAlphabetFile;
				for (std::map<std::string, std::string>::iterator iter = lineIters.begin(); iter != lineIters.end(); ++iter)
				{
					getline(directoryMap[iter->first], iter->second);
					if (iter == lineIters.begin())
					{
						alphabetString = iter->second;
					}
					else
					{
						if (alphabetString != iter->second)
						{
							isValidAlphabet = false;
							invalidAlphabetFile = iter->first;
							break;
						}
					}
				}

				if (isValidAlphabet)
				{

					std::ofstream outputFile;
					std::string outputFilePath = "wb1/" + filePath + ".wb1";

					outputFile.open(outputFilePath);
					if (outputFile)
					{
						//Write WB1 header
						outputFile << "WB1\n";

						//Read in alphabet
						std::vector<char> alphabet;
						for (int i = 0; i < alphabetString.size(); i += 2)
						{
							alphabet.push_back(alphabetString[i]);
						}

						//Write alphabet
						outputFile << alphabetString + '\n';

						//Copy main file over
						while (getline(mainFile->second, lineIters[mainFile->first]))
						{
							outputFile << lineIters[mainFile->first] + "\n";
						}

						for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
						{
							if (iter != mainFile)
							{
								while (getline(iter->second, lineIters[iter->first]))
								{
									outputFile << lineIters[iter->first] + "\n";
								}
							}
						}

						outputFile.close();
					}
				}
				else
				{
					for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
					{
						iter->second.close();
					}
					std::cout << "Error: Invalid alphabet in file " << invalidAlphabetFile << std::endl;
					return -1;
				}
			}
		}
		for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
		{
			iter->second.close();
		}

	}
	else
	{
		for (std::map<std::string, std::ifstream>::iterator iter = directoryMap.begin(); iter != directoryMap.end(); ++iter)
		{
			iter->second.close();
		}
		std::cout << "Error: Failed to open file(s): " + unopenedFiles << std::endl;
		return -1;
	}

	return 0;
}

std::vector<std::string> loadFileIntoVector(std::string filePath)
{
	std::vector<std::string> result;

	std::ifstream file;
	file.exceptions(std::ios::badbit);
	std::string line;

	std::string inputFilePath = "wb1/" + filePath + ".wb1";

	file.open(inputFilePath);

	if (file)
	{
		while (getline(file, line))
		{
			result.push_back(line);
		}
		file.close();
	}

	return result;
}

//Converts .wb1 to .tm
int wb1toTM(std::string filePath, int optimizationLevel)
{
	std::vector<std::string> loadedFile = loadFileIntoVector(filePath);
	
	if (!loadedFile.empty())
	{
		std::ofstream output;

		std::string outputFilePath = "tm/" + filePath + ".tm";

		output.open(outputFilePath);
		if (output)
		{
			//Check for valid header
			if (loadedFile[0] == "WB1")
			{
				//Write header to file
				output << "TM\n";

				std::vector<char> alphabet;
				for (int c = 0; c < loadedFile[1].size(); c += 2)
				{
					alphabet.push_back(loadedFile[1][c]);
				}

				//Write alphabet
				output << loadedFile[1] + "\n";

				//Parse labels
				std::map<std::string, unsigned int> labelMap;
				int lineNum = 0;
				for(int l = 2; l < loadedFile.size(); ++l)
				{
					std::istringstream lineStream(loadedFile[l]);
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
									std::cout << "Repeated label: " << label << " at " << l << " and " << labelMap[label];
									output.close();
									return -1;
								}
							}
							else
							{
								std::cout << "Invalid label at " << std::to_string(l + labelMap.size()) << std::endl;
								output.close();
								return -1;
							}
						}
						else
						{
							++lineNum;
						}
					}
				}

				for (int l = 1; l < loadedFile.size(); ++l)
				{
					std::istringstream lineStream(loadedFile[l]);
					std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, std::istream_iterator<std::string>());
					if (labelMap.find(args[0].substr(0, args[0].size() - 1)) != labelMap.end())
					{
						loadedFile.erase(loadedFile.begin() + l--);
					}
				}

				for(int l = 1; l < loadedFile.size(); ++l)
				{
					std::istringstream lineStream(loadedFile[l]);
					std::vector<std::string> args(std::istream_iterator<std::string>{lineStream}, std::istream_iterator<std::string>());
					if (!args.empty())
					{
						if (args[0] == "accept") // accept
						{
							loadedFile[l] = "0\n";
						}
						else if (args[0] == "reject") //reject
						{
							loadedFile[l] = "1\n";
						}
						else if (args[0] == "if") //if-goto
						{
							if (labelMap.find(args[3]) != labelMap.end())
							{
								loadedFile[l] = "2 " + std::to_string(std::find(alphabet.begin(), alphabet.end(), args[1][0]) - alphabet.begin())
									+ " " + std::to_string(labelMap[args[3]]) + "\n";
							}
							else
							{
								std::cout << "Invalid label reference at " << std::to_string(l) << std::endl;
								output.close();
								return -1;
							}
						}
						else if (args[0] == "write") //write
						{
							loadedFile[l] = "3 "
								+ std::to_string(std::find(alphabet.begin(), alphabet.end(), args[1][0]) - alphabet.begin())
								+ "\n";
						}
						else if (args[0] == "goto") //goto
						{
							if (labelMap.find(args[1]) != labelMap.end())
							{
								loadedFile[l] = "4 " + std::to_string(labelMap[args[1]]) + "\n";
							}
						}
						else if (args[0] == "move") //move
						{
							loadedFile[l] = "5 " + args[1] + "\n";
						}
					}
				}

				std::vector<std::string> goTos;

				for(int l = 2; l < loadedFile.size(); ++l)
				{
					std::istringstream lineStream(loadedFile[l]);
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

				//Maps .wb1 lines to .tm states
				std::map<std::string, unsigned int> lineMap;

				lineMap["line0"] = 0;

				//template of turing machine table to be filled out
				Table temp = { 0, {}, {}, {} };

				if (optimizationLevel > 0)
				{
					//List of current instructions read in
					std::vector<Instruction> currentInstructions;
					
					int l = 2;
					for(; l < loadedFile.size(); ++l)
					{
						std::istringstream lineStream(loadedFile[l]);
						std::vector<std::string> args(std::istream_iterator<std::string>{lineStream},
							std::istream_iterator<std::string>());

						if (currentInstructions.size() != 0 &&
							(!validOptimization(currentInstructions, args[0]) ||
							(std::find(goTos.begin(), goTos.end(), std::to_string(l-2)) != goTos.end())))
						{
							//Optimized states
							Table optimizedStates = getOptimizedStates(currentInstructions, alphabet, temp.numStates);

							temp.numStates += optimizedStates.numStates;
							temp.write.insert(temp.write.end(), optimizedStates.write.begin(), optimizedStates.write.end());
							temp.move.insert(temp.move.end(), optimizedStates.move.begin(), optimizedStates.move.end());
							temp.nextState.insert(temp.nextState.end(), optimizedStates.nextState.begin(), optimizedStates.nextState.end());

							lineMap["line" + std::to_string(l-2)] = temp.numStates;
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
					}

					Table optimizedStates = getOptimizedStates(currentInstructions, alphabet, temp.numStates);

					temp.numStates += optimizedStates.numStates;
					temp.write.insert(temp.write.end(), optimizedStates.write.begin(), optimizedStates.write.end());
					temp.move.insert(temp.move.end(), optimizedStates.move.begin(), optimizedStates.move.end());
					temp.nextState.insert(temp.nextState.end(), optimizedStates.nextState.begin(), optimizedStates.nextState.end());

					lineMap["line" + std::to_string(l-2)] = temp.numStates;
					currentInstructions.clear();

					//Replace labels with mapped states
					for (int i = 0; i < temp.nextState.size(); ++i)
					{
						if (lineMap.find(temp.nextState[i]) != lineMap.end())
						{
							temp.nextState[i] = std::to_string(lineMap[temp.nextState[i]]);
						}
					}

					for (int c = 0; c < temp.nextState.size(); ++c)
					{
						if (temp.nextState[c] != "r" && temp.nextState[c] != "a")
						{
							if (std::stoi(temp.nextState[c]) >= temp.numStates)
							{
								temp.nextState[c] = "a";
							}
						}
					}

					if (optimizationLevel == 2)
					{
						lookAheadOptimize(temp, alphabet);
					}
				}
				else
				{
					int l = 2;
					for(; l < loadedFile.size(); ++l)
					{
						std::istringstream lineStream(loadedFile[l]);
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
									if (args[1] == std::to_string(c))
									{
										temp.nextState.push_back(std::to_string(temp.numStates + 2));
									}
									else
									{
										temp.nextState.push_back(std::to_string(temp.numStates + 1));
									}
								}

								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back(std::to_string(temp.numStates + 3));
								}

								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[c]);
									temp.move.push_back('l');
									temp.nextState.push_back("line" + args[2]);
								}
								temp.numStates += 3;
								break;
							}
							case opWrite:
							{
								for (int c = 0; c < alphabet.size(); ++c)
								{
									temp.write.push_back(alphabet[std::stoi(args[1])]);
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
									temp.move.push_back(args[1][0]);
									temp.nextState.push_back(std::to_string(temp.numStates + 1));
								}
								temp.numStates += 1;
								break;
							}
						}
						lineMap["line" + std::to_string(l-1)] = temp.numStates;
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
			std::cout << "Invalid Header Found.\nExpected: \"WB1\".\nRead: \"" << loadedFile[0] << "\"" << std::endl;
			output.close();
			return -1;
			}

			output.close();
		};
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
					std::cout << "Error: Invalid path variable " << (args[2][0] == '-' ? args[3] : args[2]) << std::endl;
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

											if (machine.head.state == "r" || machine.head.state == "a")
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
										
									if (machine.head.state == "r" || machine.head.state == "a")
									{
										std::cout << ((machine.head.state == "r") ? "Rejected" : "Accepted") << std::endl;
									}
								}
								else if (isNum(flagString.substr(1, flagString.size() - 1)))
								{
									unsigned int stepLimit = std::stoi(flagString.substr(1, flagString.size() - 1));
									machine.run(true, stepLimit, false);
										
									if (machine.head.state == "r" || machine.head.state == "a")
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
			"\tcompile <ddls|wb1> <wb1|tm> <inputPath> -optimizationLevel(-Ox)\n" <<
			"\trun <inputFile> <inputTape> -stepLimit(-#) -showTape(-S)" << std::endl;
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