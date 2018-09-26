#include "TuringMachine.h"

bool IsNum(std::string s)
{
	for (int c = 0; c < s.length(); ++c)
	{
		if (s[c] != '0' && s[c] != '1' && s[c] != '2' && s[c] != '3' && s[c] != '4' &&
			s[c] != '5' &&  s[c] != '6' && s[c] != '7' && s[c] != '8' && s[c] != '9')
		{
			return false;
		}
	}
	return true;
}

void turing_machine::Step()
{

	//Extend tape on side if out of bounds
	if (Head.Position == -1)
	{
		Tape.insert(Tape.begin(), BlankSymbol);
		++Head.Position;
	}
	else if (Head.Position == (int)Tape.size()) Tape.push_back(BlankSymbol);

	int TableOffset = (Alphabet.size() + 1) * stoi(Head.State) +
		((Tape[Head.Position]) == BlankSymbol ? 0 :
		(1 + std::find(Alphabet.begin(), Alphabet.end(), Tape[Head.Position]) - Alphabet.begin()));
	Tape[Head.Position] = Table.Write[TableOffset];
	Head.Position += (Table.Move[TableOffset] == 'r') ? 1 : -1;
	Head.State = Table.NextState[TableOffset];
}

int turing_machine::Load(std::string FilePath)
{
	std::ifstream FIn;
	FIn.exceptions(std::ios::badbit);
	FIn.open(FilePath);
	std::string Line;
	if (FIn)
	{
		//Check for valid header
		getline(FIn, Line);
		if (Line == "TM")
		{
			//Read in alphabet
			getline(FIn, Line);
			BlankSymbol = Line[0];
			for (int I = 2; I < Line.length(); I += 2)
			{
				Alphabet.push_back(Line[I]);
			}

			//Get number of states
			getline(FIn, Line);
			Table.NumStates = stoi(Line);

			//Get Table
			getline(FIn, Line);
			for (int I = 0; I < 2 * Table.NumStates * (Alphabet.size() + 1); I += 2)
			{
				if (Line[I] == BlankSymbol || std::find(Alphabet.begin(), Alphabet.end(), Line[I]) != Alphabet.end())
				{
					Table.Write.push_back(Line[I]);
				}
				else
				{
					printf("Error: Character not in alphabet.\nRead: %s\n", Line[I]);
					FIn.close();
					return -1;
				}
			}

			getline(FIn, Line);
			for (int I = 0; I < 2 * Table.NumStates * (Alphabet.size() + 1); I += 2)
			{
				if (Line[I] == 'r' || Line[I] == 'l')
				{
					Table.Move.push_back(Line[I]);
				}
				else
				{
					printf("Error: Invalid move direction.\nRead: %s\n", Line[I]);
					FIn.close();
					return -1;
				}
			}

			for (int I = 0; I < Table.NumStates * (Alphabet.size() + 1); ++I)
			{
				getline(FIn, Line, ' ');
				if (Line == "a" || Line == "r")
				{
					Table.NextState.push_back(Line);
				}
				else
				{
					if (IsNum(Line.c_str()) && 0 < stoi(Line) <= Table.NumStates)
					{
						Table.NextState.push_back(Line);
					}
					else
					{
						printf("Error: Invalid State Transition.\n Read %s\n", Line);
						FIn.close();
						return -1;
					}
				}
			}
		}
		else
		{
			printf("Error: Invalid Header Filed.\nExpected: \"TM\".\nRead: %s\n", Line.c_str());
			FIn.close();
			return -1;
		}

		FIn.close();
	}
	else
	{
		printf("Error: Failed to open file!");
		return -1;
	}

	return 0;
}