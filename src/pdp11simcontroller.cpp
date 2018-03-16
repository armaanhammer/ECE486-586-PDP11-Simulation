#include "mem.h"
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
	pc = Register();
	sp = Register();
	sp.setval(OctalWord(MEMORYLENGTH / 2));
	status = StatusRegister();
	//memory = Mem();
	ci = OctalWord(0);
	for (int i = 0; i < NUMGENERALREGISTERS; i++)
	{
		r[i] = Register();
	}
	sp.setval(OctalWord(STACK_STARTING_ADDRESS));
	debugMemory = debugMem;
	debugRegisters = debugReg;
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
	while (1)
	{
		fetch();
		if (ci == HALT_OPCODE)
		{
			//Print to the trace file (instruction fetch)
			PRINT_TO_FILE(pc.getVal(), 2);

			break;
		}

		//Print to the trace file (instruction fetch)
		PRINT_TO_FILE(pc.getVal(), 2);

		if (ci != NOP_OPCODE)
		{
			if (!decode())
			{
				cerr << "DECODE FAILED. Skipping Instruction " << ci.print(true);
			}
			pc.setval(pc.getVal() + 2);

			if (pc.getVal().value % 2 != 0)
			{
				cerr << "pc is no longer word aligned.  Now terminating execution.  current instruction was: " << ci.print(true) << "\n";
			}

			if (debugMemory || debugRegisters) cout << "just executed " << ci.print(true) << endl;
			if (debugMemory) memory.print();
			if (debugRegisters) printRegs();
			instructionCount++;
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
	cout << "   sp  |  "; sp.print(); cout << endl;
	cout << "   pc  |  "; pc.print(); cout << endl << endl;

	status.print();
}

void PDP11SimController::printMem()
{
	memory.print();
}

void PDP11SimController::loadProgram(string filename)
{
	ifstream file;
	string line;
	int index = 0;
	bool startAddressFound = false;

	try
	{
		if (debugMemory || debugRegisters) cout << "Opening file\n";
		file.open(filename.c_str());
		if (file.is_open())
		{
			if (debugMemory || debugRegisters) cout << filename << " was opened successfully\n";

			while (getline(file, line))
			{
				int b[6] = {
					(line[1] - '0'),
					(line[2] - '0'),
					(line[3] - '0'),
					(line[4] - '0'),
					(line[5] - '0'),
					(line[6] - '0')
				};
				int num = (b[0] << 15) + (b[1] << 12) + (b[2] << 9) + (b[3] << 6) + (b[4] << 3) + b[5];

				switch (line[0])
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
					//cout << memory.getWord(OctalWord(index)).print(true) << endl;

					// incrememt the index
					index += 2;
					break;
				case '*':
					// set pc address
					startAddressFound = true;
					pc.setval(OctalWord(num));
					continue;
				}

				if (index % 2 != 0)
				{
					cerr << "PC has become unaligned. Now terminating load.";
				}
				//memory.print();
			}
		}
	}
	catch (exception e)
	{
		cout << e.what() << "\n\n";
	}
	file.close();

	if (debugMemory) memory.print();
}

void PDP11SimController::fetch()
{
	//Fetch the current instruction
	ci = memory.getWord(pc.getVal());
	r[6].setval(sp.getVal());
	r[7].setval(pc.getVal());
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
	OctalWord temp = getOperand(r[destNum].getVal(), destNum, destAddressMode);
	// modify stack
	sp.setval(sp.getVal() - 2);
	memory.setWord(sp.getVal(), r[regNum].getVal(), false, true);
	r[regNum].setval(pc.getVal());
	pc.setval(temp);
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
	unsigned int regNum = src[0].b;
	pc.setval(r[regNum].getVal());

	//modify stack
	sp.setval(sp.getVal() + 2);
	r[regNum].setval(memory.getWord(sp.getVal()));
}
#pragma endregion

#pragma region DECODE
///-----------------------------------------------
/// Decoding Functions
///-----------------------------------------------
bool PDP11SimController::decode()
{
	// check for too long of word
	if (ci[5] > 2)
	{
		return false;
	}
	if (ci[5] == 0 && ci[4] == 0 && ci[3] == 4)
	{
		JSR(ci);
		return true;
	}
	if (ci[5] == 0 && ci[4] == 0 && ci[3] == 0 && ci[2] == 2 && ci[1] == 0)
	{
		RTS(ci);
		return true;
	}
	// check to see if op is PSWI, if true exec op
	if (checkForPSW(ci[3], ci[4], ci[5]))
	{
		doPSWI(ci);
		return true;
	}
	// check to see if op is Branch Instruction, if true exec Instruction
	if (checkForBranch(ci.value))
	{
		doBranchInstruction(ci);
		return true;
	}
	// check to see if single operand instruction, if true exec instruction
	if (checkForSO(ci))
	{
		doSingleOpInstruction(ci);
		return true;
	}
	// check to see if double operand instruction, if true exec instruction
	if (checkForDO(ci))
	{
		doDoubleOpInstruction(ci);
		return true;
	}
	if (checkUnimplementedDoubleOp(ci))
	{
		doUnimplementedDoubleOp(ci);
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
	unsigned int opcode = (((unsigned int)ci.value) & 0xFF00) >> 8;

	switch (opcode)
	{
	case BR_OPCODE: return true;
	case BNE_OPCODE: return true;
	case BEQ_OPCODE: return true;
	case BPL_OPCODE: return true;
	case BMI_OPCODE: return true;
	case BVC_OPCODE: return true;
	case BVS_OPCODE: return true;
	case BHIS_OPCODE: return true;
	case BLO_OPCODE: return true;
	case BGE_OPCODE: return true;
	case BLT_OPCODE: return true;
	case BGT_OPCODE: return true;
	case BLE_OPCODE: return true;
	case BHI_OPCODE: return true;
	case BLOS_OPCODE: return true;
	default: return false;
	}
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
	switch (opcode)
	{
	case SPL_OPCODE: SPL(); break;
	case CLC_OPCODE: CLC(); break;
	case CLV_OPCODE: CLV(); break;
	case CLZ_OPCODE: CLZ(); break;
	case CLN_OPCODE: CLN(); break;
	case SEC_OPCODE: SEC(); break;
	case SEV_OPCODE: SEV(); break;
	case SEZ_OPCODE: SEZ(); break;
	case SEN_OPCODE: SEN(); break;
	case CCC_OPCODE: CCC(); break;
	case SCC_OPCODE: SCC(); break;
	default:
		cerr << "doPSWI() called with an invalid opcode: " << w.asString();
		break;
	}
}

void PDP11SimController::doSingleOpInstruction(OctalWord w)
{
	int regNum = w[0].b;
	int regAddressMode = w[1].b;
	int opcode = (w.value & 0xFFC0) >> 6;

	OctalWord operand = getOperand(r[regNum].getVal(), regNum, regAddressMode);
	OctalWord result = OctalWord(0);

	switch (opcode)
	{
	case CLR_OPCODE: result = CLR(operand); break;
	case INC_OPCODE: result = INC(operand); break;
	case COM_OPCODE: result = COM(operand); break;
	case DEC_OPCODE: result = DEC(operand); break;
	case NEG_OPCODE: result = NEG(operand); break;
	case ADC_OPCODE: result = ADC(operand); break;
	case SBC_OPCODE: result = SBC(operand); break;
	case TST_OPCODE: result = TST(operand); break;
	case ROR_OPCODE: result = ROR(operand); break;
	case ROL_OPCODE: result = ROL(operand); break;
	case ASR_OPCODE: result = ASR(operand); break;
	case ASL_OPCODE: result = ASL(operand); break;
	case SXT_OPCODE: result = SXT(operand); break;
	default:
		cerr << "doSingleOpInstruction() called with an invalid opcode: " << opcode
			<< " and operand " << operand.asString() << endl;
		break;
	}

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
	int opcode = (w.value & 0xF000) >> 12;

	//Create octal word (6-bit value) for the source
	OctalWord operandA = getOperand(r[srcNum].getVal(), srcNum, srcAddressMode);
	//Create octal word (6-bit value) for the destination
	OctalWord operandB = getOperand(r[destNum].getVal(), destNum, destAddressMode);
	OctalWord result = OctalWord(0);

	switch (opcode)
	{
	case MOV_OPCODE: result = MOV(operandA, operandB); break;
	case CMP_OPCODE: result = CMP(operandA, operandB); break;
	case BIT_OPCODE: result = BIT(operandA, operandB); break;
	case BIC_OPCODE: result = BIC(operandA, operandB); break;
	case BIS_OPCODE: result = BIS(operandA, operandB); break;
	case ADD_OPCODE: result = ADD(operandA, operandB); break;
	case SUB_OPCODE: result = SUB(operandA, operandB); break;
	default:
		cerr << "doDoubleOpInstruction() called with an invalid opcode: " << opcode
			<< " and operands " << operandA.asString() << " " << operandB.asString()
			<< endl;
		break;
	}

	if (srcAddressMode == 2 || srcAddressMode == 3)
	{
		//Increment the value of the register
		switch (srcNum)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			r[srcNum].setval(r[srcNum].getVal() + 2);
			break;
		default:
			break;
		}

	}

	WriteBack(destAddressMode, destNum, result);
}

void PDP11SimController::WriteBack(int am, int destReg, OctalWord writenVal)
{
	OctalWord relativeOffset;
	if (destReg == 6 || destReg == 7)
	{
		am = (8 * am) + destReg;
	}
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
		memory.setWord(r[destReg].getVal(), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(r[destReg].getVal(), 1);
		break;
		//Basic addressing autoincrement mode
	case(AUTOINC_CODE):
		//Write to the location pointed to by the register
		memory.setWord(r[destReg].getVal(), writenVal, false, true);

		//Increment the value of the register
		r[destReg].setval(r[destReg].getVal() + 2);

		//Print to the trace file (data write)
		PRINT_TO_FILE(r[destReg].getVal(), 1);
		break;
		//Indirect addressing autoincrement mode (deferred)
	case(AUTOINC_DEFERRED_CODE):
		//Write to the location pointed to by the memory pointed to by the register
		memory.setWord(memory.getWord(r[destReg].getVal()), writenVal, false, true);

		//Increment the value of the register
		r[destReg].setval(r[destReg].getVal() + 2);

		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord(r[destReg].getVal()), 1);
		break;
		//Basic addressing autodecrement mode
	case(AUTODEC_CODE):
		//Write to the location pointed to by the register
		memory.setWord(r[destReg].getVal(), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(r[destReg].getVal(), 1);
		break;
		//Indirect addressing autodecrement mode (deferred)
	case(AUTODEC_DEFERRED_CODE):
		//Write to the location pointed to by the memory pointed to by the register
		memory.setWord(memory.getWord(r[destReg].getVal()), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord(r[destReg].getVal()), 1);
		break;
		//Basic addressing index
	case(INDEX_CODE):
		//Update the PC
		pc.setval(pc.getVal() + 2);
		//Write to the location pointed to by the sum of the register and the offset
		memory.setWord(r[destReg].getVal() + memory.getWord(pc.getVal()), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(r[destReg].getVal() + memory.getWord(pc.getVal()), 1);
		break;
		//Indirect addressing 
	case(INDEX_DEFFERRED_CODE):
		//Update the PC
		pc.setval(pc.getVal() + 2);
		//Write to the location pointed to by the sum of the register and the offset
		memory.setWord(memory.getWord(r[destReg].getVal() + memory.getWord(pc.getVal())), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord(r[destReg].getVal() + memory.getWord(pc.getVal())), 1);
		break;
		//PC register addressing immediate mode
	case(PC_IMMEDIATE_CODE):
		//Write the result to the destination register
		pc.setval(writenVal);
		break;
		//PC register addressing absolute mode
	case(PC_ABSOLUTE_CODE):
		//Write to the location pointed to by the pc
		memory.setWord(memory.getWord(pc.getVal()), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord(pc.getVal()), 1);
		break;
		//PC register addressing relative mode
	case(PC_RELATIVE_CODE):
		relativeOffset = memory.getWord(pc.getVal());
		//Write to the location pointed to by the offset
		memory.setWord(pc.getVal() + relativeOffset + 2, writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord(pc.getVal() + relativeOffset + 2), 1);
		break;
		//PC register addressing relative deferred mode
	case(PC_RELATIVE_DEFERRED_CODE):
		relativeOffset = memory.getWord(pc.getVal());
		//Write to the location pointed to by the offset
		memory.setWord(memory.getWord(pc.getVal() + relativeOffset + 2), writenVal, false, true);
		//Print to the trace file (data write)
		PRINT_TO_FILE(memory.getWord((memory.getWord(pc.getVal())) + (pc.getVal() + 2)), 1);
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
	case(SP_INDEX_DEFFERRED_CODE):
		break;
	default:
		cerr << "bad addressing mode used in writeback()," << am << destReg << ", " << writenVal.asString() << endl;
		break;
	}
}

void PDP11SimController::doBranchInstruction(OctalWord w)
{
	unsigned int opcode = (((unsigned int)w.value) & 0xFF00) >> 8;

	OctalWord newpc = OctalWord(0);

	switch (opcode)
	{
	case BR_OPCODE: newpc = BR(w); break;
	case BNE_OPCODE: newpc = BNE(w); break;
	case BEQ_OPCODE: newpc = BEQ(w); break;
	case BPL_OPCODE: newpc = BPL(w); break;
	case BMI_OPCODE: newpc = BMI(w); break;
	case BVC_OPCODE: newpc = BVC(w); break;
	case BVS_OPCODE: newpc = BVS(w); break;
	case BHIS_OPCODE: newpc = BHIS(w); break;
	case BLO_OPCODE: newpc = BLO(w); break;
	case BGE_OPCODE: newpc = BGE(w); break;
	case BLT_OPCODE: newpc = BLT(w); break;
	case BGT_OPCODE: newpc = BGT(w); break;
	case BLE_OPCODE: newpc = BLE(w); break;
	case BHI_OPCODE: newpc = BHI(w); break;
	case BLOS_OPCODE: newpc = BLOS(w); break;
	default:
		cerr << "doBranchInstruction() called with an invalid opcode: " << opcode
			<< endl;
		break;
	}

	pc.setval(newpc);
}

void PDP11SimController::doUnimplementedDoubleOp(OctalWord w)
{
	int opnum = w.octbit[4].b;
	switch (opnum)
	{
	case 0:
		MUL();
		break;
	case 1:
		DIV();
		break;
	case 2:
		ASH();
		break;
	case 3:
		ASHC();
		break;
	case 4:
		XOR();
		break;
	case 5:
		FPO();
		break;
	case 6:
		SYSINSTRUCTION();
		break;
	case 7:
		SOB();
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

OctalWord PDP11SimController::getOperand(OctalWord regValue, int reg, int addressMode)
{
	OctalWord operand = OctalWord(0);

	if (reg == 6 || reg == 7)
	{
		addressMode = 8 * addressMode + reg;
	}
	switch (addressMode)
	{
	case REGISTER_CODE:operand = REGISTER(r[reg].getVal().value, reg); break;
	case REGISTER_DEFERRED_CODE: operand = REGISTER_DEFERRED(r[reg].getVal().value, reg); break;
	case AUTOINC_CODE: operand = AUTOINC(r[reg].getVal().value, reg); break;
	case AUTOINC_DEFERRED_CODE: operand = AUTODEC_DEFERRED(r[reg].getVal().value, reg); break;
	case AUTODEC_CODE: operand = AUTODEC(r[reg].getVal().value, reg); break;
	case AUTODEC_DEFERRED_CODE: operand = AUTODEC_DEFERRED(r[reg].getVal().value, reg); break;
	case INDEX_CODE: operand = INDEX(r[reg].getVal().value, reg); break;
	case INDEX_DEFFERRED_CODE: operand = INDEX_DEFERRED(r[reg].getVal().value, reg); break;
	case PC_IMMEDIATE_CODE: operand = PC_IMMEDIATE(pc.getVal().value, reg); break;
	case PC_ABSOLUTE_CODE: operand = PC_ABSOLUTE(pc.getVal().value, reg); break;
	case PC_RELATIVE_CODE: operand = PC_RELATIVE(pc.getVal().value, reg); break;
	case PC_RELATIVE_DEFERRED_CODE: operand = PC_RELATIVE_DEFERRED(pc.getVal().value, reg); break;
	case SP_DEFERRED_CODE: operand = SP_DEFERRED(sp.getVal().value, reg); break;
	case SP_AUTOINC_CODE: operand = SP_AUTOINC(sp.getVal().value, reg); break;
	case SP_AUTOINC_DEFERRED_CODE: operand = SP_AUTOINC_DEFERRED(sp.getVal().value, reg); break;
	case SP_AUTODEC_CODE: operand = SP_AUTODEC(sp.getVal().value, reg); break;
	case SP_INDEX_CODE: operand = SP_INDEXED(sp.getVal().value, reg); break;
	case SP_INDEX_DEFFERRED_CODE: operand = SP_INDEX_DEFERRED(sp.getVal().value, reg); break;
	default:
		cerr << "";
		break;
	}
	return operand;
}

//----------------------------------------------------------------------------------------------------
//Function: REGISTER
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in register mode
//Output:		(OctalWord) the contents contained in the current register
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::REGISTER(OctalWord regValue, int reg)
{
	//Obtain the value from the register
	return regValue;
}

//----------------------------------------------------------------------------------------------------
//Function: REGISTER_DEFERRED
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in register deferred mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::REGISTER_DEFERRED(OctalWord regValue, int reg)
{
	//Print to the trace file (data read)
	PRINT_TO_FILE(regValue, 0);

	//Obtain the value from memory (pointer)
	return memory.getWord(regValue);
}

//----------------------------------------------------------------------------------------------------
//Function: AUTOINC
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in autoincrement mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTOINC(OctalWord regValue, int reg)
{
	//Print to the trace file (data read)
	PRINT_TO_FILE(regValue + 2, 0);

	//Obtain the value from memory (pointer)
	return memory.getWord(regValue);
}

//----------------------------------------------------------------------------------------------------
//Function: AUTOINC_DEFERRED
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in autoincrement deferred mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTOINC_DEFERRED(OctalWord regValue, int reg)
{
	//Increment the value of the register
	r[reg].setval(regValue + 2);

	//Print to the trace file (data read)
	PRINT_TO_FILE(memory.getWord(regValue + 2), 0);

	//Obtain the value from memory (pointer)
	return memory.getWord(memory.getWord(regValue));
}

//----------------------------------------------------------------------------------------------------
//Function: AUTODEC
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in autodecrement mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTODEC(OctalWord regValue, int reg)
{
	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Print to the trace file (data read)
	PRINT_TO_FILE(regValue - 2, 0);

	//Obtain the value from memory (pointer)
	return memory.getWord(regValue - 2);
}

//----------------------------------------------------------------------------------------------------
//Function: AUTODEC_DEFERRED
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in autodecrement deferred mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::AUTODEC_DEFERRED(OctalWord regValue, int reg)
{
	//Decrement the value of the register
	r[reg].setval(regValue - 2);

	//Print to the trace file (data read)
	PRINT_TO_FILE(memory.getWord(regValue - 2), 0);

	//Obtain the value from memory (pointer)
	return memory.getWord(memory.getWord(regValue - 2));
}

//----------------------------------------------------------------------------------------------------
//Function: INDEX
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in index mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INDEX(OctalWord regValue, int reg)
{
	OctalWord location = memory.getWord(pc.getVal() + 2) + regValue;
	OctalWord operand = memory.getWord(location);

	return operand;
}

//----------------------------------------------------------------------------------------------------
//Function: INDEX_DEFERRED
//Input:		(OctalWord) regValue - contents contained in the register
//				(int) reg - current register in index deferred mode
//Output:		(OctalWord) the contents contained in the memory
//Description: 
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::INDEX_DEFERRED(OctalWord regValue, int reg)
{
	OctalWord location = memory.getWord(pc.getVal() + 2) + regValue;
	OctalWord pointer = memory.getWord(location);
	OctalWord operand = memory.getWord(pointer);

	return operand;
}

///-----------------------------------------------
/// Program Counter Addressing Mode
///-----------------------------------------------
//----------------------------------------------------------------------------------------------------
//Function: PC_IMMEDIATE
//Input:		(OctalWord) regValue - contents contained in the register (Not used for PC modes)
//				(int) reg - pc register in immediate mode
//Output:		(OctalWord) the contents contained in the memory
//Description:	Equivalent to using the autoincrement mode with the PC. Providing time improvements for
//				acessing constant operands by including the constant in the memory location immediately
//				following the instruction word.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::PC_IMMEDIATE(OctalWord regValue, int reg)
{
	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Obtain the value pointed to by the PC
	OctalWord location = pc.getVal();

	//Print to the trace file (data read)
	PRINT_TO_FILE(location, 0);

	//Return the pointed to value
	return memory.getWord(location);
}

//----------------------------------------------------------------------------------------------------
//Function: PC_ABSOLUTE
//Input:		(OctalWord) regValue - contents contained in the register (Not used for PC modes)
//				(int) reg - pc register in absolute mode
//Output:		(OctalWord) the contents contained in the memory
//Description:	Equivalent of immediate deferred or autoincrement deferred mode using the PC. The 
//				content of the location following the instruction are taken as the address of the 
//				operand. Immediate data is interperted as an absolute address
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::PC_ABSOLUTE(OctalWord regValue, int reg)
{
	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Obtain the pointer to the location
	OctalWord pointer = memory.getWord(pc.getVal());

	//Print to the trace file (data read)
	PRINT_TO_FILE(memory.getWord(pointer), 0);

	//Return the pointed to locations value
	return memory.getWord(pointer);
}

//----------------------------------------------------------------------------------------------------
//Function: PC_RELATIVE
//Input:		(OctalWord) regValue - contents contained in the register (Not used for PC modes)
//				(int) reg - pc register in relative mode
//Output:		(OctalWord) the contents contained in the memory
//Description:	Equivalent to index mode using the PC. The operand's address is calculated by adding
//				the word that follows the instruction to the updated contents of the PC. PC +2 directs
//				the CPU to the offset that follows the instruction. PC + 4 is summed with this offset
//				to provide the effective address of the operand. Note that PC + 4 is also represents
//				the address of the next instruction in the program.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::PC_RELATIVE(OctalWord regValue, int reg)
{
	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Obtain the offset value from memory
	OctalWord memoryOffset = memory.getWord(pc.getVal());

	//Print to the trace file (read data)
	PRINT_TO_FILE(pc.getVal(), 0);

	//Obtain the offset value from pc
	OctalWord pcOffset = pc.getVal() + 2;

	//Print to the trace file (read data)
	PRINT_TO_FILE(memoryOffset + pcOffset, 0);

	//Return the pointed to locations value
	return memory.getWord(memoryOffset + pcOffset);
}

//----------------------------------------------------------------------------------------------------
//Function: PC_RELATIVE_DEFERRED
//Input:		(OctalWord) regValue - contents contained in the register (Not used for PC modes)
//				(int) reg - pc register in relative deferred mode
//Output:		(OctalWord) the contents contained in the memory
//Description:	Equivalent to index deferred mode using the PC. A pointer to an operand's address is 
//				calculated by adding an offest to the updated PC. The mode is simular to relative mode
//				except one additional level of addressing to obtain the operand. The sum of the offset
//				and the updated PC (PC + 4) serves as a pointer to an address. When the address is
//				retrieved, it can be used to locate the operand.
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::PC_RELATIVE_DEFERRED(OctalWord regValue, int reg)
{
	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Obtain the offset value from memory
	OctalWord memoryOffset = memory.getWord(pc.getVal());

	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Obtain the offset value from pc
	OctalWord pcOffset = pc.getVal();

	//Print to the trace file (read data)
	PRINT_TO_FILE(memory.getWord(memoryOffset + pcOffset), 0);

	//Increment the PC
	pc.setval(pc.getVal() + 2);

	//Return the pointed to locations value
	return memory.getWord(memory.getWord(memoryOffset + pcOffset));
}

///---------------------------------------------------------------------------------
/// Stack Addressing modes
//	Note that the stack grows downward towards the lower addresses as items are push
///---------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//Function: SP_DEFERRED (SP)
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Obtain the operand pointed to on the stack (on the top)
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the operand on top of the stack
	OctalWord topStack = memory.getWord(sp.getVal());

	//Return the operand pointed to by the stack
	return topStack;
}

//----------------------------------------------------------------------------------------------------
//Function: SP_AUTOINC (SP)+
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Obtain the operand from the top of the stack and then pop the operand off the stack
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_AUTOINC(OctalWord regValue, int reg)
{
	//Obtain the operand on top of the stack
	OctalWord topStack = memory.getWord(sp.getVal());

	//Pop the operand off the stack
	sp.setval(sp.getVal() + 2);

	//Return the operand pointed to by the stack
	return topStack;
}

//----------------------------------------------------------------------------------------------------
//Function: SP_AUTOINC_DEFERRED @(SP)+
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Obtain the pointer to an operand from the top of the stack and then pop the pointer
//				off the stack
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_AUTOINC_DEFERRED(OctalWord regValue, int reg)
{
	//Obtain the pointer to an operand on top of the stack
	OctalWord topStack = memory.getWord(sp.getVal());

	//Pop the pointer to an operand off the stack
	sp.setval(sp.getVal() + 2);

	//Return the pointer to an operand pointed to by the stack
	return memory.getWord(topStack);
}

//----------------------------------------------------------------------------------------------------
//Function: SP_AUTODEC -(SP)
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Push a value onto the stack
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_AUTODEC(OctalWord regValue, int reg)
{
	//Push the item onto the stack
	memory.setWord(sp.getVal(), regValue, false, true);

	//Decrememt the stack pointer
	sp.setval(sp.getVal() - 2);

	//Return the operand pushed to the stack
	return regValue;
}

//----------------------------------------------------------------------------------------------------
//Function: SP_INDEXED X(SP)
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Reference any operand in the stack by its positive distance from the top of the stack
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_INDEXED(OctalWord regValue, int reg)
{
	//NOT WORKING YET!!!
	return regValue;
}

//----------------------------------------------------------------------------------------------------
//Function: SP_INDEX_DEFERRED @X(SP)
//Input:		(OctalWord) regValue -
//				(int) reg -
//Output:		(OctalWord)
//Description:	Reference any pointer to an operand in the stack by its positive distance from the
//				top of the stack
//----------------------------------------------------------------------------------------------------
OctalWord PDP11SimController::SP_INDEX_DEFERRED(OctalWord regValue, int reg)
{
	//NOT WORKKING YET!!!
	return regValue;
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
OctalWord PDP11SimController::MOV(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = src;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(tempDest < 0) ? SEN() : CLN();

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
OctalWord PDP11SimController::CMP(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;

	//Negate the destination, then add one, and add the result to source
	OctalWord result = tempSrc + ((~tempDest) + 1);

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

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
OctalWord PDP11SimController::BIT(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	//And the destination and the source
	OctalWord result = tempSrc & tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

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
OctalWord PDP11SimController::BIC(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempSrc = src;
	OctalWord tempDest = dest;
	//Negate the source then and with the desination
	OctalWord result = ~tempSrc & tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

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
OctalWord PDP11SimController::BIS(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	//Or the source and destination together
	OctalWord result = tempSrc | tempDest;

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

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
OctalWord PDP11SimController::ADD(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = dest;
	OctalWord tempSrc = src;
	OctalWord postiveMask = 32767;
	OctalWord negativeMask = 32768;
	//Add the source and destination together
	OctalWord result = tempSrc + tempDest;
	OctalWord tempResult = result;

	//Check if the operand signs are not equal
	if (tempSrc[5] == tempDest[5])
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (tempSrc[5] != tempResult[5])
		{
			result = (src.value > 0) ? result & postiveMask : result | negativeMask;
		}
	}

	//Check if the result is negative and if true set the N bit.  Otherwise clear the N bit
	(result < 0) ? SEN() : CLN();

	//Check if the source is equal to zero and if true set the Z bit.  Otherwise clear the Z bit
	(result == 0) ? SEZ() : CLZ();

	//Check if the operand signs are not equal
	if (tempSrc[5] == tempDest[5])
	{
		//Check if arithmetic overflow occured and if true set the V bit
		if (tempSrc[5] != tempResult[5]) SEV();
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
OctalWord PDP11SimController::SUB(const OctalWord& src, const OctalWord& dest)
{
	//Declare octalword variables
	OctalWord tempDest = dest;
	return ADD(src, -tempDest);
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
	OctalWord result = src;
	if (result == 077777)
	{
		result = OctalWord(0);
		CLN();
		SEZ();
	}
	else
	{
		++result; // do the thing
		(result == 0) ? SEZ() : CLZ();
		(result < 0) ? SEN() : CLN(); // N: set if most significant bit of result is set; cleared otherwise
	}
	(ts == 077777) ? SEV() : CLV(); // V: set if dest was 077777; cleared otherwise
									// C: not affected

	return result;
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
	OctalWord result = src;
	if (result == 0100000)
	{
		result = OctalWord(-1);
		SEN();
		CLZ();
	}
	else
	{
		--result; // do the thing
		(result == 0) ? SEZ() : CLZ();
		(result < 0) ? SEN() : CLN(); // N: set if most significant bit of result is set; cleared otherwise
	}
	(ts == 0100000) ? SEV() : CLV(); // V: set if dest was 077777; cleared otherwise
									// C: not affected

	return result;
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

	//fix edge case: if most positive number, and if Carry asserted, return 0
	tempDest = (ts == 077777 && status.C == 1) ? OctalWord(0) : tempDest;
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

	//fix edge case: if most negative number, and if Carry asserted, return -1
	tempDest = (ts == 0100000 && status.C == 1) ? 0177777 : tempDest;
	
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
	tempDest = (tempDest & andMask) | ((overflow) ? 0100000 : 0);

	//tempDest[0] = tempSrc[BIT_WIDTH -1];  // rotate the bit shifted out

	(tempDest < 0) ? SEN() : CLN(); // N: set if result < 0; cleared otherwise
	(tempDest == 0) ? SEZ() : CLZ(); // Z: set if result = 0; cleared otherwise
	(overflow) ? SEC() : CLC(); // C: loaded from low-order bit of the destination (interpreting as source instead)
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
	(overflow) ? SEC() : CLC(); // C: loaded from high-order bit of the destination
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

	offset <<= 1;
	OctalWord pcvalue = pc.getVal() + offset;
	return pcvalue;
}

/* Description: Causes a branch if N and V are either both clear or both set.
0 000 010 0xx xxx xxx BGE
PC = PC + (2 * offset) if N or V = 0 */
OctalWord PDP11SimController::BGE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.V == 0 || status.N == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:	Tests the state of the Z-bit and causes a branch if the Z-bit is clear.
0 000 001 0xx xxx xxx  BNE
PC = PC + (2 * offset) if Z = 0 */
OctalWord PDP11SimController::BNE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.Z == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: Tests the state of the Z-bit and causes a branch if Z is set.
0 000 001 1xx xxx xxx  BEQ
PC = PC + (2 * offset) if Z = 1 */
OctalWord PDP11SimController::BEQ(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.Z == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Tests the state of the N-bit and causes a branch if N is clear.
BPL is the complementary operation of BMI.
1 000 000 0xx xxx xxx  BPL
PC = PC + (2 * offset) if N = 0 */
OctalWord PDP11SimController::BPL(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.N == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: Tests the state of the N-bit and causes a branch if N is set.
It is used to test the sign (most significant bit) of the result of the
previous operation, branching if negative.
1 000 000 1xx xxx xxx BMI
PC = PC + (2 * offset) if N = 0 */
OctalWord PDP11SimController::BMI(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.N == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Tests the state of the V bit and causes a branch if the V bit is clear.
BVC is complementary operation to BVS.
1 000 010 0xx xxx xxx BVC
PC = PC + (2 * offset) if V = 0 */
OctalWord PDP11SimController::BVC(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.V == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: BHIS is the same instruction as BCC.
This mnemonic is included only for convenience,
used instead of BCC when performing unsigned comparisons, for documentation purposes.
1 000 011 0xx xxx xxx BHIS
PC = PC + (2 * offset) if C = 0 */
OctalWord PDP11SimController::BHIS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.C == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: Causes a branch if the "Exclusive Or" of the N and V bits are 1.
0 000 010 1xx xxx xxx BLT
PC = PC + (2 * offset) if N or V = 1 */
OctalWord PDP11SimController::BLT(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.N == 1 || status.V == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Operation of BGT is similar to BGE,
except BGT will not cause a branch on a zero result.
0 000 011 0xx xxx xxx BGT
PC = PC + (2 * offset) if Z or(N xor V) = 0 */
OctalWord PDP11SimController::BGT(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.N == status.V)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Operation is similar to BLT but in addition will
cause a branch if the resul of the previous operation was zero.
0 000 011 1xx xxx xxx BLE
PC = PC + (2 * offset) if Z or(N xor V) = 1 */
OctalWord PDP11SimController::BLE(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.N != status.V)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: Tests the state of V bit (overflow) and causes a branch
if the V bit is set. BVS is used to detect signed arithmetic overflow in the previous operation.
1 000 010 1xx xxx xxx   BVS */
OctalWord PDP11SimController::BVS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.V == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Causes a branch if the previous operation caused either a carry or a zero result.
1 000 001 1xx xxx xxx BLOS
PC = PC + (2 * offset) if C or Z = 1 */
OctalWord PDP11SimController::BLOS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.Z == 1 || status.C == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description:Tests the state of the C-bit and causes a branch if C is clear.
BCC is the complementary operation to BCS
1 000 011 0xx xxx xxx BCC */
OctalWord PDP11SimController::BCC(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.C == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

/* Description: Tests the state of the C-bit and causes a branch if C is set.
It is used to test for a carry in the result of a previous operation.
1 000 011 1xx xxx xxx BCS
PC = PC + (2 * offset) if C = 1 */
OctalWord PDP11SimController::BCS(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.C == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

OctalWord PDP11SimController::BLO(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.C == 1)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
}

OctalWord PDP11SimController::BHI(const OctalWord& src)
{
	int offset = src.value & BRANCH_OFFSET_MASK;
	OctalWord pcvalue = pc.getVal();

	if (status.Z == 0 || status.C == 0)
	{
		offset <<= 1;
		pcvalue = pcvalue + offset;
	}
	return pcvalue;
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
bool PDP11SimController::PRINT_TO_FILE(OctalWord address, char type)
{
	//Declare the file to be opened
	ofstream traceFile;

	try
	{
		//Declare addressing string
		string tempAddr = address.print(true);

		//Open the trace file
		traceFile.open("trace_file.txt", ofstream::out | ofstream::app);

		//Write to the end of the file
		traceFile << (int)type << " " << tempAddr << endl;
	}
	catch (exception e)
	{
		cout << e.what() << "\n\n";
		return false;
	}
	traceFile.close();
	return true;
}
#pragma endregion
