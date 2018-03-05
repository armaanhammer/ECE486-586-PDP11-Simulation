#pragma once

#include <stdio.h>
#include "memory.h"
#include "types.h"
#include "octalword.h"
#include "table.h"
#include "constants.h"
#include "register.h"

#ifndef PDP11SIMCONTROLLER_H_20180221
#define PDP11SIMCONTROLLER_H_20180221
class PDP11SimController 
{
public:
	PDP11SimController();
	~PDP11SimController();
	
	bool decode(int octalVA);
	int getTotalCount();
	int getReadCount();
	int getWriteCount();
	int getInstructionCount();

private:
#pragma region PSWI
	void SPL(OctalBit bit);
	void CLC();
	void CLV();
	void CLZ();
	void CLN();
	void SEC();
	void SEV();
	void SEZ();
	void SEN();
	void CCC();
	void SCC();
#pragma endregion
//Single operation instructions
#pragma region SOI
	OctalWord JSR(OctalWord src);
	OctalWord CLR(OctalWord src);
	OctalWord COM(OctalWord src);
	OctalWord INC(OctalWord src);
	OctalWord DEC(OctalWord src);
	OctalWord NEG(OctalWord src);
	OctalWord ADC(OctalWord src);
	OctalWord SBC(OctalWord src);
	OctalWord TST(OctalWord src);
	OctalWord ROR(OctalWord src);
	OctalWord ROL(OctalWord src);
	OctalWord ASR(OctalWord src);
	OctalWord ASL(OctalWord src);
	OctalWord SXT(OctalWord src);		// is this a byte operation?
#pragma endregion
//Double operation instructions
#pragma region DOI
	OctalWord MOV(OctalWord dest, OctalWord src);
	OctalWord CMP(OctalWord dest, OctalWord src);
	OctalWord BIT(OctalWord dest, OctalWord src);
	OctalWord BIC(OctalWord dest, OctalWord src);
	OctalWord BIS(OctalWord dest, OctalWord src);
	OctalWord ADD(OctalWord dest, OctalWord src);
	OctalWord SUB(OctalWord dest, OctalWord src);
#pragma endregion
//Extended double operation instruction
#pragma region EDOI
	OctalWord MUL();
	OctalWord DIV();
	OctalWord ASH();
	OctalWord ASHC();
	OctalWord XOR();
	OctalWord FPO();
	OctalWord SYSINSTRUCTION();
	OctalWord SOB();
#pragma endregion
#pragma region BI
	OctalWord BR(OctalWord src);
	OctalWord BNE(OctalWord src);
	OctalWord BEQ(OctalWord src);
	OctalWord BPL(OctalWord src);
	OctalWord BMI(OctalWord src);
	OctalWord BVC(OctalWord src);
	OctalWord BHIS(OctalWord src);
	OctalWord BCC(OctalWord src);
	OctalWord BLO(OctalWord src);
	OctalWord BCS(OctalWord src);
	OctalWord BGE(OctalWord src);
	OctalWord BLT(OctalWord src);
	OctalWord BGT(OctalWord src);
	OctalWord BLE(OctalWord src);
	OctalWord BHI(OctalWord src);
	OctalWord BLOS(OctalWord src);
#pragma endregion
//Null functions
#pragma region NULLFUNC
	OctalWord NULLFUNC();
	OctalWord NULLFUNC(OctalWord src);
	OctalWord NULLFUNC(OctalWord dest, OctalWord src);
#pragma endregion
//Functions to check the instruction types
#pragma region CHECK_INSTRUCTION_TYPE_FUNCTIONS
	bool checkForBranch(int value);
	bool checkForDO(OctalWord w);
	bool checkUnimplementedDoubleOp(OctalWord w);
	bool checkForSO(OctalWord w);
	bool checkForSPL(OctalBit b1, OctalBit b2, OctalBit b3, OctalBit b4, OctalBit b5);
	bool checkForPSW(OctalBit b3, OctalBit b4, OctalBit b5);
#pragma endregion
//Function to execute the intruction types
#pragma region EXEC_INSTRUCTION_TYPE_FUNCTIONS
	void PDP11SimController::WriteBack(int am, int destReg, OctalWord writenVal);
	void doBranchInstruction(int value);
	void doUnimplementedDoubleOp(int opnum);
	void doDoubleOpInstruction(OctalWord w);
	void doSingleOpInstruction(OctalWord w);
	void doPSWI(int opcode);
#pragma endregion
#pragma region TABLE
	void createSingleOpTable();
	void createDoubleOpTable();
	void createAddressingModeTable();
	void createBranchTable();
	void createPSWITable();
	void createEDOITable();
#pragma endregion
#pragma region TYPES
	typedef void(*NoParamFunc)();
	typedef OctalWord(*OneParamFunc)(OctalWord);
	typedef OctalWord(*TwoParamFunc)(OctalWord, OctalWord);
#pragma endregion
#pragma region VARS
	Register r[NUMGENERALREGISTERS]; //General purpose registers
	Register sp; //Stack pointer register
	Register pc; //Program counter register
	StatusRegister status; //Status register
	Memory memory; //Memory array
	int totalCount;
	int readCount;
	int writeCount;
	int instructionCount;
	Table<int, OneParamFunc>* AM;
	Table<int, OneParamFunc>* SO;
	Table<int, TwoParamFunc>* DO;
	Table<int, OneParamFunc>* BI;
	Table<int, NoParamFunc>*  PSWI;
	Table<int, NoParamFunc>* EDO;
#pragma endregion
};
#endif
