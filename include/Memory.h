#include "types.h"

#ifndef MEMORY_H_20180218
#define MEMORY_H_20180218
class Memory
{
public:
	Memory();
	~Memory();
	Word get(Word address);
	void set(Word address, Word value);
	
private:
	Word memory[MEMORYLENGTH];
};
#endif