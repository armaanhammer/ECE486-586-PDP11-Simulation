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
	void run();
	void loadProgram(std::string lines[], int count);
	void fetch();
	bool decode();
	int getInstructionCount();

private:
//processor status word instructions
#pragma region PSWI
	void SPL();
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
	OctalWord JSR(const OctalWord& src);
	OctalWord CLR(const OctalWord& src);
	OctalWord COM(const OctalWord& src);
	OctalWord INC(const OctalWord& src);
	OctalWord DEC(const OctalWord& src);
	OctalWord NEG(const OctalWord& src);
	OctalWord ADC(const OctalWord& src);
	OctalWord SBC(const OctalWord& src);
	OctalWord TST(const OctalWord& src);
	OctalWord ROR(const OctalWord& src);
	OctalWord ROL(const OctalWord& src);
	OctalWord ASR(const OctalWord& src);
	OctalWord ASL(const OctalWord& src);
	OctalWord SXT(const OctalWord& src);		// is this a byte operation?
#pragma endregion

//Double operation instructions
#pragma region DOI
	OctalWord MOV(const OctalWord& dest, const OctalWord& src);
	OctalWord CMP(const OctalWord& dest, const OctalWord& src);
	OctalWord BIT(const OctalWord& dest, const OctalWord& src);
	OctalWord BIC(const OctalWord& dest, const OctalWord& src);
	OctalWord BIS(const OctalWord& dest, const OctalWord& src);
	OctalWord ADD(const OctalWord& dest, const OctalWord& src);
	OctalWord SUB(const OctalWord& dest, const OctalWord& src);
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
	OctalWord BR(const OctalWord& src);
	OctalWord BNE(const OctalWord& src);
	OctalWord BEQ(const OctalWord& src);
	OctalWord BPL(const OctalWord& src);
	OctalWord BMI(const OctalWord& src);
	OctalWord BVC(const OctalWord& src);
	OctalWord BVS(const OctalWord& src);
	OctalWord BHIS(const OctalWord& src);
	OctalWord BCC(const OctalWord& src);
	OctalWord BLO(const OctalWord& src);
	OctalWord BCS(const OctalWord& src);
	OctalWord BGE(const OctalWord& src);
	OctalWord BLT(const OctalWord& src);
	OctalWord BGT(const OctalWord& src);
	OctalWord BLE(const OctalWord& src);
	OctalWord BHI(const OctalWord& src);
	OctalWord BLOS(const OctalWord& src);
#pragma endregion

//Null functions
#pragma region NULLFUNC
	OctalWord NULLFUNC();
	OctalWord NULLFUNC(const OctalWord& src);
	OctalWord NULLFUNC(const OctalWord& dest, const OctalWord& src);
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
	void doBranchInstruction(OctalWord w);
	void doUnimplementedDoubleOp(OctalWord w);
	void doDoubleOpInstruction(OctalWord w);
	void doSingleOpInstruction(OctalWord w);
	void doPSWI(OctalWord w);
	void WriteBack(int am, int destReg, OctalWord writenVal);
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
	typedef void(*executeFunction)(const OctalWord&);
	typedef OctalWord(*OneParamFunc)(const OctalWord&);
	typedef OctalWord(*TwoParamFunc)(const OctalWord&, const OctalWord&);
	typedef OctalWord(*AddressModeFunc)(const OctalWord&, const int);
#pragma endregion

#pragma region VARS
	Register r[NUMGENERALREGISTERS]; //General purpose registers
	Register sp; //Stack pointer register
	Register pc; //Program counter register
	StatusRegister status; //Status register
	Memory memory; //Memory array
	int instructionCount;

	Table<int, AddressModeFunc>* AM;
	OctalWord ci;
	executeFunction execute;
	Table<int, OneParamFunc>* SO;
	Table<int, TwoParamFunc>* DO;
	Table<int, OneParamFunc>* BI;
	Table<int, NoParamFunc>*  PSWI;
	Table<int, NoParamFunc>* EDO;
#pragma endregion

#pragma region AM
	OctalWord REGISTER(OctalWord regValue, int reg);
	OctalWord REGISTER_DEFERRED(OctalWord regValue, int reg);
	OctalWord AUTOINC(OctalWord regValue, int reg);
	OctalWord AUTOINC_DEFERRED(OctalWord regValue, int reg);
	OctalWord AUTODEC(OctalWord regValue, int reg);
	OctalWord AUTODEC_DEFERRED(OctalWord regValue, int reg);
	OctalWord INDEX(OctalWord regValue, int reg);
	OctalWord INDEX_DEFERRED(OctalWord regValue, int reg);
#pragma endregion
};
#endif
