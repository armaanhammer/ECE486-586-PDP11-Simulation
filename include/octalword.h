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

	OctalWord& operator =(const OctalWord &newAssignment);
	OctalWord operator ~();
	OctalWord operator -();
	OctalWord operator +(const OctalWord& b);
	OctalWord operator +(const int b);
	OctalWord operator -(const OctalWord& b);
	OctalWord operator -(const int b);
	OctalWord operator |(const OctalWord& rhs);
	OctalWord operator |(const int rhs);
	OctalWord operator &(const OctalWord& rhs);
	OctalWord operator &(const int rhs);
	OctalWord operator --();
	OctalWord operator --(int);
	OctalWord operator ++();
	OctalWord operator ++(int);
	OctalWord operator <<(const OctalWord& shiftAmount);
	OctalWord operator <<(const int shiftAmount);
	OctalWord operator >>(const OctalWord& shiftAmount);
	OctalWord operator >>(const int shiftAmount);
	bool operator <(const OctalWord& oneToCompareTo);
	bool operator <=(const OctalWord& oneToCompareTo);
	bool operator >(const OctalWord& oneToCompareTo);
	bool operator >=(const OctalWord& oneToCompareTo);
	bool operator ==(const OctalWord& oneToCompareTo);
	bool operator !=(const OctalWord& oneToCompareTo);

private:
	void updateBits();
};
#endif
