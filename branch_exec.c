#include <stdio.h>
#include <stdlib.h>
#include "processinstruction.h"
#include "IR_Parse.h"
#include "EA_calc.h"
#include "branch_exec.h"

extern int psw_N;
extern int psw_Z;
extern int psw_V;
extern int psw_C;

extern short regs[8];

extern int DEBUG;
extern int DISPLAY;

void branch_exec(branch_Op B)
{
//int PC;
int BA = 0;
//int regs[7] = 013000;
int x;
unsigned int mask = 0xff;
int offset;
mask = mask << 8;
offset = B.offset;
//if (DEBUG) { printf("mask = %o", mask); }

//B.offset = 0366;
//offset = 0032;

//if ((B.offset & 0200) == 0)
//B.offset = B.offset;
//else B.offset = mask | B.offset;

if ((offset & 0200) == 0)
offset = offset;
else offset = mask | offset;

if (DEBUG) { printf(" offset is %o ", offset); }
                                                      // BA(branch address) is a global variable
BA = regs[7] + (2* offset);
if (DEBUG) { printf("BA is %o ",BA); }

// for debug only
/*int oregs[7]ode = 00004;
int psw_N= 0;
int psw_Z = 0;
int psw_V = 1;
int psw_C= 1;
*/



// Main switch statement for  conditional branch instruction
// Word instruction
if ( B.byte == 1) {
	switch (B.opcode) {
		case 00004:                           // unconditional branch: BR
			regs[7] = BA;
			break;

		case 00014:                                // Branch if equal: BEQ
			if(psw_Z == 1) { regs[7] = BA; }
			break;

		case 00010:                               // Branch if not equal: BNE
			if(psw_Z == 0) { regs[7] = BA; }
			break;

		case 00024: 					// Signed conditional branches
			if(psw_N != psw_V)			// Branch if less than: BLT (branch if CMP on neg src and pos dest.(even with overflow),
			regs[7] = BA;				// No branch if pos src and neg dest, no branch if res of prev op is zeo(w/o overflow))
			break;

		case 00020:
			if (psw_N == psw_V) { regs[7] = BA; }   // Branch if greater than or equal: BGE
			break;

		case 00034:                        // Branch if less than or equal: BLE
			if(psw_Z == 1 || (psw_N != psw_V)) { regs[7] = BA; }
			break;

		case 00030:                        // Branch if greater than: BGT
			if(psw_Z == 1 || (psw_N == psw_V)) { regs[7] = BA; }
			break;

		default:
			printf("Invalid conditional branch instruction");
			break;

	} // switch

} // if


else if (B.byte == 0) {
	// Byte Instruction; Unsigned Conditional Branches
switch(B.opcode) {
	case 01010:                        // Branch if higher than: BHI      //unsigned conditional branches
		if(psw_C == 0 && psw_Z == 0) { regs[7] = BA; }
		break;

	case 01014:                        // Branch if lower or same: BLOS
		if(psw_C == 1 || psw_Z == 1) { regs[7] = BA; }
		break;

	case 01030:                        // Branch if carry clear/ higher or same: BCC/BHIS
		if(psw_C == 0) { regs[7] = BA; }
		break;

	case 01034:                        // Branch if carry set, if lower than: BCS/ BLO
		if(psw_C == 1) { regs[7] = BA; }
		break;

	case 01000:                              // Branch if plus: BPL
		if(psw_N == 0) { regs[7] = BA; }
		break;

	case 01004:                              // Branch if minus: BMI
		if(psw_N == 1) { regs[7] = BA; }
		break;

	case 01024:                           // Branch if overflow set: BVS
		if(psw_V == 1) { regs[7] = BA; }
		break;

	case 01020:                           // Branch if overflow clear: BVC
		if(psw_V == 0) { regs[7] = BA; }
		break;

	default:
		if (DEBUG) { printf(" Invalid conditional branch instruction"); }
		break;

//if (DEBUG) { printf("regs[7] = %d", regs[7]); }

	} // switch

} // else if

} // branch_exec
