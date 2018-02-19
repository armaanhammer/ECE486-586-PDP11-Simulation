#include "Memory.h"

Memory::Memory()
{
	memory = new OctalWord[MEMORYLENGTH]();
}

Memory::~Memory()
{
	delete[] memory;
}

OctalWord Memory::get(OctalWord address)
{
	return memory[address.value];
}

void Memory::set(OctalWord address, OctalWord value)
{
	memory[address.value] = value;
}