#include "register.h"
#include "octalword.h"

Register::Register()
{
	regVal = OctalWord(0);
}

Register::~Register()
{
}

OctalWord Register::getVal()
{
	return regVal;
}

void Register::setval(OctalWord value)
{
	regVal = value;
}

void Register::print()
{
}
