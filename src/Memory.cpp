#include "Memory.h"

Memory::Memory()
{
	for (int i = 0; i < MEMORYLENGTH; i++)
	{
		memory[i].value = 0;
	}
}

Memory::~Memory()
{
}

Word Memory::get(Word address)
{
	return memory[address.value];
}

void Memory::set(Word address, Word value)
{
	memory[address.value] = value;
}