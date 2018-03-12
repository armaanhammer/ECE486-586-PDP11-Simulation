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
	createEDOITable();
	ci = OctalWord(0);
	memory = Memory();
	for (int i = 0; i < NUMGENERALREGISTERS; i++)
	{
		r[i] = Register();
	}
	status = StatusRegister();
	sp = Register();
	pc = Register();
}

///-----------------------------------------------
/// Destructor Function
///-----------------------------------------------
PDP11SimController::~PDP11SimController()
{
	delete SO;
	delete DO;
	delete BI;
	delete PSWI;
	delete AM;
	delete EDO;
}
#pragma endregion


//Run the loaded program
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
	SO = new Table<int, OneParamFunc>();
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
	DO = new Table<int, TwoParamFunc>();
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
	AM = new Table<int, AddressModeFunc>();
	AM->add(REGISTER_CODE, this->REGISTER);
	AM->add(REGISTER_CODE, this->REGISTER_DEFERRED);
	AM->add(REGISTER_CODE, this->AUTOINC);
	AM->add(REGISTER_CODE, this->AUTOINC_DEFERRED);
	AM->add(REGISTER_CODE, this->AUTODEC);
	AM->add(REGISTER_CODE, this->AUTODEC_DEFERRED);
	AM->add(REGISTER_CODE, this->INDEX);
	AM->add(REGISTER_CODE, this->INDEX_DEFERRED);
	AM->invalid = this->NULLFUNC;
}

//Create a table for the branch instructions
void PDP11SimController::createBranchTable()
{
	BI = new Table<int, OneParamFunc>();
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
	PSWI = new Table<int, NoParamFunc>();
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
	EDO = new Table<int, NoParamFunc>();
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
	//Obtain the registers octal value
	int regNum = w[0].b;
	//Obtain the addressing mode
	int regAddressMode = w[1].b;
	//Obtain the opcode octal value
	int opcode = w.value >> 6;


	OctalWord operand = (*(AM->find(regAddressMode))) (r[regNum].getVal().value, regNum);

	OctalWord result = (*(SO->find(opcode)))(operand);

	WriteBack(regAddressMode, regNum, result);
}

void PDP11SimController::doDoubleOpInstruction(OctalWord w)
{
	//Obtain the destination registers octal value
	int destNum = w[0].b;
	//Obtain the destination addressing mode
	int destAddressMode = w[1].b;
	//Obtain the source registers octal value
	int srcNum = w[2].b;
	//Obtain the source addressing mode
	int srcAddressMode = w[3].b;
	//Obtain the opcode octal value
	int opcode = w.value >> 12;

	//Handle the addressing mode for the source operand
	OctalWord operandA = (*(AM->find(srcAddressMode))) (r[srcNum].getVal().value, srcNum);
	//Handle the addressing mode for the destination operand
	OctalWord operandB = (*(AM->find(destAddressMode))) (r[destNum].getVal().value, destNum);

	//Calculate the result of the double operation
	OctalWord result = (*(DO->find(opcode))) (operandA, operandB);

	//Write the value write back to the destination register
	WriteBack(destAddressMode, destNum, result);
}

void PDP11SimController::WriteBack(int am, int destReg, OctalWord writenVal)
{
	switch (am)
	{
	//Basic addressing register mode
	case(REGISTER_CODE):
		//Write the result to the destination register
		r[destReg].setval(writenVal);
		break;
	//Indirect addressing register mode (deferred)
	case(REGISTER_DEFERRED_CODE):
		//Write to the location pointed to by the register
		memory.setWord(r[destReg].getVal(),writenVal);
		break;
	//Basic addressing autoincrement mode
	case(AUTOINC_CODE):
		//Write to the location pointed to by the register
		memory.setWord(r[destReg].getVal(), writenVal);
		break;
	//Indirect addressing autoincrement mode (deferred)
	case(AUTOINC_DEFERRED_CODE):
		//Write to the location pointed to by the memory pointed to by the register
		memory.setWord(memory.getWord(r[destReg].getVal()), writenVal);
		break;
	//Basic addressing autodecrement mode
	case(AUTODEC_CODE):
		//Write to the location pointed to by the register
		memory.setWord(r[destReg].getVal(), writenVal);
		break;
	//Indirect addressing autodecrement mode (deferred)
	case(AUTODEC_DEFERRED_CODE):
		//Write to the location pointed to by the memory pointed to by the register
		memory.setWord(memory.getWord(r[destReg].getVal() - 2), writenVal);
		break;
	//Basic addressing index
	case(INDEX_CODE):
		//Write to the location pointed to by the sum of the register and the offset
		break;
	//Indirect addressing 
	case(INDEX_DEFFERRED_CODE):
		//Write to the location pointer to by memory pointed to by the sum of the register and the offset
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
//----------------------------------------------------------------------------------------------------
//Function:		REGISTER (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return the octal word value stored in the specified register
//Description:	Addressing mode for Register addressing (mode 0) returning the operand contained in the
//				register.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::REGISTER(OctalWord regValue, int reg)
{
	//Return the value from the specified register
	return regValue;
}

//----------------------------------------------------------------------------------------------------
//Function:		REGISTER_DEFERRED (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location
//Description:	Addressing mode for Register Deferred addressing (mode 1) return the operand contained
//				in the memory that's pointed to by the register.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::REGISTER_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address
	OctalWord pointerToMemory = memory.getWord(regValue);

	//Return the value from that the pointer points to
	return pointerToMemory;
}

//----------------------------------------------------------------------------------------------------
//Function:		AUTOINC (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location
//Description:  Addressing mode for Autoincrement addressing (mode 2) return the operand contained in
//				the memory that's pointed to by the register. Then the register is incremented by 2.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTOINC(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address
	OctalWord pointerToMemory = memory.getWord(regValue);

	//Increment the value of the register
	r[reg].setval(regValue + 2);

	//Return the value from that the pointer points to
	return pointerToMemory;
}

//----------------------------------------------------------------------------------------------------
//Function:		AUTOINC_DEFERRED (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location pointed to
//				by another memory address
//Description:	Addressing mode for Autoincrement Deferred addressing (mode 3) return the operand
//				contained in the memory location that is pointed to by another memory location which
//				is pointed by the register. Then the register is incremented by 2.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTOINC_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address storing the second pointer
	OctalWord pointerToPointer = memory.getWord(regValue);

	//Increment the value of the register
	r[reg].setval(regValue + 2);

	//Obtain the pointer second pointer
	OctalWord pointerToMemory = memory.getWord(pointerToPointer);

	//Return the value from that the pointer points to
	return pointerToMemory;
}

//----------------------------------------------------------------------------------------------------
//Function:		AUTODEC (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location
//Description:	Addressing mode for Autodecrement addressing (mode 4) decrements the value 
//				of the register by 2. Then returns the operand contained in the memory that's pointed 
//				to by the register.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTODEC(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address
	OctalWord pointerToMemory = memory.getWord(regValue - 2);

	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Return the value from that the pointer points to
	return pointerToMemory;
}

//----------------------------------------------------------------------------------------------------
//Function:		AUTODEC_DEFERRED (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location
//Description:	Addressing mode for Autodecrement Deferred addressing (mode 5) decrements the value 
//				of the register by 2. Then returns the operand contained in the memory that's pointed 
//				by a memory location that's pointed to by the register.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTODEC_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address storing the second pointer
	OctalWord pointerToPointer = memory.getWord(regValue - 2);

	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Obtain the pointer second pointer
	OctalWord pointerToMemory = memory.getWord(pointerToPointer);

	//Return the value from that the pointer points to
	return pointerToMemory;
}

//----------------------------------------------------------------------------------------------------
//Function: INDEX (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location
//Description:	Addressing mode for Index addressing (mode 6) adds the value stored in the register
//				with an offset. The product then points to the operand in memory.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INDEX(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address
	OctalWord pointerToMemory = memory.getWord(regValue);

	//Obtain the value of the offset
	OctalWord offset = pc.getVal() + 2;

	//Add the offset to the memory address
	OctalWord indexMemory = memory.getWord(pointerToMemory + offset);

	//Return the value from that the pointer points to
	return indexMemory;
}

//----------------------------------------------------------------------------------------------------
//Function:		INDEX_DEFERRED (Addressing Mode)
//Input:		OctalWord regValue: Octal word (16-bit) stored in specifed register
//				int reg: The specified register
//Output:		(OctalWord) Return octal word value stored in the specified memory location	
//Description:	Addressing mode for Index Deferred addressing (mode 7) adds the value stored in the 
//				register with an offset. The product then point to a memory location that pointe to
//				a location in memory that contains the operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INDEX_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the pointer to the memory address
	OctalWord pointerToMemory = memory.getWord(regValue);

	//Obtain the value of the offset
	OctalWord offset = pc.getVal() + 2;

	//Add the offset to the memory address that stores the pointer
	OctalWord indexPointer = memory.getWord(pointerToMemory + offset);

	//Obtain the pointer second pointer
	OctalWord indexMemory = memory.getWord(indexPointer);

	//Return the value from that the pointer points to
	return indexMemory;
}
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
OctalWord PDP11SimController::MOV(const OctalWord& dest,const OctalWord& src)
{
	//Declare octalword variables
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//Obtain the most significant bit of the result
	OctalBit mostSignificant = tempSrc.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificant.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (tempSrc.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Clear the V bit
	CLV();

	//Return the source
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: CMP insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::CMP(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//Negate the destination, then add one, and add the result to source
	result = tempDest.operator~();
	result = result.operator+(1);
	result = tempSrc.operator-(result);

	//Obtain the most significant bit of the result
	OctalBit mostSignificantR = result.operator[](5);
	OctalBit mostSignificantS = tempSrc.operator[](5);
	OctalBit mostSignificantD = tempDest.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificantR.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Check if the operand signs are not equal
	if (mostSignificantS.operator!=(mostSignificantD))
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (mostSignificantD.operator==(mostSignificantR)) SEV();
		//Otherwise clear the V bit
		else CLV();
	}

	//Check for carry from the most-significant bit of the result and if true set the C bit
	if (status.V == 1) SEC();
	//Otherwise clear the C bit
	else CLC();

	//Return the unmodifed destination register
	return dest;
}

//----------------------------------------------------------------------------------------------------
//Function: BIT insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodifeied value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIT(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//And the destination and the source
	result = tempDest.operator&(tempSrc);

	//Obtain the most significant bit of the result
	OctalBit mostSignificant = result.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificant.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Clear the V bit
	CLV();

	//Return the unmodifed destination register
	return dest;
}

//----------------------------------------------------------------------------------------------------
//Function: BIC insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value not(!) source unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIC(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempSrc = src;
	OctalWord tempDest = dest;
	const OctalWord zero(0);

	//Negate the source then and with the desination
	result = tempSrc.operator~();
	result = result.operator&(tempDest);

	//Obtain the most significant bit of the result
	OctalBit mostSignificant = result.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificant.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Clear the V bit
	CLV();

	//Return the unmodifed destination register
	return result;
}

//----------------------------------------------------------------------------------------------------
//Function: BIS insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination unmodified value or(|) source unmodified value and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::BIS(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//Or the source and destination together
	result = tempSrc.operator|(tempDest);

	//Obtain the most significant bit of the result
	OctalBit mostSignificant = result.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificant.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Clear the V bit
	CLV();

	//Return the unmodifed destination register
	return result;
}

//----------------------------------------------------------------------------------------------------
//Function: ADD insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination + source and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ADD(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//Add the source and destination together
	result = tempSrc.operator+(tempDest);

	//Obtain the most significant bit of the result, destination, and source
	OctalBit mostSignificantR = result.operator[](5);
	OctalBit mostSignificantS = tempSrc.operator[](5);
	OctalBit mostSignificantD = tempDest.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificantR.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Check if the operand signs are equal
	if (mostSignificantS.operator==(mostSignificantD))
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (mostSignificantS.operator!=(mostSignificantR)) SEV();
		//Otherwise clear the V bit
		else CLV();
	}

	//Check for carry from the most-significant bit of the result and if true set the C bit
	if (status.V == 1) SEC();
	//Otherwise clear the C bit
	else CLC();

	//Return the unmodifed destination register
	return result;
}

//----------------------------------------------------------------------------------------------------
//Function: SUB insturction (Double Operand Instruction)
//Input: (OctalWord) destination & source register
//Output: (OctalWord) Octal result of operation
//Description: return destination - source and modify flags
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SUB(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord result;
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	const OctalWord zero(0);

	//Add the source and destination together
	result = tempSrc.operator-(tempDest);

	//Obtain the most significant bit of the result, destination, and source
	OctalBit mostSignificantR = result.operator[](5);
	OctalBit mostSignificantS = tempSrc.operator[](5);
	OctalBit mostSignificantD = tempDest.operator[](5);

	//Check if the result is negative and if true set the N bit
	if (mostSignificantR.operator==(1)) SEN();
	//Otherwise clear the N bit
	else CLN();

	//Check if the source is equal to zero and if true set the Z bit
	if (result.operator==(zero)) SEZ();
	//Otherwise clear the Z bit
	else CLZ();

	//Check if the operand signs are not equal
	if (mostSignificantS.operator!=(mostSignificantD))
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (mostSignificantS.operator==(mostSignificantR)) SEV();
		//Otherwise clear the V bit
		else CLV();
	}

	//Check for carry from the most-significant bit of the result and if true set the C bit
	if (status.V == 1) SEC();
	//Otherwise clear the C bit
	else CLC();

	//Return the unmodifed destination register
	return result;
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
//Description: Contents of the destination operand are replaced with zeroes.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::CLR(const OctalWord& src)
{
	status.N = false;
	status.Z = true;
	status.V = false;
	status.C = false;
	return OctalWord(0);
}

//----------------------------------------------------------------------------------------------------
//Function: COM insturction (Single Operand Instruction)
//Input: (OctalWord) ource register
//Output: (OctalWord) Octal result of operation
//Description: Replaces the contents of the destination operand by their logical complement (each bit 
//equal to 0 is set and each bit equal to 1 is cleared).
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::COM(const OctalWord& src)
{
	~src;  // do the thing
	// N: set if most significant bit of result is set; cleared otherwise
	OctalWord tempVal = 0100000;  // create mask, MSB = 16th bit
	tempVal = tempVal & src;  // bitwise AND with src
	if(tempVal == 0100000)  // test if MSB set
		status.N = true;
	else
		status.N = false;
	
	if(src == 0)  // Z: set if result is 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;
	
	status.V = false;  // V: cleared
	status.C = true;  // C: set
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: INC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Add one to the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INC(const OctalWord& src)
{
	if(src == 077777)  // V: set if dest was 077777; cleared otherwise
		status.V = true;
	else
		status.V = false;
	
	++src;  // do the thing
	
	// N: set if result < 0; cleared otherwise
	OctalWord MSBmask = 0100000;  // create mask, MSB = 16th bit
	OctalWord MSBtest = src & MSBmask; // bitwise AND with src
	if(MSBtest == MSBmask)  // test if MSB set
		status.N = true;
	else
		status.N = false;
	
	if(src == 0)  // Z: set if result is 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;	
	
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: DEC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Subtract 1 from the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::DEC(const OctalWord& src)
{
	if(src == 0100000)  // V: set if dest was 100000; cleared otherwise
		status.V = true;
	else
		status.V = false;
	
	--src;  // do the thing
	
	// N: set if result < 0; cleared otherwise
	OctalWord MSBmask = 0100000;  // create mask, MSB = 16th bit
	OctalWord MSBtest = src & MSBmask; // bitwise AND with src
	if(MSBtest == MSBmask)  // test if MSB set
		status.N = true;
	else
		status.N = false;
	
	if(src == 0)  // Z: set if result is 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;	
	
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: NEG insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Replaces the contents of the destination operand by its two's complement. Note that 
//word 100000 (or byte 200) is replaced by itself (in two's complement notation the most negative 
//number has no positive counterpart).
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::NEG(const OctalWord& src)
{
	result = ~src;  // do the thing
	
	// N: set if result < 0; cleared otherwise
	OctalWord MSBmask = 0100000;  // create mask, MSB = 16th bit
	OctalWord MSBtest = result & MSBmask; // bitwise AND with result
	if(MSBtest == MSBmask)  // test if MSB set
		status.N = true;
	else
		status.N = false;
	
	if(result == 0) // Z: set if result is 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;
	
	if(result == 0100000)  // V: set if result is 100000; cleared otherwise
		status.V = true;
	else
		status.V = false;
	
	if(result == 0) // C: cleared if the result is 0; set otherwise
		status.C = false;
	else
		status.C = true;
	
	if(src == 0100000)  // prevent two's compliment ambiguity for edge case
		return src;  // word 100000 (or byte 200) is replaced by itself
	else
		return result;
}

//----------------------------------------------------------------------------------------------------
//Function: ADC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Adds the contents of the C-bit into the destination operand. This permits the carry 
//from the addition of the low-order words to be carried into the high-order result. 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ADC(const OctalWord& src)
{
	//V: set if dest was 077777 and (C) was 1; cleared otherwise
	status.V = (src == 077777 && status.C) ? true : false;
	
	//C: set if dest was 177777 and (C) was 1; cleared otherise
	bool tempBool = (src == 0177777 && status.C) ? true : false;
	
	src = src + status.C ? 1 : 0; // add src and type-converted carry
	status.C = tempBool;
	
	//N: set if result < 0; cleared otherwise
	status.N = src < 0;
	
	//Z: set if result = 0; cleared otherwise
	status.Z = (src == 0) ? true : false;
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: SBC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Subtracts the contents of the C-bit from the destination. This permits the carry from 
//the subtraction of two low-order words to be subtracted from the high order part of the result. 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SBC(const OctalWord& src)
{
	//V: set if dest was 100000; cleared otherwise
	status.V = (src == 0100000) ? true : false;
	
	//C: set if dest was 0 and C was 1; cleared otherwise
	bool tempBool = (src == 0 && status.C) ? true : false;
	
	src = src - status.C ? 1 : 0; // add src and type-converted carry
	status.C = tempBool;
	
	//N: set if result < 0; cleared otherwise
	status.N = src < 0;
	
	//Z: set if result = 0; cleared otherwise
	status.Z = (src == 0) ? true : false;
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: TST insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Sets the condition codes N and Z according to the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::TST(const OctalWord& src)
{
	// N: set if result < 0; cleared otherwise
	OctalWord MSBmask = 0100000;  // create mask, MSB = 16th bit
	OctalWord MSBtest = src & MSBmask; // bitwise AND with src
	if(MSBtest == MSBmask)  // test if MSB set
		status.N = true;
	else
		status.N = false;
	
	if(src == 0)// Z: set if result is 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;
	
	status.V = false; // V: cleared
	
	status.C = false; // C: cleared
	
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: ROR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Rotates all bits of the destination operand right one place. The LSB is loaded into the 
//C-bit and the previous contents of the C-bit are loaded into the MSB of the destination.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROR(const OctalWord& src)
{
	//C: loaded with the low-order bit of the destination
	status.C = (src[BIT_WIDTH - 1] == 1) ? true : false;
	
	src >>= 1;  // shift left 1 bit
	src[0] = status.C ? 1 : 0; //fix the 16th bit
	
	//N: set if the high-order bit of the result word is set (result < 0); cleared otherwise
	status.N = src < 0;

	//Z: set if all bits of the result word = 0; cleared otherwise
	status.Z = (src == 0) ? true : false;
	
	//V: loaded from the exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	status.V = status.N ^ status.C;  
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: ROL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Rotate all bits of the destination operand left one place. The MSB is loaded into the 
//C-bit of the status word and the previous contents of the C-bit are loaded into the LSB of the destination.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROL(const OctalWord& src)
{
	//C: loaded with the high-order bit of the destination
	status.C = (src[0] == 1) ? true : false;
	
	src <<= 1;  // shift left 1 bit
	src[BIT_WIDTH - 1] = status.C ? 1 : 0; //fix the 16th bit
	
	//N: set if the high-order bit of the result word is set (result < 0); cleared otherwise
	status.N = src < 0;

	//Z: set if all bits of the result word = 0; cleared otherwise
	status.Z = (src == 0) ? true : false;
	
	//V: loaded from the exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	status.V = status.N ^ status.C;  
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: ASR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Shifts all bits of the destination operand right one place. The MSB is replicated. The 
//C-bit is loaded from the LSB of the destination. ASR(B) performs signed division of the destination 
//by two. 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ASR(const OctalWord& src)
{
	OctalWord tempMSB = 0100000;  // create mask, MSB = 16th bit
	tempMSB = tempMSB & src;  // bitwise AND with src to capture state	
	OctalWord tempLSB = 0000001;  // create mask, LSB = 1st bit
	tempLSB = tempLSB & src;  // bitwise AND with src to capture state
	
	src >>= 1;  // shift right 1 bit
	
	// mitigate logical vs. arithmetic shift ambiguity
	if(tempMSB == 0100000) // test if MSB was set in source
		src = src | tempMSB; // if so, force MSB to be set in result
	
	if(tempMSB == 0100000) 	// N: set if the high-order bit of the result is set (result < 0); cleared otherwise
		status.N = true;
	else
		status.N = false;
	
	if(src == 0) // Z: set if the result = 0; cleared otherwise
		status.Z = true;
	else
		status.Z = false;
	
	if(tempLSB == 0000001) // C: loaded from low-order bit of the destination
		status.C = true;
	else
		status.C = false;

	//V: loaded from the Exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	status.V = status.N ^ status.C;  
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: ASL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Shifts all bits of the destination operand left one place. The LSB is loaded with a 0. 
//The C-bit of the status word is loaded from the MSB of the destination. 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ASL(const OctalWord& src)
{
	//C: loaded with the high-order bit of the destination
	status.C = (src[0] == 1) ? true : false;
	
	src <<= 1;  // shift left 1 bit

	//N: set if the high-order bit of the result is set (result < 0); cleared otherwise
	status.N = src < 0;
	
	//Z: set if the result = 0; cleared otherwise
	status.Z = (src == 0) ? true : false;

	//V: loaded from the exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	status.V = status.N ^ status.C;  
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: SXT insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: If the condition code bit N is set then a -1 is placed in the destination operand: if 
//N bit is clear, then a 0 is placed in the destination operand. This instruction is particularly 
//useful in multiple precision arithmetic because it permits the sign to be extended through multiple words.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SXT(const OctalWord& src)
{
	// the N-bit is replicated through dest: dest = 0 if N is clear; dest = -1 if N is set
	src = (status.N == false) ? 0 : -1; 
	
	//N: unaffected
	
	//Z: set if N bit clear
	status.Z = !status.N;
	
	//V: cleared
	status.V = false;
	
	//C: unaffected
	
	return src;
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
