#include "Memory.h"
#include "types.h"
#include "PDP11SimController.h"
#include "Register.h"
#include <functional>

using namespace std;

PDP11SimController::PDP11SimController()
{	
	totalCount = 0;
	readCount = 0;
	writeCount = 0;
	instructionCount = 0;
	createAddressingModeTable();
	createBranchTable();
	createDoubleOpTable();
	createPSWITable();
	createSingleOpTable();
}

PDP11SimController::~PDP11SimController()
{
}
	
void PDP11SimController::decode()
{
}

int PDP11SimController::getTotalCount()
{
	return totalCount;
}

int PDP11SimController::getReadCount()
{
	return readCount;
}

int PDP11SimController::getWriteCount()
{
	return writeCount;
}

int PDP11SimController::getInstructionCount()
{
	return instructionCount;
}

void PDP11SimController::createSingleOpTable()
{
	for (int i = 0; i < NUM_SINGLE_OP_INSTRUCTIONS; i++)
	{
		switch (i)
		{
		case SWAB_OPCODE:
			SO[i] = SWAB;
		case JSR_OPCODE:
		case EMT_OPCODE:
		case CLR_OPCODE:
		default:
			break;
		}
	}
	SO->add(SWAB_OPCODE, (f=SWAB));
	SO->add(JSR_OPCODE, &JSR);
	SO->add(EMT_OPCODE, &EMT);
	SO->add(CLR_OPCODE, &CLR);
	SO->add(COM_OPCODE, &COM);
	SO->add(INC_OPCODE, &INC);
	SO->add(DEC_OPCODE, &DEC);
	SO->add(NEG_OPCODE, &NEG);
	SO->add(ADC_OPCODE, &ADC);
	SO->add(SBC_OPCODE, &SBC);
	SO->add(TST_OPCODE, &TST);
	SO->add(ROR_OPCODE, &ROR);
	SO->add(ROL_OPCODE, &ROL);
	SO->add(ASR_OPCODE, &ASR);
	SO->add(ASL_OPCODE, &ASL);
	SO->add(SXT_OPCODE, &SXT);
}

void PDP11SimController::createDoubleOpTable()
{
	DO.add(MOV_OPCODE, &MOV); 
	DO.add(CMP_OPCODE, &CMP); 
	DO.add(BIT_OPCODE, &BIT); 
	DO.add(BIC_OPCODE, &BIC); 
	DO.add(BIS_OPCODE, &BIS);
	DO.add(ADD_OPCODE, &ADD);
	DO.add(SUB_OPCODE, &SUB);
}

void PDP11SimController::createAddressingModeTable()
{
}

void PDP11SimController::createBranchTable()
{
	BI.add(BR_OPCODE, &BR);
	BI.add(BNE_OPCODE, &BNE);
	BI.add(BEQ_OPCODE, &BEQ);
	BI.add(BPL_OPCODE, &BPL);
	BI.add(BMI_OPCODE, &BMI);
	BI.add(BVC_OPCODE, &BVC); 
	BI.add(BHIS_OPCODE, &BHIS); 
	BI.add(BCC_OPCODE, &BCC); 
	BI.add(BLO_OPCODE, &BLO); 
	BI.add(BCS_OPCODE, &BCS); 
	BI.add(BGE_OPCODE, &BGE); 
	BI.add(BLT_OPCODE, &BLT); 
	BI.add(BGT_OPCODE, &BGT); 
	BI.add(BLE_OPCODE, &BLE); 
	BI.add(BHI_OPCODE, &BHI); 
	BI.add(BLOS_OPCODE, &BLOS);
}

void PDP11SimController::createPSWITable()
{
	PSWI.add(SPL_OPCODE, &SPL); 
	PSWI.add(CLC_OPCODE, &CLC); 
	PSWI.add(CLV_OPCODE, &CLV); 
	PSWI.add(CLZ_OPCODE, &CLZ); 
	PSWI.add(CLN_OPCODE, &CLN); 
	PSWI.add(SEC_OPCODE, &SEC); 
	PSWI.add(SEV_OPCODE, &SEV); 
	PSWI.add(SEZ_OPCODE, &SEZ); 
	PSWI.add(SEN_OPCODE, &SEN); 
	PSWI.add(CCC_OPCODE, &CCC); 
	PSWI.add(SCC_OPCODE, &SCC);
}

void PDP11SimController::NULLFUNC()
{
}

void PDP11SimController::NULLFUNC(int src)
{
}

void PDP11SimController::NULLFUNC(int dest, int src)
{
}

// Processor Status Word Instructions
void PDP11SimController::SPL()
{
}

void PDP11SimController::CLC()
{
}

void PDP11SimController::CLV()
{
}

void PDP11SimController::CLZ()
{
}

void PDP11SimController::CLN()
{
}

void PDP11SimController::SEC()
{
}

void PDP11SimController::SEV()
{
}

void PDP11SimController::SEZ()
{
}

void PDP11SimController::SEN()
{
}

void PDP11SimController::CCC()
{
}

void PDP11SimController::SCC()
{
}
	
// Double Operand Instructions
void PDP11SimController::MOV(int dest, int src)
{
}

void PDP11SimController::CMP(int dest, int src)
{
}

void PDP11SimController::BIT(int dest, int src)
{
}

void PDP11SimController::BIC(int dest, int src)
{
}

void PDP11SimController::BIS(int dest, int src)
{
}

void PDP11SimController::ADD(int dest, int src)
{
}

void PDP11SimController::SUB(int dest, int src)
{
}


// Single Operand Instructions
void PDP11SimController::SWAB(int src)
{
}

void PDP11SimController::JSR(int src)
{
}

void PDP11SimController::EMT(int src)
{
}

void PDP11SimController::CLR(int src)
{
}

void PDP11SimController::COM(int src)
{
}

void PDP11SimController::INC(int src)
{
}

void PDP11SimController::DEC(int src)
{
}

void PDP11SimController::NEG(int src)
{
}

void PDP11SimController::ADC(int src)
{
}

void PDP11SimController::SBC(int src)
{
}

void PDP11SimController::TST(int src)
{
}

void PDP11SimController::ROR(int src)
{
}

void PDP11SimController::ROL(int src)
{
}

void PDP11SimController::ASR(int src)
{
}

void PDP11SimController::ASL(int src)
{
}

void PDP11SimController::SXT(int src)
{
}

// Branch Instructions
void PDP11SimController::BR(int src)
{
}

void PDP11SimController::BNE(int src)
{
}

void PDP11SimController::BEQ(int src)
{
}

void PDP11SimController::BPL(int src)
{
}

void PDP11SimController::BMI(int src)
{
}

void PDP11SimController::BVC(int src)
{
}

void PDP11SimController::BHIS(int src)
{
}

void PDP11SimController::BCC(int src)
{
}

void PDP11SimController::BLO(int src)
{
}

void PDP11SimController::BCS(int src)
{
}

void PDP11SimController::BGE(int src)
{
}

void PDP11SimController::BLT(int src)
{
}

void PDP11SimController::BGT(int src)
{
}

void PDP11SimController::BLE(int src)
{
}

void PDP11SimController::BHI(int src)
{
}

void PDP11SimController::BLOS(int src)
{
}
