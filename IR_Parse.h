#ifndef _IR_PARSE_H_
#define _IR_PARSE_H_

#include <stdio.h>
#include <stdlib.h>
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

 //struct for single operand instructions
 typedef struct singleOp{
 	unsigned int opcode;
 	unsigned int w;
 	unsigned int mode;
 	unsigned int reg;
 }single_Op;

 //struct for double operand instructions
 typedef struct doubleOp{
 	unsigned int opcode;
 	unsigned int w;
 	unsigned int mode1;
 	unsigned int reg1;
 	unsigned int mode2;
 	unsigned int reg2;
 }double_Op;

 //struct for branch instructions
 typedef struct branchOp{
	unsigned int byte;
 	unsigned int opcode;
 	unsigned int offset;
 }branch_Op;

 //struct for jump instructions
 typedef struct jumpOp{
 	unsigned int opcode;
 	unsigned int w;
	unsigned int dmode;
 	unsigned int dreg;
 }jump_Op;

 //struct for subroutine instructions
 typedef struct subroutineOp{
 	unsigned int opcode;
 	unsigned int w;
 	unsigned int reg;
	unsigned int dmode;
 	unsigned int dreg;
 }subroutine_Op;



//function to parse double operand instruction
//
int Parse_double(unsigned int IR);


//function to parse single operand instruction
//
int Parse_single(unsigned int IR);


//function to parse branch instruction
//
int Parse_branch(unsigned int IR);


// function to parse jump instruction
//
int Parse_jump(unsigned int IR);


// function to parse SUB instruction
//
int Parse_subroutine(unsigned int IR);

#endif
