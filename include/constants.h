#pragma once

#define MEMORYLENGTH 36000
#define NUMGENERALREGISTERS 6

#define HALT_OPCODE 0000000
#define NOP_OPCODE 000240
#define RETURN_FROM_SUBROUTINE_OPCODE 00020

// Processor Status Word Instructions Opcodes
#define SPL_OPCODE 000230
#define CLC_OPCODE 000241
#define CLV_OPCODE 000242
#define CLZ_OPCODE 000244
#define CLN_OPCODE 000250
#define SEC_OPCODE 000261
#define SEV_OPCODE 000262
#define SEZ_OPCODE 000264
#define SEN_OPCODE 000270
#define CCC_OPCODE 000257
#define SCC_OPCODE 000277

// Single Operand Instructions Opcodes
#define JSR_OPCODE 004
#define CLR_OPCODE 0050
#define COM_OPCODE 0051
#define INC_OPCODE 0052
#define DEC_OPCODE 0053
#define NEG_OPCODE 0054
#define ADC_OPCODE 0055
#define SBC_OPCODE 0056
#define TST_OPCODE 0057
#define ROR_OPCODE 0060
#define ROL_OPCODE 0061
#define ASR_OPCODE 0062
#define ASL_OPCODE 0063
#define SXT_OPCODE 0067

// Double Operand Instructions Opcodes
#define MOV_OPCODE 01
#define CMP_OPCODE 02
#define BIT_OPCODE 03
#define BIC_OPCODE 04
#define BIS_OPCODE 05
#define ADD_OPCODE 06
#define SUB_OPCODE 016

// Extended Double Operand Instruction Opcodes
#define MUL_OPCODE 070
#define DIV_OPCODE 071
#define ASH_OPCODE 072
#define ASHC_OPCODE 073
#define XOR_OPCODE 074
#define FLOATING_POINT_OPCODE 075
#define SYSTEM_NSTRUCTION_OPCODE 076
#define SOB_OPCODE 077

// Branch Instructions Opcodes
#define BRANCH_OFFSET_MASK 0x000000FF
#define BR_OPCODE 0001
#define BNE_OPCODE 0002
#define BEQ_OPCODE 0003
#define BPL_OPCODE 0200
#define BMI_OPCODE 0201
#define BVC_OPCODE 0204
#define BVS_OPCODE 0205
#define BHIS_OPCODE 0206
#define BCC_OPCODE 0206
#define BLO_OPCODE 0207
#define BCS_OPCODE 0207
#define BGE_OPCODE 0004
#define BLT_OPCODE 0005
#define BGT_OPCODE 0006
#define BLE_OPCODE 0007
#define BHI_OPCODE 0202
#define BLOS_OPCODE 0203

// Addressing Modes
#define REGISTER_CODE			00
#define REGISTER_DEFERRED_CODE	01
#define AUTOINC_CODE			02
#define AUTOINC_DEFERRED_CODE	03
#define AUTODEC_CODE			04
#define AUTODEC_DEFERRED_CODE	05
#define INDEX_CODE				06
#define INDEX_DEFFERRED_CODE	07
#define PC_IMMEDIATE_CODE			027
#define PC_ABSOLUTE_CODE			037
#define PC_RELATIVE_CODE			067
#define PC_RELATIVE_DEFERRED_CODE	077
#define SP_DEFERRED_CODE			016
#define SP_AUTOINC_CODE				026
#define SP_AUTOINC_DEFERRED_CODE	036
#define SP_AUTODEC_CODE				046
#define SP_INDEX_CODE				066
#define Sp_INDEX_DEFFERRED_CODE		076

#define WORD_OCTAL_LENGTH 6
#define MAX_OCTAL_VALUE 0177777

#define BIT_WIDTH 16
