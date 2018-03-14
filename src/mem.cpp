#include "types.h"
#include "constants.h"
#include "octalword.h"
#include "mem.h"
#include <string>
#include <iostream>

using namespace std;

Mem::Mem()
{
	memory = new MemSpot[MEMORYLENGTH/2];
	for (int i = 0; i < MEMORYLENGTH/2; i++) 
	{
		memory[i] = MemSpot(0);
	}
}

Mem::~Mem()
{
	delete[] memory;
}

OctalWord Mem::getWord(OctalWord address)
{
	return memory[address.value/2].value;
}

bool Mem::isTouched(OctalWord address)
{
	return memory[address.value/2].touched;
}

void Mem::setWord(OctalWord address, OctalWord value, bool isInstruction, bool touched)
{
	memory[address.value/2].value = value;
	memory[address.value/2].instruction = isInstruction;
	memory[address.value/2].touched = touched;
}

void Mem::print()
{
	cout << "memory hierarchy\n  location  |   value\n";
	for (int i = 0; i < MEMORYLENGTH / 2 ; i++)
	{
		OctalWord index = OctalWord(2 * i);
		if (memory[i].touched)
		{
			OctalWord toPrint = memory[index.value /2].value;
			cout << "   ";
			index.print();
			cout << "   |   ";
			toPrint.print();
			cout << "  \n";
		}
	}
}
