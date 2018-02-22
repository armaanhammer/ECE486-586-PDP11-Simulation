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
	// overload =
};

typedef struct Bit
{
	int b : 1;
};

typedef struct Word
{
    int value:WORD_OCTAL_LENGTH*3;
	bool overflow = false;
	Bit binary[WORD_OCTAL_LENGTH * 3];
	// overload ==
};

typedef struct OctalBit
{
	unsigned int b : 3;
	// overload ==
};
#endif
