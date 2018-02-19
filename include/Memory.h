#include "types.h"

#ifndef MEMORY_H_20180218
#define MEMORY_H_20180218
class Memory
{
public:
	Memory();
	~Memory();
	OctalWord get(OctalWord address);
	void set(OctalWord address, OctalWord value);
	
private:
	OctalWord* memory;
};
#endif