#include "types.h"
#include "constants.h"
#include "octalword.h"
#include <string>
#include "table.h"
#include "memory.h"
#include <iostream>

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

void Memory::setWord(OctalWord address, OctalWord value, bool isInstruction, bool touched)
{
	memory->add(address, MemSpot(value, isInstruction, touched));
}

void Memory::print()
{
	cout << "memory hierarchy\n  location  |   value\n";
	for (int i = 0; i < MEMORYLENGTH; i+=2)
	{
		OctalWord index = OctalWord(2 * i);
		OctalWord toPrint = memory->find(index).value;
		cout << "   ";
		index.print();
		cout << "   |   ";
		toPrint.print();
		cout << "  \n";
	}
}
