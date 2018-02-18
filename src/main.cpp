#include <iostream.h>
#include <fstream.h>
#include <stdio.h>

using namespace std;

#ifdef LINUX
bool linuxOp = true;
#else
bool linuxOp = false;
#endif

bool checkFlags(char* arg, string flag);

int main(int argc, char* argv)
{
	string line;
	ifstream file;
	bool debug = false;
	bool verbose = false;
	int count = 0;
	
	// determine operation ie is debug mode or verbose mode etc
	for (int i = 0; i < argc; i++)
	{
		debug = debug | checkFlags(argv[i], "-D");
		debug = debug | checkFlags(argv[i], "-d");
	}
	for (int i = 0; i < argc; i++)
	{
		verbose = verbose | checkFlags(argv[i], "-A");
		verbose = verbose | checkFlags(argv[i], "-a");
	}
	
	if (verbose) cout << "Opening file\n";

	try
	{
		file.open(argv[1]);
		if (file.is_open())
		{
			if (verbose) cout << argv[1] << " was opened successfully\n";
			while (getline(file, line))
			{
				istringstream iss(line);
				unsigned int instruction, address;
				count++;
				
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

				/***********************************************************************************************************
				// this is were we actually handle cache instructions
				//if (verbose) cout << endl << instruction << ',' << hex << address << '\n';

				//switch (instruction)
				//{
				//default:
				//	cout << instruction << " found at line " << count << ", is not a valid instruction for a cache" << endl;
				//	break;
				//}
				*************************************************************************************************************/

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
	if (!linuxOp) getchar();
	return 0;
}

bool checkFlags(char* arg, string flag)
{
	string str = string(arg);
	return str == flag;
}
