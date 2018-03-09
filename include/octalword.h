#pragma once

#include "types.h"
#include "constants.h"
#include <string>

#ifndef OCTALWORD_H_20180221
#define OCTALWORD_H_20180221
class OctalWord : public Word
{
public:
	// public member functions
	OctalWord();
	OctalWord(int val);
	~OctalWord();
	std::string asString();

	// public member variables
	OctalBit octbit[6];

	// operator overloads
	OctalWord& operator =(const OctalWord &newAssignment);	//assignment
	OctalWord operator ~();									//ones compliment
	OctalWord operator -();									//twos compliment
	OctalWord operator +(const OctalWord& b);				// returns result of operandA + operandB
	OctalWord operator +(const int b);						// returns result of operandA + operandB
	OctalWord operator -(const OctalWord& b);				// returns result of operandA - operandB
	OctalWord operator -(const int b);						// returns result of operandA - operandB
	OctalWord operator |(const OctalWord& rhs);				// returns result of operandA | operandB
	OctalWord operator |(const int rhs);					// returns result of operandA | operandB
	OctalWord operator &(const OctalWord& rhs);				// returns result of operandA & operandB
	OctalWord operator &(const int rhs);					// returns result of operandA & operandB
	OctalWord operator --();								// returns result of operandA--
	OctalWord operator --(int);								// returns result of operandA--
	OctalWord operator ++();								// returns result of operandA++
	OctalWord operator ++(int);								// returns result of operandA++
	OctalWord operator <<(const OctalWord& shiftAmount);	// returns result of operandA<<
	OctalWord operator <<(const int shiftAmount);			// returns result of operandA<<
	OctalWord operator >>(const OctalWord& shiftAmount);	// returns result of operandA>>
	OctalWord operator >>(const int shiftAmount);			// returns result of operandA>>
	bool operator <(const OctalWord& oneToCompareTo);		// returns result of operandA < operandB
	bool operator <=(const OctalWord& oneToCompareTo);		// returns result of operandA <= operandB
	bool operator >(const OctalWord& oneToCompareTo);		// returns result of operandA > operandB
	bool operator >=(const OctalWord& oneToCompareTo);		// returns result of operandA >= operandB
	bool operator ==(const OctalWord& oneToCompareTo);		// returns result of operandA == operandB
	bool operator !=(const OctalWord& oneToCompareTo);		// returns result of operandA != operandB

private:
	void updateBits();
};
#endif
