#define <Memory.h>
#define <types.h>

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
	
	// Double Operand Instructions
	void MOV();
	void CMP();
	void BIT();
	void BIC();
	void BIS();
	void ADD();
	void SUB();
	
	// Single Operand Instructions
	void SWAB();		// is this a byte operation?
	void JSR();
	void EMT();
	void CLR();
	void COM();
	void INC();
	void DEC();
	void NEG();
	void ADC();
	void SBC();
	void TST();
	void ROR();
	void ROL();
	void ASR();
	void ASL();
	void SXT();			// is this a byte operation?
	
	// Branch Instructions
	void BR();
	void BNE();
	void BEQ();
	void BPL();
	void BMI();
	void BVC();
	void BHIS();
	void BCC();
	void BLO();
	void BCS();
	void BGE();
	void BLT();
	void BGT();
	void BLE();
	void BHI();
	void BLOS();
	
private:
	Register r[];
	Register sp;
	Register pc;
	StatusRegister;
	Memory memory;
	int totalCount;
	int readCount;
	int writeCount;
	int instructionCount;
}
