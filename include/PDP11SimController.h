#include <stdio.h>
#include "Memory.h"
#include "types.h"
#include "constants.h"
#include "Register.h"

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
#pragma region SOI
	void JSR(int src);
	void CLR(int src);
	void COM(int src);
	void INC(int src);
	void DEC(int src);
	void NEG(int src);
	void ADC(int src);
	void SBC(int src);
	void TST(int src);
	void ROR(int src);
	void ROL(int src);
	void ASR(int src);
	void ASL(int src);
	void SXT(int src);		// is this a byte operation?
#pragma endregion
#pragma region DOI
	void MOV(int dest, int src);
	void CMP(int dest, int src);
	void BIT(int dest, int src);
	void BIC(int dest, int src);
	void BIS(int dest, int src);
	void ADD(int dest, int src);
	void SUB(int dest, int src);
#pragma endregion
#pragma region EDOI
	void MUL();
	void DIV();
	void ASH();
	void ASHC();
	void XOR();
	void FPO();
	void SYSINSTRUCTION();
	void SOB();
#pragma endregion
#pragma region BI
	void BR(int src);
	void BNE(int src);
	void BEQ(int src);
	void BPL(int src);
	void BMI(int src);
	void BVC(int src);
	void BHIS(int src);
	void BCC(int src);
	void BLO(int src);
	void BCS(int src);
	void BGE(int src);
	void BLT(int src);
	void BGT(int src);
	void BLE(int src);
	void BHI(int src);
	void BLOS(int src);
#pragma endregion
#pragma region NULLFUNC
	void NULLFUNC();
	void NULLFUNC(int src);
	void NULLFUNC(int dest, int src);
#pragma endregion
#pragma region CHECK_INSTRUCTION_TYPE_FUNCTIONS
	bool checkForBranch(int value);
	bool checkForDO(OctalWord w);
	bool checkUnimplementedDoubleOp(OctalWord w);
	bool checkForSO(OctalWord w);
	bool checkForSPL(OctalBit b1, OctalBit b2, OctalBit b3, OctalBit b4, OctalBit b5);
	bool checkForPSW(OctalBit b3, OctalBit b4, OctalBit b5);
#pragma endregion
#pragma region EXEC_INSTRUCTION_TYPE_FUNCTIONS
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
	typedef void(*OneParamFunc)(int);
	typedef void(*TwoParamFunc)(int, int);
#pragma endregion
#pragma region VARS
	Register r[NUMGENERALREGISTERS];
	Register sp;
	Register pc;
	StatusRegister status;
	Memory memory;
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
