#include "memory.h"
#include "types.h"
#include "constants.h"
#include "pdp11simcontroller.h"
#include "register.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <string.h>

using namespace std;

#pragma region CONSTRUCT_DESTRUCT
///-----------------------------------------------
/// Constructor Function
///-----------------------------------------------
PDP11SimController::PDP11SimController(bool debugMem, bool debugReg)
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
	sp.setval(OctalWord(STACK_STARTING_ADDRESS));
	pc = Register();
	debugMemory = debugMem;
	debugRegisters = debugReg;
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

void PDP11SimController::run()
{
	while(ci != HALT_OPCODE)
	{
		fetch();
		if (ci != NOP_OPCODE) 
		{
			if (!decode())
			{
				cerr << "DECODE FAILED. Skipping Instruction " << ci.print(true);
			}
			(*execute)(ci);
			pc.setval(pc.getVal() + 2);

			if (pc.getVal().value % 2 != 0)
			{
				cerr << "pc is no longer word aligned.  Now terminating execution.  current instruction was: " << ci.print(true) << "\n";
			}

			if (debugMemory || debugRegisters) cout << "just executed " << ci.print(true);
			if (debugMemory) memory.print();
			if (debugRegisters) printRegs();
		}
	}
}

void PDP11SimController::printRegs()
{
	cout << "register contents\n"
		<< "  reg  |   value\n";
	cout << "   0   |  "; r[0].print(); cout << endl;
	cout << "   1   |  "; r[1].print(); cout << endl;
	cout << "   2   |  "; r[2].print(); cout << endl;
	cout << "   3   |  "; r[3].print(); cout << endl;
	cout << "   4   |  "; r[4].print(); cout << endl;
	cout << "   5   |  "; r[5].print(); cout << endl;
	cout << "   6   |  "; r[6].print(); cout << endl;
	cout << "   7   |  "; r[7].print(); cout << endl;
	cout << "   sp  |  "; sp.print(); cout << endl;
	cout << "   pc  |  "; pc.print(); cout << endl;
}

void PDP11SimController::loadProgram(string filename)
{
	ifstream file;
	string line;
	int index = 0;
	char* c_string;		// used as an intermediate for converting string to octal
	bool startAddressFound = false;

	try
	{
		if (debugMemory || debugRegisters) cout << "Opening file\n";
		file.open(filename);
		if (file.is_open())
		{
			if (debugMemory || debugRegisters) cout << filename << " was opened successfully\n";

			while (getline(file, line))
			{
				strcpy(c_string, line.c_str());
				int b[6] = {
					(c_string[1] - '0'),
					(c_string[2] - '0'),
					(c_string[3] - '0'),
					(c_string[4] - '0'),
					(c_string[5] - '0'),
					(c_string[6] - '0')
				};
				int num = b[0] << 15 + b[1] << 12 + b[2] << 9 + b[3] << 6 + b[4] << 3 + b[5];

				switch (c_string[0])
				{
				case '@':
					// change index
					index = num;
					if (!startAddressFound)
					{
						// set pc address
						startAddressFound = true;
						pc.setval(OctalWord(num));
					}
					break;
				case '-':
					// call setword at 2*i, isInstruction, touched
					memory.setWord(OctalWord(index), OctalWord(num), true, true);
					break;
				case '*':
					// set pc address
					startAddressFound = true;
					pc.setval(OctalWord(num));
					continue;
				}

				// incrememt the index
				index += 2;

				if (index % 2 != 0)
				{
					cerr << "PC has become unaligned. Now terminating load.";
				}
			}
		}
	}
	catch (exception e)
	{
		cout << e.what() << "\n\n";
	}
	file.close();

	for (index; index < MEMORYLENGTH; index += 2)
	{
		memory.setWord(OctalWord(index), OctalWord(0));
	}
}

void PDP11SimController::fetch()
{
	ci = pc.getVal();
}

#pragma region JUMP
//----------------------------------------------------------------------------------------------------
//Function: JSR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Operation: temp = dest (temp is an internal processor register)
//			 -(SP) = reg (push reg contents onto processor stack)
//			 reg = PC (PC holds location following JSR; this address now put in reg)
//			 PC = temp (PC now points to subroutine address)
//Condition Codes: Unaffected
//Description: In execution of the JSR, the old contents of the specified register (the "Linkage Register") 
//				are automatically pushed onto the processor stack and new linkage information placed in the 
//				register. Thus subroutines nested within subroutines to any depth may all be called with the 
//				same linkage register. There is no need either to plan the maximum depth at which any 
//				particular subroutine will be called or to include instructions in each routine to save and 
//				restore the linkage pointer. Further, since all linkages are saved in a reentrant manner on 
//				the processor stack, execution of a subroutine reentered and executed by an interrupt service 
//				routine. Execution of the initial subroutine can then be resumed when other requests are 
//				satisfied. This process (called nesting) can proceed to any level.
//
//				In both JSR and JMP instructions the destination address is used to load the program counter, 
//				R7. Thus for example a JSR in destination mode 1 for general register R1 (where (R1) = 100), 
//				will access a subroutine at location 100. This is effectively one level less of deferral 
//				than operate instructions such as ADD.
//
//				In the PDP-11/60, a JSR using addressing mode 0 will result in an "illegal" instruction and a 
//				trap through the trap vector address 4.
//
//				A subroutine called with a JSR reg,dest instruction can access the arguments following the 
//				call with either autoincrement addressing, (reg) +, (if arguments are accessed sequentially)
//				or by indexed addressing, X(reg), (if accessed in random order). These addressing modes may 
//				also be deferred, @(reg) + and @X(reg) if the parameters are operand addresses rather than the
//				operand themselves.
//
//				JSR PC,dest is a special case of the PDP-11 subroutine call suitable for subroutine calls that 
//				transmit parameters through the general registers or on the system stack. The SP and the PC 
//				are the only registers that may be modified by this call.
//
//				Another special case of the JSR instruction is JSR PC,@(SP) + which exchanges the top element 
//				of the processor stack and the contents of the program counter. Use of this instruction allows 
//				two routines to swap program control and resume operation when recalled where they left off. 
//				Such routines are called "co-routines."
//
//				Return from a subroutine is done by the RTS instruction. RTS reg loads the contents of reg into 
//				the PC and pops the top element of the processor stack into the specified register.
//----------------------------------------------------------------------------------------------------
void PDP11SimController::JSR(OctalWord src)
{
	unsigned int regNum = src[2].b;
	unsigned int destNum = src[0].b;
	unsigned int destAddressMode = src[0].b;
	//OctalWord temp = (*(AM[destAddressMode])(r[destNum].getVal());
}

///operation: PC = reg
///			  reg = (SP)+
///condition codes: Unaffected
///Loads the contents of reg into PC and pops the top element of the processor stack into the specified register. 
///Return from a non-reentrant subroutine is typically made through the same register that was used in its call. 
///Thus, a subroutine called with a JSR PC,dest exits with a RTS PC and a subroutine called with a JSR R5,dest 
///may retrieve parameters with addressing modes (R5)+, X(R5), or @X(R5), and finally exits with an RTS R5.
void PDP11SimController::RTS(OctalWord src)
{
}
#pragma endregion

#pragma region TABLE
///-----------------------------------------------
/// Table Creation Functions
///-----------------------------------------------
//Create a table of single operation instructions
void PDP11SimController::createSingleOpTable()
{
	SO = new Table<int, OneParamFunc>();
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
	SO->invalid = this->NULLFUNC1;
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
	DO->invalid = this->NULLFUNC2;
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
	AM->invalid = this->NULLFUNCAM;
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
	BI->invalid = this->NULLFUNC1;
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
	PSWI->invalid = this->NULLFUNC0;
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
	EDO->invalid = this->NULLFUNC0;
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
	if (ci[5] == 0 && ci[4] == 0 && ci[3] == 4)
	{
		execute = this->JSR;
		return true;
	}
	if (ci[5] == 0 && ci[4] == 0 && ci[3] == 0 && ci[2] == 2 && ci[1] == 0)
	{
		execute = this->RTS;
		return true;
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

	OctalWord operand = (*(AM->find(regAddressMode))) (r[regNum].getVal().value, regNum);

	OctalWord result = (*(SO->find(opcode)))(operand);

	WriteBack(regAddressMode, regNum, result);
}

void PDP11SimController::doDoubleOpInstruction(OctalWord w)
{
	//Obtain the destination register octal value
	int destNum = w[0].b;
	//Obtain the destination addressing mode octal value
	int destAddressMode = w[1].b;
	//Obtain the source register octal value
	int srcNum = w[2].b;
	//Obtain the source addressing mode octal value
	int srcAddressMode = w[3].b;
	//Obtain the opcode octal value
	int opcode = w.value >> 12;

	//Create octal word (6-bit value) for the source
	OctalWord operandA = (*(AM->find(srcAddressMode))) (r[srcNum].getVal().value, srcNum);
	//Create octal word (6-bit value) for the source
	OctalWord operandB = (*(AM->find(destAddressMode))) (r[destNum].getVal().value, destNum);

	//Calculate the result octal word (6-bit value) for the result
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
		memory.setWord(r[destReg].getVal(), writenVal);
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
		//Update the PC
		pc.setval(pc.getVal() + 2);
		//Write to the location pointed to by the sum of the register and the offset
		memory.setWord(r[destReg].getVal() + memory.getWord(pc.getVal()), writenVal);
		break;
	//Indirect addressing 
	case(INDEX_DEFFERRED_CODE):
		//Update the PC
		pc.setval(pc.getVal() + 2);
		//Write to the location pointed to by the sum of the register and the offset
		memory.setWord(memory.getWord(r[destReg].getVal() + memory.getWord(pc.getVal())), writenVal);
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
void PDP11SimController::NULLFUNC0()
{
	cout << "some opcode has resulted in a NULLFUNC with no parameters called\n";
}

OctalWord PDP11SimController::NULLFUNC1(const OctalWord& src)
{
	cout << "some opcode has resulted in a NULLFUNC with one parameter:" << src.value << " called\n";
}

OctalWord PDP11SimController::NULLFUNC2(const OctalWord& dest, const OctalWord& src)
{
	cout << "some opcode has resulted in a NULLFUNC with two parameters:";
	cout << dest.value << " and " << src.value << " called\n";
}

OctalWord PDP11SimController::NULLFUNCAM(OctalWord dest, int src)
{
	cout << "some opcode has resulted in a NULL Addressing Mode function with two parameters:";
	cout << dest.value << " and " << src << " called\n";
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

OctalWord PDP11SimController::REGISTER(OctalWord regValue, int reg)
{
	//Obtain the value from the register
	return regValue;
}

OctalWord PDP11SimController::REGISTER_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the value from memory (pointer)
	return memory.getWord(regValue);
}

OctalWord PDP11SimController::AUTOINC(OctalWord regValue, int reg)
{
	//Increment the value of the register
	r[reg].setval(regValue + 2);

	//Obtain the value from memory (pointer)
	return memory.getWord(regValue);
}

OctalWord PDP11SimController::AUTOINC_DEFERRED(OctalWord regValue, int reg)
{
	//Increment the value of the register
	r[reg].setval(regValue + 2);

	//Obtain the value from memory (pointer)
	return memory.getWord(memory.getWord(regValue));
}

OctalWord PDP11SimController::AUTODEC(OctalWord regValue, int reg)
{
	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Obtain the value from memory (pointer)
	return memory.getWord(regValue - 2);
}

OctalWord PDP11SimController::AUTODEC_DEFERRED(OctalWord regValue, int reg)
{
	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Obtain the value from memory (pointer)
	return memory.getWord(memory.getWord(regValue));
}

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
OctalWord PDP11SimController::MOV(const OctalWord& dest, const OctalWord& src)
{
	//Declare octalword variables
	OctalWord tempDest = src;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(tempDest < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(tempDest == 0) ? SEZ() : CLZ();

	//Clear the V bit
	CLV();

	//Return the source
	return tempDest;
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
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;

	//Negate the destination, then add one, and add the result to source
	OctalWord result = tempSrc + ((~tempDest) + 1);

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();

	//Check if the operand signs are not equal
	if (tempSrc[5] != tempDest[5])
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (tempDest[5] == result[5]) SEV();
		//Otherwise clear the V bit
		else CLV();
	}
	else 
	{
		CLV();
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
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	//And the destination and the source
	OctalWord result = tempSrc & tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();

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
	OctalWord tempSrc = src;
	OctalWord tempDest = dest;
	//Negate the source then and with the desination
	OctalWord result = ~tempSrc & tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();
	
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
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	//Or the source and destination together
	OctalWord result = tempSrc | tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();
	
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
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	//Add the source and destination together
	OctalWord result = tempSrc + tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() :CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();

	//Check if the operand signs are not equal
	if (tempSrc[5] != tempDest[5])
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (tempSrc[5] != result[5]) SEV();
		//Otherwise clear the V bit
		else CLV();
	}
	else
	{
		CLV();
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

	//Add the source and destination together
	result = tempSrc - tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result==0) ? SEZ() : CLZ();

	//Check if the operand signs are not equal
	if (tempSrc[5] != tempDest[5])
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (tempSrc[5] != result[5]) SEV();
		//Otherwise clear the V bit
		else CLV();
	}
	else
	{
		CLV();
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
//Function: CLR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Contents of the destination operand are replaced with zeroes.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::CLR(const OctalWord& src)
{
	CLN();
	SEZ();
	CLV();
	CLC();
	
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
	OctalWord tempDest = src;  // do the thing
	tempDest = ~tempDest;
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if most significant bit of result is set; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result is 0; cleared otherwise
	CLV();  // V: cleared
	CLC();  // C: set
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: INC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Add one to the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INC(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = src;
	OctalWord tempSrc = src;
	++tempDest; // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if most significant bit of result is set; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result is 0; cleared otherwise
	(ts == 077777) ? SEV() : CLV(); // V: set if dest was 077777; cleared otherwise
	// C: not affected
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: DEC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Subtract 1 from the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::DEC(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = src;
	OctalWord tempSrc = src;
	--tempDest; // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if most significant bit of result is set; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result is 0; cleared otherwise
	(ts == 0100000) ? SEV() : CLV(); // V: set if dest was 100000; cleared otherwise
	// C: not affected

	return tempDest;
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
	OctalWord ts = src;
	OctalWord tempDest = -ts;  // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result is 0; cleared otherwise
	(ts == 0100000) ? SEV() : CLV(); // V: set if dest was 100000; cleared otherwise
	(tempDest == 0) ? CLC() : SEC();

	// premept potential two's compliment ambiguity for edge case:
	// word 100000 (or byte 200) must be replaced by itself
	return 	(ts == 0100000) ? src : tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: ADC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ADC(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = ts + status.C;  // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result is 0; cleared otherwise
	(ts == 077777 && status.C == 1) ? SEV() : CLV(); // V: set if dest was 077777 and (C) was 1; cleared otherwise
	(ts == 0177777 && status.C == 1) ? SEC() : CLC(); // C: set if dest was 177777 and (C) was 1; cleared otherise
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: SBC insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SBC(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = ts - status.C;  // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(ts == 0100000) ? SEV() : CLV(); // V: set if dest was 100000; cleared otherwise
	(ts == 0 && status.C == 1) ? SEC() : CLC(); // C: set if dest was 177777 and (C) was 1; cleared otherise
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: TST insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: Sets the condition codes N and Z according to the contents of the destination operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::TST(const OctalWord& src)
{
	// do nothing
	OctalWord test = src;
	
	(test < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(test == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	CLV(); // V: cleared
	CLC(); // C: cleared
	
	return src;
}

//----------------------------------------------------------------------------------------------------
//Function: ROR insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROR(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = ts >> 1;  // do the thing
	int andMask = 077777;
	bool overflow = tempDest.overflow;
	tempDest = (tempDest & andMask) | ((overflow) ? 0100000: 0);

	//tempDest[0] = tempSrc[BIT_WIDTH -1];  // rotate the bit shifted out
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(overflow)? SEC() : CLC(); // C: loaded from low-order bit of the destination (interpreting as source instead)
	status.V = status.N ^ status.C;  //V: loaded from the Exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: ROL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ROL(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = ts << 1; // do the thing
	int andMask = 01;
	bool overflow = tempDest.overflow;
	tempDest = (tempDest & andMask) | (overflow ? 1 : 0);  // rotate the bit shifted out

	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(overflow)? SEC() : CLC(); // C: loaded from high-order bit of the destination
	status.V = status.N ^ status.C;  //V: loaded from the Exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	
	return tempDest;
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
	OctalWord ts = src;
	OctalWord tempDest = ts >> 1;  // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(tempDest.overflow) ? SEC() : CLC(); // C: loaded from low-order bit of the destination (interpreting as source instead)
	status.V = status.N ^ status.C;  //V: loaded from the Exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: ASL insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::ASL(const OctalWord& src)
{
	OctalWord ts = src;
	OctalWord tempDest = ts << 1;  // do the thing
	
	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(tempDest.overflow) ? SEC() : CLC(); // C: loaded from low-order bit of the destination (interpreting as source instead)
	status.V = status.N ^ status.C;  //V: loaded from the Exclusive OR of the N-bit and C-bit (as set by the completion of the shift operation)
	
	return tempDest;
}

//----------------------------------------------------------------------------------------------------
//Function: SUB insturction (Single Operand Instruction)
//Input: (OctalWord) source register
//Output: (OctalWord) Octal result of operation
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SXT(const OctalWord& src)
{
	OctalWord tempDest = (status.N == 0) ? OctalWord(0) : OctalWord(-1);  // do the thing
	
	// N: unaffected
	status.Z = !status.N; // Z: set if N bit clear
	status.V = 0; // V: cleared
	// C: unaffected
	
	return tempDest;
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
void PDP11SimController::MUL()
{
	cout << "a MUL instruction was detected and skipped\n";
}

void PDP11SimController::DIV()
{
	cout << "a DIV instruction was detected and skipped\n";
}

void PDP11SimController::ASH()
{
	cout << "a ASH instruction was detected and skipped\n";
}

void PDP11SimController::ASHC()
{
	cout << "a ASHC instruction was detected and skipped\n";
}

void PDP11SimController::XOR()
{
	cout << "a XOR instruction was detected and skipped\n";
}

void PDP11SimController::FPO()
{
	cout << "a floating point instruction was detected and skipped\n";
}

void PDP11SimController::SYSINSTRUCTION()
{
	cout << "a system instruction was detected and skipped\n";
}

void PDP11SimController::SOB()
{
	cout << "a SOB instruction was detected and skipped\n";
}
#pragma endregion

#pragma region PRINT_TO_FILE
bool PRINT_TO_FILE(OctalWord address, char type)
{
	//Declare the file to be opened
	ofstream traceFile;

	//Declare addressing string
	string tempAddr = address.print(true);

	//Open the trace file
	traceFile.open("trace_file.txt", ofstream::out | ofstream::app);

	//Write to the end of the file
	traceFile << type << " " << tempAddr << endl;

	//Close the file
	traceFile.close();

	//Needs a return statement
}
#pragma endregion