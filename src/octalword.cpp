#include "octalword.h"
#include "constants.h"
#include "types.h"

OctalWord::OctalWord()
{
	value = 0; 
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++) 
	{ 
		octbit[i].b = 0; 
	}
}

OctalWord::OctalWord(int val)
{
	value = val;
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = val << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}

OctalWord::~OctalWord()
{
}

void OctalWord::updateBits()
{
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = this->value << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		this->octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}

void OctalWord::updateBinary()
{
}


#pragma region UNARY_OP_OVERLOAD
// negate
OctalWord OctalWord::operator-()
{
	return OctalWord();
}

// negate?
OctalWord OctalWord::operator~()
{
	return OctalWord();
}
#pragma endregion

#pragma region BINARY_OP_OVERLOAD
OctalWord OctalWord::operator+(const OctalWord& b)
{
	int newValue = this->value + b.value;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator-(const OctalWord& b)
{
	int newValue = this->value - b.value;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator+=(const OctalWord & rhs)
{
	return OctalWord(*this + rhs);
}

OctalWord OctalWord::operator-=(const OctalWord & rhs)
{
	return OctalWord();
}

OctalWord OctalWord::operator|=(const OctalWord & rhs)
{
	return OctalWord();
}

OctalWord OctalWord::operator&=(const OctalWord & rhs)
{
	return OctalWord();
}
#pragma endregion

#pragma region ASSIGNMENT_OP_OVERLOAD
OctalWord& OctalWord::operator=(const OctalWord &newAssignment)
{
	if (this != &newAssignment)
	{
		this->value = newAssignment.value;
		this->updateBits();
		this->updateBinary();
	}
	return *this;
}
#pragma endregion

#pragma region SHIFT_OP_OVERLOAD
OctalWord OctalWord::operator<<(const int shiftAmount)
{
	return OctalWord();
}

OctalWord OctalWord::operator>>(const int shiftAmount)
{
	return OctalWord();
}
#pragma endregion

#pragma region RELATIONAL_OP_OVERLOAD
bool OctalWord::operator<(const OctalWord& oneToCompareTo)
{
	return this->value < oneToCompareTo.value;
}

bool OctalWord::operator<=(const OctalWord& oneToCompareTo)
{
	return this->value <= oneToCompareTo.value;
}

bool OctalWord::operator>(const OctalWord& oneToCompareTo)
{
	return this->value > oneToCompareTo.value;
}

bool OctalWord::operator>=(const OctalWord& oneToCompareTo)
{
	return this->value >= oneToCompareTo.value;
}

bool OctalWord::operator==(const OctalWord& oneToCompareTo)
{
	return this->value == oneToCompareTo.value;
}

bool OctalWord::operator!=(const OctalWord& oneToCompareTo)
{
	return this->value != oneToCompareTo.value;
}
#pragma endregion

#pragma region INC_DEC_OP_OVERLOAD
OctalWord OctalWord::operator++()
{
	int newValue = this->value + 1;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator++(int)
{
	int newValue = this->value + 1;
	this->overflow = (newValue > MAX_OCTAL_VALUE);
	this->value = newValue;
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator--()
{
	this->updateBits();
	return *this;
}

OctalWord OctalWord::operator--(int)
{
	this->updateBits();
	return *this;
}
#pragma endregion