#include <types.h>

class Memory
{
public:
	Memory();
	~Memory();
	Word get(Word address);
	bool set(Word address, Word value);
	
private:
	int length;
	Word memory[];
}
