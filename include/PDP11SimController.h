#include <stdio.h>
#include "Memory.h"
#include "types.h"

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
	
	Register r[NUMGENERALREGISTERS];
	Register sp;
	Register pc;
	StatusRegister status;
	Memory memory;
	int totalCount;
	int readCount;
	int writeCount;
	int instructionCount;
	void (*procStatusWordInstructions[])() = {
		&SPL, &CLC, &CLV, &CLZ, &CLN, &SEC, &SEV, &SEZ, &SEN, &CCC, &SCC
	};
	void (*singleOperandInstructions[])(int src) = {
		&SWAB, &JSR, &EMT, &CLR, &COM, &INC, &DEC, &NEG, &ADC, &SBC, &TST, &ROR, &ROL, &ASR, &ASL, &SXT
	};
	void (*doubleOperandInstructions[])(int dest, int src) = {
		&MOV, &CMP, &BIT, &BIC, &BIS, &ADD, &SUB
	};
	void (*branchInstructions[])(int src) = {
		&BR, &BNE, &BEQ, &BPL, &BMI, &BVC, &BHIS, &BCC, &BLO, &BCS, &BGE, &BLT, &BGT, &BLE, &BHI, &BLOS
	};
};
