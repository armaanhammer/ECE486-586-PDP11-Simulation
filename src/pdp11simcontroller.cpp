#include "memory.h"
#include "types.h"
#include "constants.h"
#include "pdp11simcontroller.h"
#include "register.h"
#include <iostream>

using namespace std;

#pragma region CONSTRUCT_DESTRUCT
///-----------------------------------------------
/// Constructor Function
///-----------------------------------------------
PDP11SimController::PDP11SimController()
{	
	//Initialize the PDP11SimController class
	instructionCount = 0;
	createAddressingModeTable();
	createBranchTable();
	createDoubleOpTable();
	createPSWITable();
	createSingleOpTable();
	ci = OctalWord(0);
}

///-----------------------------------------------
/// Destructor Function
///-----------------------------------------------
PDP11SimController::~PDP11SimController()
{
}
#pragma endregion

void PDP11SimController::run()
{
	while(ci != HALT_OPCODE)
	{
		fetch();
		decode();
		(*execute)(ci);
		pc.setval(pc.getVal() + 2);
	}
}

void PDP11SimController::loadProgram()
{
}

void PDP11SimController::fetch()
{
}

#pragma region TABLE
///-----------------------------------------------
/// Table Creation Functions
///-----------------------------------------------
//Create a table of single operation instructions
void PDP11SimController::createSingleOpTable()
{
	SO->add(JSR_OPCODE, this->JSR);
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
	SO->invalid = this->NULLFUNC;
}

//Create a table of double operation instructions
void PDP11SimController::createDoubleOpTable()
{
	DO->add(MOV_OPCODE, this->MOV);
	DO->add(CMP_OPCODE, this->CMP);
	DO->add(BIT_OPCODE, this->BIT);
	DO->add(BIC_OPCODE, this->BIC);
	DO->add(BIS_OPCODE, this->BIS);
	DO->add(ADD_OPCODE, this->ADD);
	DO->add(SUB_OPCODE, this->SUB);
	DO->invalid = this->NULLFUNC;
}

//Create a table for addressing modes
void PDP11SimController::createAddressingModeTable()
{
	AM->add(REGISTER_CODE, this->/*function name no parenthesises*/);
	AM->invalid = this->NULLFUNC;
}

//Create a table for the branch instructions
void PDP11SimController::createBranchTable()
{
	BI->add(BR_OPCODE, this->BR);
	BI->add(BNE_OPCODE, this->BNE);
	BI->add(BEQ_OPCODE, this->BEQ);
	BI->add(BPL_OPCODE, this->BPL);
	BI->add(BMI_OPCODE, this->BMI);
	BI->add(BVC_OPCODE, this->BVC);
	BI->add(BVS_OPCODE, this->BVS);
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
	BI->invalid = this->NULLFUNC;
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
	PSWI->invalid = this->NULLFUNC;
}

//Create a table for the extended double operation instructions
void PDP11SimController::createEDOITable()
{
	EDO->add(MUL_OPCODE, this->MUL);
	EDO->add(DIV_OPCODE, this->DIV);
	EDO->add(ASH_OPCODE, this->ASH);
	EDO->add(ASHC_OPCODE, this->ASHC);
	EDO->add(XOR_OPCODE, this->XOR);
	EDO->add(FLOATING_POINT_OPCODE, this->FPO);
	EDO->add(SYSTEM_NSTRUCTION_OPCODE, this->SYSINSTRUCTION);
	EDO->add(SOB_OPCODE, this->SOB);
	EDO->invalid = this->NULLFUNC;
}
#pragma endregion

#pragma region DECODE
///-----------------------------------------------
/// Decoding Functions
///-----------------------------------------------
bool PDP11SimController::decode()
{
	// check for too long of word
	if (ci > MAX_OCTAL_VALUE) 
	{ 
		return false; 
	}
	// check to see if op is PSWI, if true exec op
	if (checkForPSW(ci[3], ci[4], ci[5]))
	{ 	
		execute = this->doPSWI; 
		return true; 
	}
	// check to see if op is Branch Instruction, if true exec Instruction
	if (checkForBranch(ci.value)) 
	{ 
		execute = this->doBranchInstruction; 
		return true; 
	}
	// check to see if single operand instruction, if true exec instruction
	if (checkForSO(ci)) 
	{ 	
		execute = this->doSingleOpInstruction;
		return true; 
	}
	// check to see if double operand instruction, if true exec instruction
	if (checkForDO(ci)) 
	{
		execute = this->doDoubleOpInstruction; 
		return true; 
	}
	if (checkUnimplementedDoubleOp(ci)) 
	{ 
		execute = this->doUnimplementedDoubleOp; 
		return true; 
	}
	cerr << "un reachable code segment end of bool PDP11SimController::decode(int octalVA) reached\n";
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkForSPL
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is SPL
//Description: Function for opcode checking the prioity level
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkForSPL(OctalBit b1, OctalBit b2, OctalBit b3, OctalBit b4, OctalBit b5)
{
	if (b5 == 0 && b4 == 0 && b3 == 0 && b2 == 2 && b1 == 3) return true;
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkForPSW
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is PSW instruction
//Description: Function for opcode checking the 
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkForPSW(OctalBit b3, OctalBit b4, OctalBit b5)
{
	if (b3 == 0 && b4 == 0 && b5 == 0) return true;
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkForSO
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is a single operation instruction
//Description: Function for checking if the operation is a single operand
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkForSO(OctalWord w)
{
	if (w.octbit[4] == 0) return true;
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkForDO
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is a double operation instruction
//Description: Function for checking if the operation is a double operation
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkForDO(OctalWord w)
{
	if (w.octbit[4] >= 1 && w.octbit[4] <= 6) return true;
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkUnimplementedDoubleOp
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is a unimlemented (extended) double instruction
//Description: Function for checking if the operation is a unimplmented double operation
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkUnimplementedDoubleOp(OctalWord w)
{
	if (w.octbit[5] == 7)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
//Function: checkForBranch
//Input: (OctalBit) specified address bits
//Output: (bool) true if the instruction is a branch instruction
//Description: Function for checking if the operation is a branch operation
//----------------------------------------------------------------------------------------------------
bool PDP11SimController::checkForBranch(int value)
{
	int opcode = value >> 8;
	
	return (BI->find(opcode)) ? true : false;
}

#pragma endregion

#pragma region INSTRUCTION_EXECUTION
///-----------------------------------------------
/// Instruction Execution Functions
///-----------------------------------------------
void PDP11SimController::doPSWI(OctalWord w)
{
	int opcode = w.value;
	if (w[5] == 0 && w[4] == 0 && w[3] == 0 && w[2] == 2 && w[1] == 3)
	{
		opcode = SPL_OPCODE;
	}
	//find and exec op by opcode
	(*(PSWI->find(opcode)))();
}

void PDP11SimController::doSingleOpInstruction(OctalWord w)
{
	int regNum = w[0].b;
	int regAddressMode = w[1].b;
	int opcode = w.value >> 6;

	OctalWord operand = (*(AM->find(regAddressMode)))(r[regNum].getVal());

	OctalWord result = (*(SO->find(opcode)))(operand);

	WriteBack(regAddressMode, regNum, result);
}

void PDP11SimController::doDoubleOpInstruction(OctalWord w)
{
	int destNum = w.octbit[0].b;
	int destAddressMode = w.octbit[1].b;
	int srcNum = w.octbit[2].b;
	int srcAddressMode = w.octbit[3].b;
	int opcode = w.value >> 12;

	OctalWord operandA = (*(AM->find(srcAddressMode))) (r[srcNum].getVal());

	OctalWord operandB = (*(AM->find(destAddressMode))) (r[destNum].getVal());

	OctalWord result = (*(DO->find(opcode)))(operandA, operandB);

	WriteBack(destAddressMode, destNum, result);
}

void PDP11SimController::WriteBack(int am, int destReg, OctalWord writenVal)
{
	switch (am)
	{
	//Basic addressing register mode
	case(00):
		break;
	//Indirect addressing register mode (deferred)
	case(REGISTER_DEFERRED_CODE):
		break;
	//Basic addressing autoincrement mode
	case(AUTOINC_CODE):
		break;
	//Indirect addressing autoincrement mode (deferred)
	case(AUTOINC_DEFERRED_CODE):
		break;
	//Basic addressing autodecrement mode
	case(AUTODEC_CODE):
		break;
	//Indirect addressing autodecrement mode (deferred)
	case(AUTODEC_DEFERRED_CODE):
		break;
	//Basic addressing index
	case(INDEX_CODE):
		break;
	//Indirect addressing 
	case(INDEX_DEFFERRED_CODE):
		break;
	//PC register addressing immediate mode
	case(PC_IMMEDIATE_CODE):
		break;
	//PC register addressing absolute mode
	case(PC_ABSOLUTE_CODE):
		break;
	//PC register addressing relative mode
	case(PC_RELATIVE_CODE):
		break;
	//PC register addressing relative deferred mode
	case(PC_RELATIVE_DEFERRED_CODE):
		break;
	case(SP_DEFERRED_CODE):
		break;
	case(SP_AUTOINC_CODE):
		break;
	case(SP_AUTOINC_DEFERRED_CODE):
		break;
	case(SP_AUTODEC_CODE):
		break;
	case(SP_INDEX_CODE):
		break;
	case(Sp_INDEX_DEFFERRED_CODE):
		break;
	default:
		break;
	}
}

void PDP11SimController::doBranchInstruction(OctalWord w)
{
	int value = w.value;
	int opcode = value >> 8;

	(*(BI->find(opcode)))(ci);
}

void PDP11SimController::doUnimplementedDoubleOp(OctalWord w)
{
	int opnum = w.octbit[4].b;
	switch (opnum)
	{
	case 0:
		(*(EDO->find(MUL_OPCODE)))();
		break;
	case 1:
		(*(EDO->find(DIV_OPCODE)))();
		break;
	case 2:
		(*(EDO->find(ASH_OPCODE)))();
		break;
	case 3:
		(*(EDO->find(ASHC_OPCODE)))();
		break;
	case 4:
		(*(EDO->find(XOR_OPCODE)))();
		break;
	case 5:
		(*(EDO->find(FLOATING_POINT_OPCODE)))();
		break;
	case 6:
		(*(EDO->find(SYSTEM_NSTRUCTION_OPCODE)))();
		break;
	case 7:
		(*(EDO->find(SOB_OPCODE)))();
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region GETTERS
///-----------------------------------------------
/// Getters
///-----------------------------------------------
int PDP11SimController::getInstructionCount()
{
	return instructionCount;
}
#pragma endregion

#pragma region NULL_FUNCTIONS
///-----------------------------------------------
/// NULL Functions
///-----------------------------------------------
OctalWord PDP11SimController::NULLFUNC()
{
	cout << "some opcode has resulted in a NULLFUNC with no parameters called\n";
}

OctalWord PDP11SimController::NULLFUNC(const OctalWord& src)
{
	cout << "some opcode has resulted in a NULLFUNC with one parameter:" << src.value << " called\n";
}

OctalWord PDP11SimController::NULLFUNC(const OctalWord& dest, const OctalWord& src)
{
	cout << "some opcode has resulted in a NULLFUNC with two parameters:";
	cout << dest.value << " and " << src.value << " called\n";
}

#pragma endregion

#pragma region PROCESSOR_STATUS_WORD_INSTRUCTIONS
///-----------------------------------------------
/// Processor Status Word Instruction Functions
///-----------------------------------------------

void PDP11SimController::SPL()
{
	OctalBit bit = ci.octbit[0];
	status.I = bit.b;
}

void PDP11SimController::CLC()
{
	status.C = 0;
}

void PDP11SimController::CLV()
{
	status.V = 0;
}

void PDP11SimController::CLZ()
{
	status.Z = 0;
}

void PDP11SimController::CLN()
{
	status.N = 0;
}

void PDP11SimController::SEC()
{
	status.C = 1;
}

void PDP11SimController::SEV()
{
	status.V = 1;
}

void PDP11SimController::SEZ()
{
	status.Z = 1;
}

void PDP11SimController::SEN()
{
	status.N = 1;
}

void PDP11SimController::CCC()
{
	status.C = 0;
	status.V = 0;
	status.Z = 0;
	status.N = 0;
}

void PDP11SimController::SCC()
{
	status.C = 1;
	status.V = 1;
	status.Z = 1;
	status.N = 1;
}
#pragma endregion

#pragma region ADDRESSING_MODES
///-----------------------------------------------
/// Addressing Mode Functions
///----------------------------------------------
///
/// The following are the defined constatns from types.h
/// that are used in this section:
///
///#define NUM_ADDRESSING_MODES			18
///#define REGISTER_CODE				00
///#define REGISTER_DEFERRED_CODE		01
///#define AUTOINC_CODE					02
///#define AUTOINC_DEFERRED_CODE		03
///#define AUTODEC_CODE					04
///#define AUTODEC_DEFERRED_CODE		05
///#define INDEX_CODE					06
///#define INDEX_DEFFERRED_CODE			07
///#define PC_IMMEDIATE_CODE			027
///#define PC_ABSOLUTE_CODE				037
///#define PC_RELATIVE_CODE				067
///#define PC_RELATIVE_DEFERRED_CODE	077
///#define SP_DEFERRED_CODE				016
///#define SP_AUTOINC_CODE				026
///#define SP_AUTOINC_DEFERRED_CODE		036
///#define SP_AUTODEC_CODE				046
///#define SP_INDEX_CODE				066
///#define Sp_INDEX_DEFFERRED_CODE		076



#pragma endregion

#pragma region DOUBLE_OPERAND_INSTRUCTIONS
///-----------------------------------------------
/// Double Operand Instruction Functions
///-----------------------------------------------
//----------------------------------------------------------------------------------------------------
//Function: MOV insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: returns source unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::MOV(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: CMP insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::CMP(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: BIT insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodifeied value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIT(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: BIC insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value not(!) source unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIC(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: BIS insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value or(|) source unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIS(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ADD insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination + source and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ADD(const OctalWord& dest, const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: SUB insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination - source and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SUB(const OctalWord& dest, const OctalWord& src)
{
}
#pragma endregion

#pragma region SINGLE_OPERAND_INSTRUCTIONS
///-----------------------------------------------
/// Single Operand Instruction Functions
///-----------------------------------------------
//----------------------------------------------------------------------------------------------------
//Function: JSR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::JSR(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: CLR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::CLR(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: COM insturction (Single Operand Instruction)
//Input: (OctalWord) ource register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::COM(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: INC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INC(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: DEC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::DEC(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: NEG insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::NEG(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ADC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ADC(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: SBC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SBC(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: TST insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::TST(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ROR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROR(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ROL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROL(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ASR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ASR(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: ASL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ASL(const OctalWord& src)
{
}

//----------------------------------------------------------------------------------------------------
//Function: SUB insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SXT(const OctalWord& src)
{
}

#pragma endregion

#pragma region BRANCH_INSTRUCTIONS
///-----------------------------------------------
/// Branch Instruction Functions
///-----------------------------------------------
/* Description:Provides a way of transferring program control within a 
   range of -128 to +127 words with a one word instruction.
   0 000 000 1xx xxx xxx BR
   PC = PC + (2 * offset)
*/
OctalWord PDP11SimController::BR(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	offset<<1;
	OctalWord pcvalue = pc.getVal() + offset;
	cout <<"BR\n"; 
	WriteBack(0, 7, pcvalue);
}

/* Description: Causes a branch if N and V are either both clear or both set.
   0 000 010 0xx xxx xxx BGE
   PC = PC + (2 * offset) if N or V = 0 */
OctalWord PDP11SimController::BGE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.V == 0 || status.N == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BGE\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:	Tests the state of the Z-bit and causes a branch if the Z-bit is clear.
   0 000 001 0xx xxx xxx  BNE
   PC = PC + (2 * offset) if Z = 0 */
OctalWord PDP11SimController::BNE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.Z == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BNE\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: Tests the state of the Z-bit and causes a branch if Z is set.
0 000 001 1xx xxx xxx  BEQ
PC = PC + (2 * offset) if Z = 1 */
OctalWord PDP11SimController::BEQ(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.Z == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BEQ\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:Tests the state of the N-bit and causes a branch if N is clear. 
   BPL is the complementary operation of BMI.
   1 000 000 0xx xxx xxx  BPL
   PC = PC + (2 * offset) if N = 0 */
OctalWord PDP11SimController::BPL(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.N == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BPL\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: Tests the state of the N-bit and causes a branch if N is set. 
   It is used to test the sign (most significant bit) of the result of the 
   previous operation, branching if negative.
   1 000 000 1xx xxx xxx BMI
   PC = PC + (2 * offset) if N = 0 */
OctalWord PDP11SimController::BMI(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.N == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BMI\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:Tests the state of the V bit and causes a branch if the V bit is clear. 
   BVC is complementary operation to BVS.
   1 000 010 0xx xxx xxx BVC
   PC = PC + (2 * offset) if V = 0 */
OctalWord PDP11SimController::BVC(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.V == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BVC\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: BHIS is the same instruction as BCC. 
   This mnemonic is included only for convenience, 
   used instead of BCC when performing unsigned comparisons, for documentation purposes.
   1 000 011 0xx xxx xxx BHIS
   PC = PC + (2 * offset) if C = 0 */
OctalWord PDP11SimController::BHIS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.C == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BHIS\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: Causes a branch if the "Exclusive Or" of the N and V bits are 1.
   0 000 010 1xx xxx xxx BLT
   PC = PC + (2 * offset) if N or V = 1 */
OctalWord PDP11SimController::BLT(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.N == 1 || status.V == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BLT\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:Operation of BGT is similar to BGE, 
   except BGT will not cause a branch on a zero result.
   0 000 011 0xx xxx xxx BGT
   PC = PC + (2 * offset) if Z or(N xor V) = 0 */
OctalWord PDP11SimController::BGT(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.N == status.V)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BGT\n"; 
		WriteBack(0, 7, pcvalue);
	}

}

/* Description:Operation is similar to BLT but in addition will 
   cause a branch if the resul of the previous operation was zero.
   0 000 011 1xx xxx xxx BLE
   PC = PC + (2 * offset) if Z or(N xor V) = 1 */
OctalWord PDP11SimController::BLE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.N != status.V)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BLE\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: Tests the state of V bit (overflow) and causes a branch 
   if the V bit is set. BVS is used to detect signed arithmetic overflow in the previous operation.
   1 000 010 1xx xxx xxx   BVS */
OctalWord PDP11SimController::BVS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.V == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BVS\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:Causes a branch if the previous operation caused either a carry or a zero result.
   1 000 001 1xx xxx xxx BLOS
   PC = PC + (2 * offset) if C or Z = 1 */
OctalWord PDP11SimController::BLOS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.Z == 1 || status.C == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BLOS\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description:Tests the state of the C-bit and causes a branch if C is clear. 
   BCC is the complementary operation to BCS
   1 000 011 0xx xxx xxx BCC */
OctalWord PDP11SimController::BCC(const OctalWord& src)
{   
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.C == 0)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BCC\n"; 
		WriteBack(0, 7, pcvalue);
	}
}

/* Description: Tests the state of the C-bit and causes a branch if C is set. 
   It is used to test for a carry in the result of a previous operation.
   1 000 011 1xx xxx xxx BCS
   PC = PC + (2 * offset) if C = 1 */
OctalWord PDP11SimController::BCS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;

	if(status.C == 1)
	{
		offset<<1;
		OctalWord pcvalue = pc.getVal() + offset;
		cout <<"BCS\n"; 
		WriteBack(0, 7, pcvalue);
	}
}
#pragma endregion

#pragma region EXTENDED_DOUBLE_OPERAND_INSTRUCTIONS
///----------------------------------
/// Extended Double Operand Instruction Functions
///----------------------------------
OctalWord PDP11SimController::MUL()
{
	cout << "a MUL instruction was detected\n";
}

OctalWord PDP11SimController::DIV()
{
	cout << "a DIV instruction was detected\n";
}

OctalWord PDP11SimController::ASH()
{
	cout << "a ASH instruction was detected\n";
}

OctalWord PDP11SimController::ASHC()
{
	cout << "a ASHC instruction was detected\n";
}

OctalWord PDP11SimController::XOR()
{
	cout << "a XOR instruction was detected\n";
}

OctalWord PDP11SimController::FPO()
{
	cout << "a floating point instruction was detected\n";
}

OctalWord PDP11SimController::SYSINSTRUCTION()
{
	cout << "a system instruction was detected\n";
}

OctalWord PDP11SimController::SOB()
{
	cout << "a SOB instruction was detected\n";
}
#pragma endregion
