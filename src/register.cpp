#include "register.h"
#include "octalword.h"

Register::Register()
{
	r = new OctalWord();
}

Register::~Register()
{
	delete r;
}

int Register::getVal()
{
	return r->value;
}

void Register::setval(int value)
{
	delete r;
	r = new OctalWord(value);
}
