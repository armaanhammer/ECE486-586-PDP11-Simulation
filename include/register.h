#pragma once
#include "types.h"

#ifndef REGISTER_H_20180218
#define REGISTER_H_20180218

class Register
{
public:
	Register();
	~Register();
	int getVal();
	void setval(int value);
private:
	OctalWord* r;
};
#endif
