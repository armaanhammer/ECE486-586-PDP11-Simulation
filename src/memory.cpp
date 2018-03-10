#include "types.h"
#include "constants.h"
#include "octalword.h"
#include <string>
#include "table.h"
#include "memory.h"

using namespace std;

Memory::Memory()
{
	memory = new Table<OctalWord, MemSpot>();
}

Memory::~Memory()
{
	delete memory;
}

OctalWord Memory::getWord(OctalWord address)
{
	return (*memory)[address].value;
}

bool Memory::isTouched(OctalWord address)
{
	return (*memory)[address].touched;
}

void Memory::setWord(OctalWord address, OctalWord value, bool isInstruction = false, bool touched = false)
{
	memory->add(address, MemSpot(value, isInstruction, touched));
}

bool Memory::loadProgramIntoMem(string lines[], int count)
{
	int index = 0;		// for keeping track of where the word is going to get loaded into memory
	char* c_string;		// used as an intermediate for converting string to octal

	for (int i = 0; i < count; i++) {
		// split each line
		strcpy(c_string, lines[i].c_str());

		// convert the last part to an octal
		int b[6] = { 
			(c_string[1] - '0'), 
			(c_string[2] - '0'), 
			(c_string[3] - '0'), 
			(c_string[4] - '0'), 
			(c_string[5] - '0'), 
			(c_string[6] - '0') 
		};
		// turn the bits into an int
		int num = b[0] << 15 + b[1] << 12 + b[2] << 9 + b[3] << 6 + b[4] << 3 + b[5];

		switch (c_string[0])
		{
		case '@':
			// change index
			index = num;
			break;
		case '-':
			// call setword at 2*i, isInstruction, touched
			setWord(OctalWord(index), OctalWord(num), true, true);
			break;
		case '*':
			// skip
			continue;
		}

		// incrememt the index
		index += 2;

		if (index % 2 != 0)
		{
			// ERROR!!!!
		}
	}

	for (index; index < MEMORYLENGTH; index += 2)
	{
		setWord(OctalWord(index), OctalWord(0));
	}
}

void Memory::print()
{
}
