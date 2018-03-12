#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <sstream>
#include "types.h"
#include "constants.h"
#include "pdp11simcontroller.h"

using namespace std;

#ifdef LINUX
bool linuxOp = true;
#else
bool linuxOp = false;
#endif

bool checkFlags(char* arg, string flag);

int main(int argc, char* argv[])
{
	PDP11SimController* pdp = new PDP11SimController();
	string line;
	ifstream file;
	bool debugMem = false;
	bool debugReg = false;
	int count = 0;

	// determine operation ie is debug mode or verbose mode etc
	// easy to use:
	// boolean variable = checkFlags(char* of the thing you want to check, "" string of the thing to check against)
	for (int i = 0; i < argc; i++)
	{
		debugReg = debugReg | checkFlags(argv[i], "-D");
		debugReg = debugReg | checkFlags(argv[i], "-d");
		debugMem = debugMem | checkFlags(argv[i], "-A");
		debugMem = debugMem | checkFlags(argv[i], "-a");
	}
	
	/*******************************************************************
	// prompt the user here if no command line flag
	*******************************************************************/
	if (!debugReg && ! debugMem)
	{
		// ask for debug modes
	}
	if (argc <= 1)
	{
		// ask for a file
	}
	
	if (debugMem || debugReg) cout << "Opening file\n";

	try
	{
		file.open(argv[1]);
		if (file.is_open())
		{
			if (debugMem || debugReg) cout << argv[1] << " was opened successfully\n";

			while (getline(file, line))
			{
				istringstream iss(line);
				unsigned int instruction, address;
				
				// error handling section
				if (!(iss >> instruction))
				{
					cerr << "failed to read instruction code at line: " << count << endl << endl;
					continue;
				}
				else if (instruction < 0 || instruction > 4 && instruction < 8 || instruction > 9)
				{
					cerr << "not actual code, skipping instruction code and address at line: " << count << endl << endl;
					continue;
				}
				else if ((instruction != 9 || instruction != 8) && !(iss >> hex >> address))
				{
					cerr << "ignoring address after instruction code: " << instruction << ", THIS IS NOT AN ACTUAL ERROR\n\n";
				}

				if (verbose)
				{
					cout << endl;
					char next = getchar();
					if (next == 'Q' || next == 'q' || next == '')
					{
						cout << "Now terminating simulation\n\n";
						break;
					}
				}
			}
		}
	}
	catch (exception e)
	{
		cout << e.what() << "\n\n";
	}
	file.close();

	int instructioncount = pdp->getInstructionCount();
	
	// do some final prints
	if (!linuxOp) getchar();
	return 0;
}

bool checkFlags(char* arg, string flag)
{
	string str = string(arg);
	return str == flag;
}
