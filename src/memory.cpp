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
	return memory[address];
}

bool Memory::isTouched(OctalWord address)
{
	return memory[address].touched;
}

void Memory::setWord(OctalWord address, OctalWord value)
{
	memory[address] = value;
}

void Memory::setWord(OctalWord address, OctalWord value, bool isInstruction)
{
}

bool Memory::loadProgramIntoMem(string lines[])
{
}

void Memory::print()
{
}
