#include <stdio.h>
#include <stdlib.h>
#include "IR_Parse.h"

/*
 ================================================================================================================================
 Name        : IR_Parse.c
 Author      : Kaustubh Agashe
 Version     :
 Copyright   : Your copyright notice
 Description : 3 struct defined. 1 each for single operand instructions, double operand instructions and branch instructions.
 	 	 	   then 3 functions declared. each reads the respective values and fills up the struct.
 	 	 	   main() function and the bunch of print statements were used for my debugging purposes. the functions take the
 	 	 	   IR value in octal and gives the values of offset,opcode,w(bit 15th),mode 1, src_reg, mode 2 and destination
 	 	 	   register in octal.
 ================================================================================================================================
 */

 extern int DEBUG;

 // declare the structs used for parsing
 single_Op 	SingleOpInstr;
 double_Op 	DoubleOpInstr;
 branch_Op 	BranchOpInstr;
 jump_Op 		JumpOpInstr;
 subroutine_Op 	SubOpInstr;

//function to parse double operand instruction
int Parse_double(unsigned int IR){

	if ((IR & 0100000) > 0)	{ DoubleOpInstr.w=0; }	// set w if bit[15] = 0
	else 				{ DoubleOpInstr.w=1; }

	DoubleOpInstr.opcode 	= (IR & 0070000) >> 12;
		if (DEBUG) { printf("Parse_double: DoubleOpInstr.opcode value=%06o\n",DoubleOpInstr.opcode); }

	DoubleOpInstr.mode1 	= (IR & 0007000) >> 9;
	DoubleOpInstr.reg1 	= (IR & 0000700) >> 6;
	DoubleOpInstr.mode2 	= (IR & 0000070) >> 3;
	DoubleOpInstr.reg2 	= (IR & 0000007);

	return 0;
}


//function to parse single operand instruction

int Parse_single(unsigned int IR){

	if ((IR & 0100000) > 0)	{ SingleOpInstr.w=0; }	// set w if bit[15] = 0
	else 				{ SingleOpInstr.w=1; }

	SingleOpInstr.opcode 	= (IR & 0077700) >> 6;
	 	if (DEBUG) { printf("Parse_single: SingleOpInstr.opcode value=%o\n",SingleOpInstr.opcode); }

	SingleOpInstr.mode 	= (IR & 0000070) >> 3;
	 	if (DEBUG) { printf("Parse_single: SingleOpInstr.mode value=%o\n",SingleOpInstr.mode); }

	SingleOpInstr.reg 	= (IR & 0000007);
	 	if (DEBUG) { printf("Parse_single: SingleOpInstr.reg value=%o\n",SingleOpInstr.reg); }

	return 0;
}


//function to parse branch instruction
int Parse_branch(unsigned int IR){

	if (IR & 0100000)	{ BranchOpInstr.byte = 1; }
	else 			{ BranchOpInstr.byte = 0; }
		if (DEBUG) { printf("Parse_branch: BranchOpInstr.opcode value=%o\n",BranchOpInstr.opcode); }

     BranchOpInstr.opcode = (IR & 0003400) >> 8;
		if (DEBUG) { printf("Parse_branch: BranchOpInstr.opcode value=%o\n",BranchOpInstr.opcode); }

     BranchOpInstr.offset = (IR & 0000377);
		if (DEBUG) { printf("Parse_branch: BranchOpInstr.offset value=%o\n",BranchOpInstr.offset); }

	return 0;
}

//function to parse jump instruction
int Parse_jump(unsigned int IR){

	if ((IR & 0100000) > 0)	{ JumpOpInstr.w=0; }	// set w if bit[15] = 0
	else 				{ JumpOpInstr.w=1; }

     JumpOpInstr.opcode = (IR & 0077700) >> 6;
      	if (DEBUG) { printf("JumpOpInstr.opcode value=%o\n",JumpOpInstr.opcode); }

	JumpOpInstr.dmode = (IR & 0000070) >> 3;
	 	if (DEBUG) { printf("JumpOpInstr.dmode: value=%o\n",JumpOpInstr.dmode); }

	JumpOpInstr.dreg 	= (IR & 0000007);
	 	if (DEBUG) { printf("JumpOpInstr.dreg: value=%o\n",JumpOpInstr.dreg); }

		return 0;
} // parse jump


//function to parse subroutine instruction
int Parse_subroutine(unsigned int IR){

	if ((IR & 0100000) > 0)	{ SubOpInstr.w=0; }	// set w if bit[15] = 0
	else 				{ SubOpInstr.w=1; }

     SubOpInstr.opcode=(IR & 0077000) >> 9;
     	if (DEBUG) { printf("SubOpInstr.opcode value=%o\n",SubOpInstr.opcode); }

     SubOpInstr.reg=(IR & 0000700) >> 6;
          if (DEBUG) { printf("SubOpInstr.reg value=%o\n",SubOpInstr.reg); }

	SubOpInstr.dmode = (IR & 0000070) >> 3;
	 	if (DEBUG) { printf("SubOpInstr.dmode: value=%o\n",SubOpInstr.dmode); }

	SubOpInstr.dreg 	= (IR & 0000007);
	 	if (DEBUG) { printf("SubOpInstr.dreg: value=%o\n",SubOpInstr.dreg); }

	return 0;
}
