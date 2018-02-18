#include "Memory.h"
#include "types.h"
#include "PDP11SimController.h"
#include "Register.h"

using namespace std;

PDP11SimController::PDP11SimController()
{
	sp. = 0;
	pc.regVal = 0;
	
	status;
	
	memory = new Memory();
	
	totalCount = 0;
	readCount = 0;
	writeCount = 0;
	instructionCount = 0;
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
