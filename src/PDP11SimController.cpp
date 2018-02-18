#define <Memory.h>
#define <types.h>

using namespace std;

PDP11SimController::PDP11SimController()
{
	r = new Register[6];
	
	for (int i=0; i < 6; i++)
	{
		r[i].regVal = 0;
	}
	
	sp.regVal = 0;
	pc.regVal = 0;
	
	status;
	
	memory = new Memory(std::pow(2,16));
	
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
void PDP11SimController::MOV()
{
}

void PDP11SimController::CMP()
{
}

void PDP11SimController::BIT()
{
}

void PDP11SimController::BIC()
{
}

void PDP11SimController::BIS()
{
}

void PDP11SimController::ADD()
{
}

void PDP11SimController::SUB()
{
}


// Single Operand Instructions
void PDP11SimController::SWAB()
{
}

void PDP11SimController::JSR()
{
}

void PDP11SimController::EMT()
{
}

void PDP11SimController::CLR()
{
}

void PDP11SimController::COM()
{
}

void PDP11SimController::INC()
{
}

void PDP11SimController::DEC()
{
}

void PDP11SimController::NEG()
{
}

void PDP11SimController::ADC()
{
}

void PDP11SimController::SBC()
{
}

void PDP11SimController::TST()
{
}

void PDP11SimController::ROR()
{
}

void PDP11SimController::ROL()
{
}

void PDP11SimController::ASR()
{
}

void PDP11SimController::ASL()
{
}

void PDP11SimController::SXT()
{
}

// Branch Instructions
void PDP11SimController::BR(Word address)
{
}

void PDP11SimController::BNE(Word address)
{
}

void PDP11SimController::BEQ(Word address)
{
}

void PDP11SimController::BPL(Word address)
{
}

void PDP11SimController::BMI(Word address)
{
}

void PDP11SimController::BVC(Word address)
{
}

void PDP11SimController::BHIS(Word address)
{
}

void PDP11SimController::BCC(Word address)
{
}

void PDP11SimController::BLO(Word address)
{
}

void PDP11SimController::BCS(Word address)
{
}

void PDP11SimController::BGE(Word address)
{
}

void PDP11SimController::BLT(Word address)
{
}

void PDP11SimController::BGT(Word address)
{
}

void PDP11SimController::BLE(Word address)
{
}

void PDP11SimController::BHI(Word address)
{
}

void PDP11SimController::BLOS(Word address)
{
}
