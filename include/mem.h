#include "types.h"
#include "octalword.h"

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
	MemSpot(int v, bool i, bool t) { touched = false; instruction = i; value = OctalWord(v); };
	MemSpot(OctalWord v) { touched = false; instruction = false; value = v; };
	MemSpot(OctalWord v, bool i) { touched = false; instruction = i; value = v; };
	MemSpot(OctalWord v, bool i, bool t) { touched = t; instruction = i; value = v; };
};

class Mem
{
public:
	Mem();
	~Mem();
	OctalWord getWord(OctalWord address);
	bool isTouched(OctalWord address);
	void setWord(OctalWord address, OctalWord value, bool isInstruction = false, bool touched = true);
	void print();
	
private:
	MemSpot* memory;
};
#endif
