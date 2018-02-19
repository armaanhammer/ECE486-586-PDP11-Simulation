#include "Memory.h"
#include "types.h"
#include "PDP11SimController.h"
#include "Register.h"

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
	SO->add(SWAB_OPCODE, this->SWAB);
	SO->add(JSR_OPCODE, this->JSR);
	SO->add(EMT_OPCODE, this->EMT);
	SO->add(CLR_OPCODE, this->CLR);
	SO->add(COM_OPCODE, this->COM);
	SO->add(INC_OPCODE, this->INC);
	SO->add(DEC_OPCODE, this->DEC);
	SO->add(NEG_OPCODE, this->NEG);
	SO->add(ADC_OPCODE, this->ADC);
	SO->add(SBC_OPCODE, this->SBC);
	SO->add(TST_OPCODE, this->TST);
	SO->add(ROR_OPCODE, this->ROR);
	SO->add(ROL_OPCODE, this->ROL);
	SO->add(ASR_OPCODE, this->ASR);
	SO->add(ASL_OPCODE, this->ASL);
	SO->add(SXT_OPCODE, this->SXT);
}

void PDP11SimController::createDoubleOpTable()
{
	DO->add(MOV_OPCODE, this->MOV);
	DO->add(CMP_OPCODE, this->CMP);
	DO->add(BIT_OPCODE, this->BIT);
	DO->add(BIC_OPCODE, this->BIC);
	DO->add(BIS_OPCODE, this->BIS);
	DO->add(ADD_OPCODE, this->ADD);
	DO->add(SUB_OPCODE, this->SUB);
}

void PDP11SimController::createAddressingModeTable()
{
}

void PDP11SimController::createBranchTable()
{
	BI->add(BR_OPCODE, this->BR);
	BI->add(BNE_OPCODE, this->BNE);
	BI->add(BEQ_OPCODE, this->BEQ);
	BI->add(BPL_OPCODE, this->BPL);
	BI->add(BMI_OPCODE, this->BMI);
	BI->add(BVC_OPCODE, this->BVC);
	BI->add(BHIS_OPCODE, this->BHIS);
	BI->add(BCC_OPCODE, this->BCC);
	BI->add(BLO_OPCODE, this->BLO);
	BI->add(BCS_OPCODE, this->BCS);
	BI->add(BGE_OPCODE, this->BGE);
	BI->add(BLT_OPCODE, this->BLT);
	BI->add(BGT_OPCODE, this->BGT);
	BI->add(BLE_OPCODE, this->BLE);
	BI->add(BHI_OPCODE, this->BHI);
	BI->add(BLOS_OPCODE, this->BLOS);
}

void PDP11SimController::createPSWITable()
{
	PSWI->add(SPL_OPCODE, this->SPL);
	PSWI->add(CLC_OPCODE, this->CLC);
	PSWI->add(CLV_OPCODE, this->CLV);
	PSWI->add(CLZ_OPCODE, this->CLZ);
	PSWI->add(CLN_OPCODE, this->CLN);
	PSWI->add(SEC_OPCODE, this->SEC);
	PSWI->add(SEV_OPCODE, this->SEV);
	PSWI->add(SEZ_OPCODE, this->SEZ);
	PSWI->add(SEN_OPCODE, this->SEN);
	PSWI->add(CCC_OPCODE, this->CCC);
	PSWI->add(SCC_OPCODE, this->SCC);
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
