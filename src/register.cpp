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

OctalWord Register::getVal()
{
	return r->value;
}

void Register::setval(OctalWord value)
{
	delete r;
	r = new OctalWord(value);
}

void Register::print()
{
}
