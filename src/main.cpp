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
	ifstream file;
	bool debugMem = false;
	bool debugReg = false;
	int count = 0;
	string filename;
	int fileargnum = -1;

	// determine operation ie is debug mode or verbose mode etc
	// easy to use:
	// boolean variable = checkFlags(char* of the thing you want to check, "" string of the thing to check against)
	for (int i = 0; i < argc; i++)
	{
		debugReg = debugReg | checkFlags(argv[i], "-R");
		debugReg = debugReg | checkFlags(argv[i], "-r");
		debugMem = debugMem | checkFlags(argv[i], "-M");
		debugMem = debugMem | checkFlags(argv[i], "-m");
	}
	
	/*******************************************************************
	// prompt the user here if no command line flag
	*******************************************************************/
	if (!debugReg && ! debugMem)
	{
		char choice;
		// ask for debug modes
		cout << "no debug flags set would you like to set them?\n"
			<< " < 1: debug memory\n"
			<< " < 2: debug register\n"
			<< " < 3: both\n";
		cin >> choice;
		switch (choice)
		{
		case '1':
			debugMem = true;
			break;
		case '2':
			debugReg = true;
			break;
		case '3':
			debugMem = true;
			debugReg = true;
			break;
		default:
			break;
		}
	}
	PDP11SimController* pdp = new PDP11SimController(debugMem, debugReg);

	for (int i = 1; i < argc; i++)
	{
		try
		{
			file.open(argv[1]);
			if (file.is_open()) 
			{
				fileargnum = i;
				break;
			}
		}
		catch (exception e)
		{
			cout << e.what() << "\n\n";
		}
	}
	
	if (fileargnum == -1)
	{
		cout << "a file path to a program to run was missing from the command line arguments\n";
		cin >> filename;
	}
	else
	{
		filename = string(argv[fileargnum]);
	}

	pdp->loadProgram(filename);
	pdp->run();

	while (1)
	{
		cout << endl << endl;
		if (debugMem || debugReg)
		{
			cout << endl;
			cout << "would you like to load another program?\n";
			char next = getchar();
			if (next == 'Q' || next == 'q' || next == '')
			{
				cout << "Now terminating simulation\n\n";
				break;
			}
			delete pdp;
			pdp = new PDP11SimController(debugMem, debugReg);
			cout << "please enter a new file name: ";
			cin >> filename;
			cout << endl << endl;
			pdp->loadProgram(filename);
			pdp->run();
		}
	}

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