#pragma once
#include "types.h"

#ifndef REGISTER_H_20180218
#define REGISTER_H_20180218

class Register
{
public:
	Register();
	~Register();
	OctalWord getVal();
	void setval(OctalWord value);
	void print();
private:
	OctalWord* r;
};
#endif
