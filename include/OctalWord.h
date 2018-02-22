#pragma once
#include "types.h"
#include "constants.h"

#ifndef OCTALWORD_H_20180221
#define OCTALWORD_H_20180221
class OctalWord : public Word
{
public:
	OctalWord();
	OctalWord(int val);
	~OctalWord();
	OctalBit octbit[6];

	operator=();
	OctalWord operator+(const OctalWord& b);
	OctalWord operator-(const OctalWord& b);
	operator*();
	OctalWord operator--();
	OctalWord operator--(int);
	OctalWord operator++();
	OctalWord operator++(int);
	operator<<();
	operator>>();
	operator+=();
	operator-=();
private:
	void updateBits();
};
#endif