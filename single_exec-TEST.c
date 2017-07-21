/*
 ============================================================================
 Name        : single_exec.c
 Author      :
 Version     : 3-14-2017
 Description : Low level execution functions for the main macro11 decode logic
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
// #include "processinstruction.h"
#include "IR_Parse.h"
//#include "EA_calc.h"

/*
extern int psw_N;
extern int psw_Z;
extern int psw_V;
extern int psw_C;

extern short regs[8];

extern int DEBUG;
extern int DISPLAY;
*/
short MemArray[32*1024];

short regs[8];

int psw_N = 0;
int psw_Z = 0;
int psw_V = 0;
int psw_C = 0;
int DEBUG = 1;
int DISPLAY = 1;


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



////////////// single_exec ///////////////////
////////////// single_exec ///////////////////
////////////// single_exec ///////////////////

void single_exec(single_Op instr) {
	unsigned int 	effaddr;
	short		dest;	// need extra space for overflow/carry?
	unsigned short carry_out;  // temp for rotate ops

	// grab the operand, use EA if not mode 0
	//
	if (instr.mode > 0) {
		//effaddr 	= Eff_Address(instr.mode, instr.reg, instr.w);  // grab EA
		//dest 	= ReadMem(effaddr, MEMREAD_DATA);
	}
	else  { 	dest = regs[instr.reg];   }



	// switch on opcode, do operation
	//
	switch (instr.opcode) {
		case 0050:				// CLR
					if (DISPLAY) { printf("CLR "); }
					dest = 0;
					SetZero(1);
					SetNegative(0);
					SetCarry(0);
					SetOverFlow(0);
					break;
		case 0051:				// COMplement
					if (DISPLAY) { printf("COM "); }
					dest = ~dest;
					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					SetCarry(1);
					SetOverFlow(0);
					break;

		case 0052:				// INCrement
					if (DISPLAY) { printf("INC "); }
					if (dest == (signed short)0077777)	{ SetOverFlow(1); }	// test BEFORE increment
					else 				{ SetOverFlow(0); }

					dest += 1;

					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					//SetCarry(); //not affected
					break;

		case 0053:				// DECrement
					if (DISPLAY) { printf("DEC "); }
					if (dest == (signed short)0100000 )	{ SetCarry(1); }	// test BEFORE decrement
					else 				{ SetCarry(0); }

					dest -= 1;

					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					//SetOverFlow(); //not affected
					break;

		case 0054:				// NEGation
					if (DISPLAY) { printf("NEG "); }
					dest = ~dest + (short)1;	// make sure we work w/ shorts, not ints

					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					if (dest == (signed short)0100000 )	{ SetOverFlow(1); }
					else 		{ SetOverFlow(0); }
					if (dest == 0)	{ SetCarry(0); }
					else 		{ SetCarry(1); }
					break;

		case 0055:				// ADDC
					if (DISPLAY) { printf("ADDC "); }
					int setC=0;
					int setV=0;
					if ((dest == (signed short)0177777) && psw_C)	{ setC=1; }
					if ((dest == (signed short)0077777) && psw_C)	{ setV=1; }

					dest += psw_C;

					if (setC) {	SetCarry(1); }	// need to set these AFTER doing the add,
					else		{	SetCarry(0); } // but test BEFORE doing the add!
					if (setV) {	SetOverFlow(1); }
					else		{	SetOverFlow(0); }
					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					break;

		case 0056:				// SUBC
					if (DISPLAY) { printf("SUBC "); }
					dest -= psw_C;

					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest < 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					if ((dest == 0) && psw_C)	{ SetCarry(0); }
					else 					{ SetCarry(1); }
					if (dest == (signed short)0100000)	{ SetOverFlow(1); }
					else 				{ SetOverFlow(0); }
					break;

		case 0057:				// TST
					if (DISPLAY) { printf("TST "); }
					// We DON'T want to write back dest, only condition codes are
					// affected, so we actually RETURN from the function call.
					// operation is -(dest), but no need to calc anything.
					//  if dest is 0, -dest = 0
					//  if dest is >0, -dest is <0
					SetCarry(0);
					SetOverFlow(0);
					if (dest == 0)	{ SetZero(1); }
					else 		{ SetZero(0); }
					if (dest > 0)	{ SetNegative(1); }
					else 		{ SetNegative(0); }
					return;
					break;

		case 0060:				// ROR, rotate right
					if (DISPLAY) { printf("ROR "); }
					carry_out = (dest & 1);		// grab the LSB of dest
					dest = dest >> 1;			// shift right 1
					if (psw_C)	{  dest = dest | (short) 0100000; }	// 0 already shifted in

					if (carry_out)	{  SetCarry(1); }			// set if LSB was 1
					else 		{  SetCarry(0); }
					if (dest == 0)	{  SetZero(1); }
					else 		{  SetZero(0); }
					if (dest < 0)	{  SetNegative(1); }
					else 		{  SetNegative(0); }
					SetOverFlow(psw_N ^ psw_C);		// make V = N xor C
					break;

		case 0061:				// ROL, rotate left
					if (DISPLAY) { printf("ROL "); }
					carry_out = 0;			// init carry_out to 0
					if (dest & 0100000) { carry_out = 1; }  // grab the MSB of dest

					dest = dest << 1;					// shift left 1
					if (psw_C)	{  dest = dest | (short) 1; }	// 0 already shifted in

					if (carry_out)	{  SetCarry(1); }			// set if MSB was 1
					else 		{  SetCarry(0); }
					if (dest == 0)	{  SetZero(1); }
					else 		{  SetZero(0); }
					if (dest < 0)	{  SetNegative(1); }
					else 		{  SetNegative(0); }
					SetOverFlow(psw_N ^ psw_C);		// make V = N xor C
					break;

		case 0062:				// ASR, arithmetic shift right
					if (DISPLAY) { printf("ASR "); }
					unsigned short msb = (dest & 0100000);		// grab the MSB of dest
					SetCarry((dest & 1));					// carry gets the LSB
					dest = dest >> 1;						// shift right 1

					if (msb)	{  dest = dest | (short) 0100000; }	// sign extend for negative #s

					if (dest == 0)	{  SetZero(1); }
					else 		{  SetZero(0); }
					if (dest < 0)	{  SetNegative(1); }
					else 		{  SetNegative(0); }
					SetOverFlow(psw_N ^ psw_C);		// make V = N xor C
					break;

		case 0063:				// ASL, arithmetic shift left
					if (DISPLAY) { printf("ASL "); }
					if (dest & 0100000) { SetCarry(1); }	// set if MSB was 1

					dest = dest << 1;					// shift left 1

					if (dest == 0)	{  SetZero(1); }
					else 		{  SetZero(0); }
					if (dest < 0)	{  SetNegative(1); }
					else 		{  SetNegative(0); }
					SetOverFlow(psw_N ^ psw_C);		// make V = N xor C
					break;

	}

	// write back
	//
	if (instr.mode > 0) {
		//WriteMem(effaddr, (short) dest);
	}
	else  {  regs[instr.reg] = dest;   }

} // end single_exec




/*		test code for single_exec		*/
/*		test code for single_exec		*/
/*		test code for single_exec		*/
/*		test code for single_exec		*/


int main () {

	single_Op instr;	// FIELDs: w, opcode, mode, reg
	short expectval;
	unsigned int expectZ, expectN, expectV, expectC;

	int reg = 5;

	instr.w = 1;		// always a word
	instr.mode	= 0;
	instr.reg		= reg;

	printf ("SINGLE TEST: -----> Using reg %d\n", reg);

	// print header
	//printf("opcode\tr0     r1     r2     r3     r4     r5     r6     r7     PSW bits\n");


///// CLR  							//
////  only one case, znvc = 1000 			//
////									//
	instr.opcode	= 050;
	regs[reg]		= (short) 0177777;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// INC: -1 to 0 (z bit), DON'T TEST C
	instr.opcode	= 052;
	regs[reg]		= (short) -1;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	//if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// INC	< -1 (n bit), DON'T TEST C
	instr.opcode	= 052;
	regs[reg]		= (short) -4;

	expectval		= -3;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	//if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// INC	077_777 (v bit), DON'T TEST C
	instr.opcode	= 052;
	regs[reg]		= (short) 0077777;

	expectval		= (short) 0100000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	//if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// DEC: 1 to 0 (z), DON'T TEST V
	instr.opcode	= 053;
	regs[reg]		= 1;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	//if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// DEC:	<1 (n bit), DON'T TEST V
	instr.opcode	= 053;
	regs[reg]		= (short) -4;

	expectval		= (short) -5;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	//if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// DEC:	100_000 (c), DON'T TEST V
	instr.opcode	= 053;
	regs[reg]		= (short) 0100000;

	expectval		= (short) 0077777;

	expectZ		= 0;
	expectN		= 0;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	//if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// NEG: 0 (z=1,c=0, z/c are opposite)
	instr.opcode	= 054;
	regs[reg]		= 0;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// NEG:	positive (n=0,c=1),
	instr.opcode	= 054;
	regs[reg]		= 10;

	expectval		= -10;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// NEG:	100_000 (v)
	instr.opcode	= 054;
	regs[reg]		= (short) 0100000;

	expectval		= (short) 0100000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


// TST: 0 (z),
	instr.opcode	= 057;
	regs[reg]		= 0;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


// TST:	positive (n)
	instr.opcode	= 057;
	regs[reg]		= 10;

	expectval		= 10;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// TST:	negative (n)
	instr.opcode	= 057;
	regs[reg]		= -100;

	expectval		= -100;

	expectZ		= 0;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// COM: zero (z bit set),
	instr.opcode	= 051;
	regs[reg]		= (short) -1;

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// COM: negative (n bit)
	instr.opcode	= 051;
	regs[reg]		= 0;

	expectval		= (short) -1;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


// ADDC: 0 (z)
	instr.opcode	= 055;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ADDC: negative (n)
	instr.opcode	= 055;
	regs[reg]		= (short) -10;
	SetCarry(1);

	expectval		= (short) -9;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ADDC: 177_777 and c=1 (c)
	instr.opcode	= 055;
	regs[reg]		= (short) 0177777;
	SetCarry(1);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ADDC: 077_777 and c=1, (v)
	instr.opcode	= 055;
	regs[reg]		= (short) 0077777;
	SetCarry(1);

	expectval		= (short) 0100000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// SUBC: 1->0 (z, c=0)
	instr.opcode	= 056;
	regs[reg]		= 1;
	SetCarry(1);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// SUBC: negative (n)
	instr.opcode	= 056;
	regs[reg]		= -10;
	SetCarry(0);

	expectval		= -10;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// subc: 0 -> 0 (c=1)
	instr.opcode	= 056;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// subc: 100_000 (v)
	instr.opcode	= 056;
	regs[reg]		= (short) 0100000;
	SetCarry(0);

	expectval		= (short) 0100000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROR: 0, set C=0 before: (z=1, n=0, c=0, v=0)
	instr.opcode	= 060;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROR: negative, set C=1 before: (n), lsb 0 (c=0, v=1)
	instr.opcode	= 060;
	regs[reg]		= (short) 0100000;
	SetCarry(1);

	expectval		= (short) 0140000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROR: negative, set C=1 before: (n), lsb 1 (c=1, v=0)
	instr.opcode	= 060;
	regs[reg]		= (short) 0100001;
	SetCarry(1);

	expectval		= (short) 0140000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROR: positive, set C=0 before: (n=0), lsb 1 (c=1, v=1)
	instr.opcode	= 060;
	regs[reg]		= 1;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROL: 0, set C=0 before: (z=1, n=0, c=0, v=0)
	instr.opcode	= 061;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROL: negative after 170_000: (n=1), prev msb 1 (c=1, v=0)
	instr.opcode	= 061;
	regs[reg]		= (short) 0170000;
	SetCarry(1);

	expectval		= (short) 0160001;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROL: negative after 070_000: (n=1), prev msb 0 (c=0, v=1) (test #30)
	instr.opcode	= 061;
	regs[reg]		= (short) 0070000;
	SetCarry(1);

	expectval		= (short) 0160001;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ROL: positive after 100_000: (n=0), prev msb 1 (c=1, v=1)
	instr.opcode	= 061;
	regs[reg]		= (short) 0100000;
	SetCarry(0);

	expectval		= (short) 0000000;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASR: 0 (n0 c0 v0 z1)
	instr.opcode	= 062;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASR: 1 (n0 c1 v1 z0)
	instr.opcode	= 062;
	regs[reg]		= 1;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASR: 100_000 (n1 c0 v1 z0)
	instr.opcode	= 062;
	regs[reg]		= (short) 0100000;
	SetCarry(0);

	expectval		= (short) 0140000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASR: 100_001 (n1 c1 v0 z0)
	instr.opcode	= 062;
	regs[reg]		= (short) 0100001;
	SetCarry(0);

	expectval		= (short) 0140000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASL: 0 (n0 c0 v0 z1)
	instr.opcode	= 063;
	regs[reg]		= 0;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 0;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASL: 100_000 (n0 c1 v1 z0)
	instr.opcode	= 063;
	regs[reg]		= (short) 0100000;
	SetCarry(0);

	expectval		= 0;

	expectZ		= 1;
	expectN		= 0;
	expectV		= 1;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASL: 070_000 (n1 c0 v1 z0)
	instr.opcode	= 063;
	regs[reg]		= (short) 0070000;
	SetCarry(0);

	expectval		= (short) 0160000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 1;
	expectC		= 0;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

// ASL: 170_000 (n1 c1 v0 z0)
	instr.opcode	= 063;
	regs[reg]		= (short) 0170000;
	SetCarry(0);

	expectval		= (short) 0160000;

	expectZ		= 0;
	expectN		= 1;
	expectV		= 0;
	expectC		= 1;

	single_exec(instr);

	printf("\t%06o  z%d n%d v%d c%d\n", (unsigned short) regs[reg], psw_Z, psw_N, psw_V, psw_C);

	if (regs[instr.reg] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
			instr.reg, (unsigned short) expectval, (unsigned short) regs[instr.reg]);  }

	if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
	if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
	if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
	if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


} // end main
