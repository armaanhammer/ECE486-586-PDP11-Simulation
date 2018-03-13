#pragma once

#include "constants.h"

#ifndef TYPES_LIB_20180221
#define TYPES_LIB_20180221

// cannot do unsigned int I:0 = 1;
typedef struct StatusRegister
{
    unsigned int I : 3;
    unsigned int T : 1;
    unsigned int N : 1;
    unsigned int Z : 1;
    unsigned int V : 1;
    unsigned int C : 1;

	bool operator !=(const StatusRegister& oneToCompareTo) 
	{
		bool ret = true;
		ret &= (this->I == oneToCompareTo.I);
		ret &= (this->T == oneToCompareTo.T);
		ret &= (this->N == oneToCompareTo.N);
		ret &= (this->Z == oneToCompareTo.Z);
		ret &= (this->V == oneToCompareTo.V);
		ret &= (this->C == oneToCompareTo.C);
		return ret; 
	};

	StatusRegister& operator=(const StatusRegister& newAssignment)
	{
		if (*this != newAssignment)
		{
			this->I = newAssignment.I;
			this->T = newAssignment.T;
			this->N = newAssignment.N;
			this->Z = newAssignment.Z;
			this->V = newAssignment.V;
			this->C = newAssignment.C;
		}
		return *this;
	};

	void print()
	{
		cout << "processor status word\n"
			<< " bit | value\n"
			<< "  I  |   " << I << "\n"
			<< "  T  |   " << T << "\n"
			<< "  N  |   " << N << "\n"
			<< "  Z  |   " << Z << "\n"
			<< "  V  |   " << V << "\n"
			<< "  C  |   " << C << "\n";
	}
};

typedef struct Bit
{
	unsigned int b : 1;
	bool overflow = false;

	Bit operator +(const Bit& b) 
	{
		overflow = (*this == 1 && b.b == 1);
		this->b = (*this == 1 && b.b == 1) ? 0: this->b + b.b ;
		return *this;
	};
	Bit operator +(const int& b) 
	{
		overflow = this->b + b > 1;
		this->b = (b == 0) ? this->b : !this->b; 
		return *this; 
	};
	Bit operator &(const Bit& b) 
	{ 
		this->b = (*this == b) ? 1 : 0; 
		return *this; 
	};
	Bit operator |(const Bit& b) 
	{
		this->b = (this->b == 1 || b.b == 1) ? 1 : 0; 
		return *this; 
	};
	Bit operator !() { this->b = (this->b == 0) ? 1 : 0; return *this; };
	bool operator ==(const Bit& oneToCompareTo) { return this->b == oneToCompareTo.b; };
	bool operator ==(const int& oneToCompareTo) { return this->b == oneToCompareTo; };
	bool operator !=(const Bit& oneToCompareTo) { return !(*this == oneToCompareTo); };
	bool operator !=(const int& oneToCompareTo) { return !(*this == oneToCompareTo); };
};

typedef struct OctalBit
{
	unsigned int b : 3;
	bool operator <(const OctalBit& oneToCompareTo) { return this->b < oneToCompareTo.b; };
	bool operator <(const int& oneToCompareTo) { return this->b < oneToCompareTo; };
	bool operator <=(const OctalBit& oneToCompareTo) { return !(*this > oneToCompareTo); };
	bool operator <=(const int& oneToCompareTo) { return !(*this > oneToCompareTo); };
	bool operator >(const OctalBit& oneToCompareTo) { return this->b > oneToCompareTo.b; };
	bool operator >(const int& oneToCompareTo) { return this->b > oneToCompareTo; };
	bool operator >=(const OctalBit& oneToCompareTo) { return !(*this < oneToCompareTo); };
	bool operator >=(const int& oneToCompareTo) { return !(*this < oneToCompareTo); };
	bool operator ==(const OctalBit& oneToCompareTo) { return this->b == oneToCompareTo.b; };
	bool operator ==(const int& oneToCompareTo) { return this->b == oneToCompareTo; };
	bool operator !=(const OctalBit& oneToCompareTo) { return !(*this == oneToCompareTo); };
	bool operator !=(const int& oneToCompareTo) { return !(*this == oneToCompareTo); };
};

typedef struct Word
{
    int value:WORD_OCTAL_LENGTH*3-2;
	bool overflow = false;
	Bit binary[WORD_OCTAL_LENGTH * 3];
	Word() {};
	Word(int value) {};
	bool operator <(const Word& oneToCompareTo) { return this->value < oneToCompareTo.value; };
	bool operator <=(const Word& oneToCompareTo) { return !(*this > oneToCompareTo); };
	bool operator >(const Word& oneToCompareTo) { return this->value > oneToCompareTo.value; };
	bool operator >=(const Word& oneToCompareTo) { return !(*this < oneToCompareTo); };
	bool operator ==(const Word& oneToCompareTo) { return this->value == oneToCompareTo.value; };
	bool operator !=(const Word& oneToCompareTo) { return !(*this == oneToCompareTo); };
};
#endif
