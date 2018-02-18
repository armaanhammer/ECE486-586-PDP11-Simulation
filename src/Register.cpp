#include "Register.h"

Register::Register()
{
	r.value = 0;
}

Register::~Register()
{
}

int Register::getVal()
{
	return r.value;
}

void Register::setval(int value)
{
	r.value = value;
}