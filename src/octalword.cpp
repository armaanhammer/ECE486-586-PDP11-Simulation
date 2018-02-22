#include "octalword.h"
#include "constants.h"
#include "types.h"

OctalWord::OctalWord()
{
	value = 0; for (int i = 0; i < WORD_OCTAL_LENGTH; i++) { octbit[i].b = 0; }
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

void OctalWord::updateBits()
{
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = this->value << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		this->octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}
