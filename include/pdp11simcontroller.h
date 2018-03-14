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
	PDP11SimController(bool debugMem, bool debugReg);
	~PDP11SimController();
	void run();
	void loadProgram(std::string filename);
	void fetch();
	bool decode();
	int getInstructionCount();
	void printRegs();

private:
#pragma region TYPES
	typedef void(*NoParamFunc)();
	typedef void(*executeFunction)(OctalWord);
	typedef OctalWord(*OneParamFunc)(const OctalWord&);
	typedef OctalWord(*TwoParamFunc)(const OctalWord&, const OctalWord&);
	typedef OctalWord(*AddressModeFunc)(OctalWord, int);
#pragma endregion

#pragma region VARS
	static Register r[NUMGENERALREGISTERS]; //General purpose registers
	static Register sp; //Stack pointer register
	static Register pc; //Program counter register
	static StatusRegister status; //Status register
	static Memory memory; //Memory array
	int instructionCount;
	bool debugMemory;
	bool debugRegisters;
	Table<int, AddressModeFunc>* AM;
	static OctalWord ci;
	executeFunction execute;
	Table<int, OneParamFunc>* SO;
	Table<int, TwoParamFunc>* DO;
	Table<int, OneParamFunc>* BI;
	Table<int, NoParamFunc>*  PSWI;
	Table<int, NoParamFunc>* EDO;
#pragma endregion

	void JSR(OctalWord src);
	void RTS(OctalWord src);
//processor status word instructions
#pragma region PSWI
	static void SPL();
	static void CLC();
	static void CLV();
	static void CLZ();
	static void CLN();
	static void SEC();
	static void SEV();
	static void SEZ();
	static void SEN();
	static void CCC();
	static void SCC();
#pragma endregion

//Single operation instructions
#pragma region SOI
	static OctalWord CLR(const OctalWord& src);
	static OctalWord COM(const OctalWord& src);
	static OctalWord INC(const OctalWord& src);
	static OctalWord DEC(const OctalWord& src);
	static OctalWord NEG(const OctalWord& src);
	static OctalWord ADC(const OctalWord& src);
	static OctalWord SBC(const OctalWord& src);
	static OctalWord TST(const OctalWord& src);
	static OctalWord ROR(const OctalWord& src);
	static OctalWord ROL(const OctalWord& src);
	static OctalWord ASR(const OctalWord& src);
	static OctalWord ASL(const OctalWord& src);
	static OctalWord SXT(const OctalWord& src);		// is this a byte operation?
#pragma endregion

//Double operation instructions
#pragma region DOI
	static OctalWord MOV(const OctalWord& dest, const OctalWord& src);
	static OctalWord CMP(const OctalWord& dest, const OctalWord& src);
	static OctalWord BIT(const OctalWord& dest, const OctalWord& src);
	static OctalWord BIC(const OctalWord& dest, const OctalWord& src);
	static OctalWord BIS(const OctalWord& dest, const OctalWord& src);
	static OctalWord ADD(const OctalWord& dest, const OctalWord& src);
	static OctalWord SUB(const OctalWord& dest, const OctalWord& src);
#pragma endregion

//Extended double operation instruction
#pragma region EDOI
	static void MUL();
	static void DIV();
	static void ASH();
	static void ASHC();
	static void XOR();
	static void FPO();
	static void SYSINSTRUCTION();
	static void SOB();
#pragma endregion

#pragma region BI
	static OctalWord BR(const OctalWord& src);
	static OctalWord BNE(const OctalWord& src);
	static OctalWord BEQ(const OctalWord& src);
	static OctalWord BPL(const OctalWord& src);
	static OctalWord BMI(const OctalWord& src);
	static OctalWord BVC(const OctalWord& src);
	static OctalWord BVS(const OctalWord& src);
	static OctalWord BHIS(const OctalWord& src);
	static OctalWord BCC(const OctalWord& src);
	static OctalWord BLO(const OctalWord& src);
	static OctalWord BCS(const OctalWord& src);
	static OctalWord BGE(const OctalWord& src);
	static OctalWord BLT(const OctalWord& src);
	static OctalWord BGT(const OctalWord& src);
	static OctalWord BLE(const OctalWord& src);
	static OctalWord BHI(const OctalWord& src);
	static OctalWord BLOS(const OctalWord& src);
#pragma endregion

//Null functions
#pragma region NULLFUNC
	static void NULLFUNC0();
	static OctalWord NULLFUNC1(const OctalWord& src);
	static OctalWord NULLFUNC2(const OctalWord& dest, const OctalWord& src);
	static OctalWord NULLFUNCAM(OctalWord, int);
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
	void WriteBack(int am, int destReg, OctalWord writenVal);
	void doBranchInstruction(OctalWord w);
	void doUnimplementedDoubleOp(OctalWord w);
	void doDoubleOpInstruction(OctalWord w);
	void doSingleOpInstruction(OctalWord w);
	void doPSWI(OctalWord w);
#pragma endregion

#pragma region TABLE
	void createSingleOpTable();
	void createDoubleOpTable();
	void createAddressingModeTable();
	void createBranchTable();
	void createPSWITable();
	void createEDOITable();
#pragma endregion

#pragma region AM
	static OctalWord REGISTER(OctalWord regValue, int reg);
	static OctalWord REGISTER_DEFERRED(OctalWord regValue, int reg);
	static OctalWord AUTOINC(OctalWord regValue, int reg);
	static OctalWord AUTOINC_DEFERRED(OctalWord regValue, int reg);
	static OctalWord AUTODEC(OctalWord regValue, int reg);
	static OctalWord AUTODEC_DEFERRED(OctalWord regValue, int reg);
	static OctalWord INDEX(OctalWord regValue, int reg);
	static OctalWord INDEX_DEFERRED(OctalWord regValue, int reg);
#pragma endregion

#pragma region PRINT_TO_FILE
	static bool PRINT_TO_FILE(OctalWord address, char type);
#pragma endregion
};
#endif
