#include <stdio.h>
#include "Memory.h"
#include "types.h"
#include "Register.h"

class PDP11SimController 
{
public:
	PDP11SimController();
	~PDP11SimController();
	
	void decode();
	int getTotalCount();
	int getReadCount();
	int getWriteCount();
	int getInstructionCount();

private:
	// Processor Status Word Instructions
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
	
	// Single Operand Instructions
	void SWAB(int src);		// is this a byte operation?
	void JSR(int src);
	void EMT(int src);
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
	
	// Double Operand Instructions
	void MOV(int dest, int src);
	void CMP(int dest, int src);
	void BIT(int dest, int src);
	void BIC(int dest, int src);
	void BIS(int dest, int src);
	void ADD(int dest, int src);
	void SUB(int dest, int src);
	
	// Branch Instructions
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

	void NULLFUNC();
	void NULLFUNC(int src);
	void NULLFUNC(int dest, int src);
	void createSingleOpTable();
	void createDoubleOpTable();
	void createAddressingModeTable();
	void createBranchTable();
	void createPSWITable();
	
	Register r[NUMGENERALREGISTERS];
	Register sp;
	Register pc;
	StatusRegister status;
	Memory memory;
	int totalCount;
	int readCount;
	int writeCount;
	int instructionCount;

	typedef void(*NoParamFunc)();
	typedef void(*OneParamFunc)(int);
	typedef void(*TwoParamFunc)(int, int);

	Table<int, OneParamFunc>* AM;
	Table<int, OneParamFunc>* SO;
	Table<int, TwoParamFunc>* DO;
	Table<int, OneParamFunc>* BI;
	Table<int, NoParamFunc>* PSWI;
};
