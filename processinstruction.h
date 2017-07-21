#ifndef _PROCESSINSTRUCTION_H_
#define _PROCESSINSTRUCTION_H_


#include <stdio.h>
#include <stdlib.h>

/*
 ============================================================================
 Name        : processinstruction.h
 Author      :
 Version     : 5/17/17
 Description : Main constants and function declarations for PDP11 high level
 			decode.
 ============================================================================
 */


/* Program Constants */

#define MEMSIZE			(32*1024)		/* 64k bytes in the memory array = 100_000 octal */
#define MEMREAD_DATA		0    		/* values used for memory trace output */
#define MEMREAD_INST		2


//extern FILE*	memoutfp;		/* external file ptr var, initialized in main.c */
 FILE*	memoutfp;		/* used to unit test processinstruction() */



/* Process instruction function  */
void processinstruction();

/* Write to mem function  */
void WriteMem(unsigned int byte_addr, short value);

/* Read from mem function  */
short ReadMem(unsigned int byte_addr, int readtype);

/* set PSW carry bit */
void SetCarry(unsigned int val);

/* set PSW zero bit */
void SetZero(unsigned int val);

/* set PSW overflow bit */
void SetOverFlow(unsigned int val);

/* set PSW negative bit */
void SetNegative(unsigned int val);

/* write to register */
void WriteReg(int reg, short val);




#endif
