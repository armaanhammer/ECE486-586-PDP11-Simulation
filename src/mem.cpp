#include "types.h"
#include "constants.h"
#include "octalword.h"
#include "mem.h"
#include <string>
#include <iostream>

using namespace std;

Mem::Mem()
{
	memory = new MemSpot[MEMORYLENGTH]();
	for (int i = 0; i < MEMORYLENGTH; i++) 
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
	return memory[address.value].value;
}

bool Mem::isTouched(OctalWord address)
{
	return memory[address.value].touched;
}

void Mem::setWord(OctalWord address, OctalWord value, bool isInstruction, bool touched)
{
	memory[address.value].value = value;
	memory[address.value].instruction = isInstruction;
	memory[address.value].touched = touched;
}

void Mem::print()
{
	cout << "memory hierarchy\n  location  |   value\n";
	for (int i = 0; i < MEMORYLENGTH; i+=2)
	{
		OctalWord index = OctalWord(2 * i);
		OctalWord toPrint = memory[index.value].value;
		cout << "   ";
		index.print();
		cout << "   |   ";
		toPrint.print();
		cout << "  \n";
	}
}
