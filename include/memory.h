#pragma once

#include "types.h"
#include "octalword.h"
#include "table.h"

#ifndef MEMORY_H_20180218
#define MEMORY_H_20180218

typedef struct MemSpot 
{
	OctalWord value;
	bool touched;
	bool instruction;

	MemSpot() { touched = false; instruction = false; value = OctalWord(0); };
	MemSpot(int v) { touched = false; instruction = false; value = OctalWord(v); };
	MemSpot(int v, bool i) { touched = false; instruction = i; value = OctalWord(v); };
};

class Memory
{
public:
	Memory();
	~Memory();
	OctalWord getWord(OctalWord address);
	bool isTouched(OctalWord address);
	void setWord(OctalWord address, OctalWord value);
	void setWord(OctalWord address, OctalWord value, bool isInstruction);
	bool loadProgramIntoMem(std::string lines[]);
	
private:
	Table<OctalWord, MemSpot>* memory;
};
#endif
