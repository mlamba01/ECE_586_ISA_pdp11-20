/*
 ============================================================================
 Name        : processinstruction.c
 Author      :
 Version     : 5-26-2017
 Description : Contains functions for the main macro11 decode logic
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "processinstruction.h"
#include "single_exec.h"
#include "double_exec.h"
#include "branch_exec.h"
#include "IR_Parse.h"
#include "EA_calc.h"


// declare global vars here:
// main memory array, registers, status bits
//
short MemArray[MEMSIZE];

short regs[8];

int psw_N = 0;
int psw_Z = 0;
int psw_V = 0;
int psw_C = 0;

int halt_found = 0;
int register_write_count = 0;

extern int DEBUG;
extern int DISPLAY;

// declare the structs used for parsing
extern single_Op 	SingleOpInstr;
extern double_Op 	DoubleOpInstr;
extern branch_Op 	BranchOpInstr;
extern jump_Op 		JumpOpInstr;
extern subroutine_Op 	SubOpInstr;


// main logic to decode: assumes access to the MemArray[] and that PC/R7
//	pointer is set up correctly
//
void processinstruction() {


	unsigned int IR;

	// grab the next instruction
	//

	IR = ReadMem((unsigned int) regs[7], MEMREAD_INST);

	if (DEBUG) {printf("ProcessInstruction:: Found IR: %06o at PC %06o\n", IR, regs[7]); }

	// the following use of an unsigned short variable is to allow PC to be
	//	treated as an unsigned when incrementing, otherwise anything above
	//	address 0100_000 will be treated as negative, thus +2 will not calculate
	//	correctly.
	//
	unsigned short uPC = regs[7];
	uPC += 2;			// increment PC to next word
	regs[7] = (short) uPC;

	if ( (IR & 0177770) == 0)  { // this is an OPERATE instruction

		if (DEBUG) {printf("ProcessInstruction:: Found OPERATE instruction\n"); }
		if (DISPLAY) { printf("OPR "); }


		switch (IR & 07) {
			case 0:	halt_found=1;
					if (DEBUG) {printf("ProcessInstruction:: \tFound HALT!\n"); }
					break;	// halt
			default:	if (DEBUG) {printf("ProcessInstruction:: ERROR, Found unsupported command!\n"); }

					break;	// found unsupported command
		} // switch
	} // if


	if ( (IR & 0177770) == 0000200 )  {	// SUB RETURN  instruction

		if (DEBUG) {printf("ProcessInstruction:: Found RTS (subroutine return)\n"); }
		if (DISPLAY) { printf("RTS "); }

		unsigned int reg = (IR & 07);	// get the register

		regs[7] 	= regs[reg];		// PC = specified register contents
		regs[6] 	-= 2;			// decrement SP by 2 FIRST
		regs[reg] = ReadMem(regs[6], MEMREAD_DATA);	// register gets popped off stack
	}


	if ( (IR & 0177740) == 0000240 )  {	// OPR: CONDITION CODE instruction

		if (DEBUG) {printf("ProcessInstruction:: Found CONDITION CODE\n"); }
		if (DISPLAY) { printf("CCD "); }
		int setbit = (IR & 020);
		if (setbit) {
			if (IR & 01)	{ SetCarry(1); }	// carry
			if (IR & 02)	{ SetOverFlow(1); }	// oVerflow
			if (IR & 04)	{ SetZero(1); }	// Zero
			if (IR & 010)	{ SetNegative(1); }	// negative
		} else {
			if (IR & 01)	{ SetCarry(0); }	// carry
			if (IR & 02)	{ SetOverFlow(0); }	// oVerflow
			if (IR & 04)	{ SetZero(0); }	// Zero
			if (IR & 010)	{ SetNegative(0); }	// negative

		}
	}


	if ( (IR & 0177700) == 0000100)  {		// JUMP instruction

		if (DEBUG) {printf("ProcessInstruction:: Found JUMP instruction\n"); }
		if (DISPLAY) { printf("JMP "); }

		Parse_jump(IR);
		if (JumpOpInstr.dmode == 0) { printf("ProcessInstruction:: ERROR, mode is 0 (not allowed)\n"); }

		unsigned int destaddr = Eff_Address(JumpOpInstr.dmode, JumpOpInstr.dreg, JumpOpInstr.w);
		regs[7] = ReadMem(destaddr, MEMREAD_DATA);	// (dest) -> (PC)

	}

	if ( (IR & 0177700) == 0000300)  {		// SWAB instruction

		if (DEBUG) {printf("ProcessInstruction:: Found SWAB instruction\n"); }
		if (DISPLAY) { printf("SWB "); }

		unsigned short mode = IR & 070;
		unsigned short reg 	= IR & 007;
		unsigned short contents = 0;
		unsigned short EA = 0;

		if (mode == 0) { contents = regs[reg]; }
		else {
			EA = Eff_Address(mode, reg, 1);
			contents = ReadMem(EA, MEMREAD_DATA);
		}

		if (DEBUG) {printf("ProcessInstruction:: SWAB contents before swap: %04x\n", contents); }

		// now, we finally have the contents.  swap them:
		unsigned short lower = contents & 0xFF;
		if (DEBUG) {printf("ProcessInstruction:: SWAB lower is: %04x\n", lower); }
		contents = contents >> 8;	// upper is now lower
		if (DEBUG) {printf("ProcessInstruction:: SWAB contents now: %04x\n", contents); }

		lower = lower << 8;			// shift up lower byte
		if (DEBUG) {printf("ProcessInstruction:: SWAB lower shifted: %04x\n", lower); }

		contents = contents | lower;	// lower byte is now in upper byte

		if (DEBUG) {printf("ProcessInstruction:: SWAB contents after swap: %04x\n", contents); }

		// now write it all back
		if (mode == 0) { regs[reg] = contents; }
		else 		{ WriteMem(EA, contents); }

	}

	if ( (IR & 0177000) == 0004000)  {		// JSR (SUBROUTINE) instruction

		if (DEBUG) {printf("ProcessInstruction:: Found JSR instruction\n"); }
		if (DISPLAY) { printf("JSR "); }

		Parse_subroutine(IR);
		if (DEBUG) {printf("ProcessInstruction:: SubOpInstr opcode %d, reg %d\n", SubOpInstr.opcode, SubOpInstr.reg); }
		if (DEBUG) {printf("ProcessInstruction:: SubOpInstr dmode %d, dreg %d\n", SubOpInstr.dmode, SubOpInstr.dreg); }

		short temp;	// temp var
		if (SubOpInstr.dmode == 0) { temp = regs[SubOpInstr.dreg]; }
		else {
			unsigned int regaddress = Eff_Address(SubOpInstr.dmode, SubOpInstr.dreg, SubOpInstr.w);
			temp = ReadMem(regaddress, MEMREAD_DATA);	// (dst) -> (temp), grab subroutine address from mem
		}
		if (DEBUG) {printf("ProcessInstruction:: temp = %06o\n", (unsigned short)temp); }

		WriteMem(regs[6], regs[SubOpInstr.reg]);		// push (reg) onto stack

		regs[6] += 2;		// increment SP
		if (DEBUG) {printf("ProcessInstruction:: SP++, now = %06o\n", (unsigned short)regs[6]); }
		regs[SubOpInstr.reg] = regs[7];				// (PC) -> (reg), save PC to come back to after subroutine
		if (DEBUG) {printf("ProcessInstruction:: regs[%d] =  %06o (PC)\n", SubOpInstr.reg, (unsigned short)regs[7]); }

		regs[7] = temp;							// (temp) -> (PC), give PC subroutine address
		if (DEBUG) {printf("ProcessInstruction:: Set PC to temp ( %06o )\n", (unsigned short)regs[7]); }

	}

	if ( (IR & 0177000) == 0104000)  {		// TRAP instruction <-- detect but NOP

		if (DEBUG) {printf("ProcessInstruction:: Found TRAP instruction\n"); }
		if (DISPLAY) { printf("TRAP"); }

	}

	if ( (IR & 0070000) > 0)  {		// DOUBLE OPERAND instruction

		if (DEBUG) {printf("ProcessInstruction:: Found DOUBLE OP instruction\n"); }

		Parse_double(IR);
		double_exec(DoubleOpInstr);

	}


	if ( ( ((IR & 0077400) >= 0000400) &&  ((IR & 0077400) <= 0003400) ) ||
	 	( ((IR & 0777400) >= 0100000) &&  ((IR & 0777400) <= 0103400) ) )
	{	// BRANCH instruction
		if (DEBUG) {printf("ProcessInstruction:: Found BRANCH\n"); }
		Parse_branch(IR);
		branch_exec(BranchOpInstr);

	} // if

	if ( ((IR & 0077000) == 0005000) || ((IR & 0077000) == 0006000)) {	// SINGLE OP
		if (DEBUG) { printf("ProcessInstruction:: Found SINGLE OP\n"); }

		// first, parse the IR and populate the struct
		//
		Parse_single(IR);

		// execute function: get operand, perform op
		//
		single_exec(SingleOpInstr);

	} // if

	// print per-instruction info if DISPLAY is enabled.  The header is already
	//	printed in the main.c code
	//
	if (DISPLAY) { printf("\t%06o %06o %06o %06o %06o %06o %06o %06o z%d n%d v%d c%d\n",
			(unsigned short) regs[0], (unsigned short) regs[1], (unsigned short) regs[2], (unsigned short) regs[3],
			(unsigned short) regs[4], (unsigned short) regs[5], (unsigned short) regs[6], (unsigned short) regs[7],
			psw_Z, psw_N, psw_V, psw_C); }

} // end processinstruction()




// Write to memory function: This will write to a file (file* assigned in main.c)
//	anytime a write to memory occurs.  It updates the memory array and records the
//	write (w/o the data) into the trace file.
//
void WriteMem(unsigned int byte_addr, short value) {	// get rid of FP???

	unsigned int word_addr = byte_addr >> 1;
	// test if it's to a register...
	//	--> registers are from 177_770 (r0), up to 177_716 (r7), so to get reg#
	//		shift right by one bit on the 4 lsb's of byte_addr, or just use word_addr[3:0]
	if ( (byte_addr & 0177700) == 0177700) {
		fprintf(memoutfp, "1 %06o\n", byte_addr);

		short whichreg = word_addr & 07;	// word address is already shifted, just use that
		regs[whichreg] = value;
		if (DEBUG) { printf("WriteMem: writing %06o to register %d\n", value, whichreg); }
		return;
	}
	if (byte_addr == 0177776) {
		if (DEBUG) { printf("WriteMem: writing %06o to PSW register\n", value); }
		fprintf(memoutfp, "1 %06o\n", byte_addr);

		int setbit = (value & 020);
		if (setbit) {
			if (value & 01)	{ SetCarry(1); }	// carry
			if (value & 02)	{ SetOverFlow(1); }	// oVerflow
			if (value & 04)	{ SetZero(1); }	// Zero
			if (value & 010)	{ SetNegative(1); }	// negative
		} else {
			if (value & 01)	{ SetCarry(0); }	// carry
			if (value & 02)	{ SetOverFlow(0); }	// oVerflow
			if (value & 04)	{ SetZero(0); }	// Zero
			if (value & 010)	{ SetNegative(0); }	// negative
		} // end else
		return;
	}

	if (byte_addr < 0177700) {
		MemArray[word_addr] = value; 				// store values by words
		if (DEBUG) { printf("WriteMem: writing %06o to %06o\n", MemArray[word_addr], byte_addr); }
		fprintf(memoutfp, "1 %06o\n", byte_addr);
	}

} // end WriteMem()


// Read from memory function: This will write to a file (file* assigned in main.c)
//	anytime a read from memory occurs.  It returns the memory array value and records the
//	read (w/o the data) into the trace file.
//
// Types of reads:
//	MEMREAD_DATA
//	MEMREAD_INST
//
short ReadMem(unsigned int byte_addr, int readtype) {
	unsigned int word_addr = byte_addr >> 1;

	if (((byte_addr & 1) == 1) && (readtype == MEMREAD_INST)) {
		printf("ReadMem: ERROR, attempt to read instruction at misaligned address %06o\n", byte_addr);
	}

	// test if it's to a register...
	//	--> registers are from 177_770 (r0), up to 177_716 (r7), so to get reg#
	//		shift right by one bit on the 4 lsb's of byte_addr, or just use word_addr[3:0]
	if ( (byte_addr & 0177770) == 0177700) {
		short whichreg = word_addr & 07;	// word address is already shifted, just use that
		if (DEBUG) { printf("ReadMem: reading register %d\n", whichreg); }
		fprintf(memoutfp, "%1d %06o\n", readtype, byte_addr);
		return regs[whichreg];
	}
	if (byte_addr == 0177776) {
		if (DEBUG) { printf("ReadMem: reading PSW register\n"); }
		fprintf(memoutfp, "%1d %06o\n", readtype, byte_addr);
		short pswval = 0;
		if (psw_C) { pswval = pswval | 01; }
		if (psw_V) { pswval = pswval | 02; }
		if (psw_Z) { pswval = pswval | 04; }
		if (psw_N) { pswval = pswval | 010; }
		return pswval;
	}

	if (DEBUG) { printf("ReadMem: returning %06o from byte_addr %06o, word_addr %06o\n", (unsigned short) MemArray[word_addr], byte_addr, word_addr); }
	fprintf(memoutfp, "%1d %06o\n", readtype, byte_addr);
	return MemArray[word_addr];

}


/* set PSW carry bit */
void SetCarry(unsigned int val) {
	psw_C = val & 1;	// just take the LSB just in case of misuse
}

/* set PSW zero bit */
void SetZero(unsigned int val) {
	psw_Z = val & 1;
}

/* set PSW overflow bit */
void SetOverFlow(unsigned int val) {
	psw_V = val & 1;
}

/* set PSW negative bit */
void SetNegative(unsigned int val) {
	psw_N = val & 1;
}

/* write to register */
void WriteReg(int reg, short val) {
	regs[reg] = val;
	register_write_count += 1;
}

//////////////// processinstruction test code /////////////////////
//////////////// processinstruction test code /////////////////////
//////////////// processinstruction test code /////////////////////
//////////////// processinstruction test code /////////////////////
//////////////// processinstruction test code /////////////////////
// this tests the JSR/RTS/SWAB functions
/*
int DEBUG=1;
int DISPLAY=1;

int main (int argc, char *argv[]) {

	memoutfp = fopen("memorytrace.txt", "w");

	if (memoutfp == NULL) {
		printf("Error trying to open memory output file!\n");
		//return(1);
	}

	printf("About to init array...\n");
	int i = 0;
	// initialize mem array
	//
	for (i=0; i<MEMSIZE; i=i+1) {
			MemArray[i] = 0;
	} // for

	// set up the JSR/RTS/SWAB case
	regs[0] = (short) 0xDEAF; //  data to swap (0377)
	regs[1] = 0124;		// address of RTS command
	regs[6] = 0400;		// SP pointer address
	regs[7] = 010;			// set prog start (where JSR command is)

	WriteMem(010,0004001);	// JSR:, R=r0, DD=mode 0, r1
	WriteMem(012,0000300);	// SWAB, R0 should have been reloaded from memory with 0x00FF,
						// after this will be 0xFF00
	WriteMem(014,0000277);	// OPR, set nzvc=1111
	WriteMem(016,0000257);	// OPR, clr nzvc=0000
	WriteMem(020,0000265);	// OPR, set nzvc=.1.1 (0101)
	WriteMem(022,0000262);	// OPR, set nzvc=..1. (0111)
	WriteMem(0124,0000200); // RTS, R=r0

	printf ("Set MemArray[010] = %06o (should be 0004001)\n",(unsigned short) MemArray[010>>1]);
	printf ("Set MemArray[012] = %06o (should be 0000300)\n",(unsigned short) MemArray[012>>1]);
	printf ("Set MemArray[0124] = %06o (should be 0000200)\n",(unsigned short) MemArray[0124>>1]);

	if (DISPLAY) { printf("\t%06o %06o %06o %06o %06o %06o %06o %06o z%d n%d v%d c%d\n",
			(unsigned short) regs[0], (unsigned short) regs[1], (unsigned short) regs[2], (unsigned short) regs[3],
			(unsigned short) regs[4], (unsigned short) regs[5], (unsigned short) regs[6], (unsigned short) regs[7],
			psw_Z, psw_N, psw_V, psw_C); }


	printf ("ISSUING JSR\nISSUING JSR\nISSUING JSR\n");
	processinstruction();	// exec JSR
	if (MemArray[0400>>1] != 0x00FF) { printf("ERROR, r0 didn't get pushed on the stack. [0400]=%04x\n", (unsigned short) MemArray[0400]); }
	if (regs[0] != 012) { printf("ERROR, r0 didn't get PC+2, r0=%06o\n", (unsigned short) regs[0]); }
	if (regs[6] != 0402) { printf("ERROR, SP didn't get incremented properly, r6=%06o\n", (unsigned short) regs[6]); }
	if (regs[7] != 0124) { printf("ERROR, PC didn't get assigned properly, r7=%06o\n", (unsigned short) regs[7]); }

	printf ("ISSUING RTS\nISSUING RTS\nISSUING RTS\n");
	processinstruction();	// exec RTS
	if (regs[0] != 0x00FF) { printf("ERROR, r0 didn't get 0x00FF, r0=%06o\n", (unsigned short) regs[0]); }
	if (regs[6] != 0400) { printf("ERROR, SP didn't get decremented properly, r6=%06o\n", (unsigned short) regs[6]); }
	if (regs[7] != 012) { printf("ERROR, PC should be 012, r7=%06o\n", (unsigned short) regs[7]); }


	printf ("ISSUING SWAB\nISSUING SWAB\nISSUING SWAB\n");
	processinstruction();	// exec swab on r0
	if ( ((unsigned short)regs[0]) != 0xAFDE) { printf("ERROR, unexpected postSWAB value: %04x\n", (unsigned short) regs[0]); }



	if (DISPLAY) { printf("\t n%d z%d v%d c%d\n", psw_N, psw_Z, psw_V, psw_C); }

	printf ("ISSUING OPR1\nISSUING OPR\nISSUING OPR\n");
	processinstruction();	// exec OPR
	if ( ReadMem(0177776, MEMREAD_DATA) != 0xF) { printf("ERROR, unexpected psw value: %x\n", (unsigned short) ReadMem(0177776, MEMREAD_DATA)); }
	if (DISPLAY) { printf("\t n%d z%d v%d c%d\n", psw_N, psw_Z, psw_V, psw_C); }

	printf ("ISSUING OPR2\nISSUING OPR\nISSUING OPR\n");
	processinstruction();	// exec OPR
	if ( ReadMem(0177776, MEMREAD_DATA) != 0x0) { printf("ERROR, unexpected psw value: %x\n", (unsigned short) ReadMem(0177776, MEMREAD_DATA)); }
	if (DISPLAY) { printf("\t n%d z%d v%d c%d\n", psw_N, psw_Z, psw_V, psw_C); }

	printf ("ISSUING OPR3\nISSUING OPR\nISSUING OPR\n");
	processinstruction();	// exec OPR
	if ( ReadMem(0177776, MEMREAD_DATA) != 0x5) { printf("ERROR, unexpected psw value: %x\n", (unsigned short) ReadMem(0177776, MEMREAD_DATA)); }
	if (DISPLAY) { printf("\t n%d z%d v%d c%d\n", psw_N, psw_Z, psw_V, psw_C); }

	printf ("ISSUING OPR4\nISSUING OPR\nISSUING OPR\n");
	processinstruction();	// exec OPR
	if ( ReadMem(0177776, MEMREAD_DATA) != 0x7) { printf("ERROR, unexpected psw value: %x\n", (unsigned short) ReadMem(0177776, MEMREAD_DATA)); }
	if (DISPLAY) { printf("\t n%d z%d v%d c%d\n", psw_N, psw_Z, psw_V, psw_C); }


	return 0;
}
*/
