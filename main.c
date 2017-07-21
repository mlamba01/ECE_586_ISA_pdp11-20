/*
 * main.c
 *
 * Entry point of the PDP11 Simulator
 *
 *  Created on: Feb 26, 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include "processinstruction.h"

// extern for testing
extern short MemArray[MEMSIZE];
extern short regs[8];
extern int halt_found;

// global but not extern vars
int DEBUG = 1;		// for debug, verbose
int DISPLAY = 1;	// for extra display info, for demo

FILE *memoutfp = NULL;	// make this global for memory trace file logging...


/*
 *  import the errors_found flag
 */
int errors_found;


int main(int argc, char *argv[])
{
	//assert(sizeof(short)==2);
	FILE *objfp = NULL;		// vars to facilitate obj file reading
	char readvalue[3];		// first char in each line
	int objvalue = 0;		// number value following char
	int line_num = 0;
	int found_progstart = 0;
	unsigned int mempointer = 0;	// MemArray element pointer, for populating memory
	unsigned int i;		// fscanf return value, temp var for array init


/*	attempt to open memory regions file */
	if (DEBUG) { printf("About to open obj and mem files...\n"); }

	if (argc == 1) {
		printf("ERROR, nothing specified!  Usage: pdp11 objfile <octal start addr> <display> <debug>\n");
		return 0;
	}
	if (argc > 1) {
		if (DEBUG) { printf("ASCII file specified: %s\n", argv[1]); }
		objfp = fopen(argv[1], "r");
	}
	if (argc > 2) {
		regs[7] = (short) strtoul(argv[2],NULL,8);  // if start addr specified, grab it
		if (DEBUG) { printf("Start addr specified: %06o\n", (unsigned short) strtoul(argv[2],NULL,8) ); }
		found_progstart = 1;
	}
	if (argc > 3) {
		if (DEBUG) { printf("DISPLAY enabled!\n"); }
		DISPLAY = 1;
	}
	if (argc > 4) {
		if (DEBUG) { printf("DEBUG enabled!\n"); }
		DEBUG = 1;
	}


	// check obj file
	if (objfp == NULL) {
		printf("Error trying to open ascii file!\n");
		return(1);
	}

	memoutfp = fopen("memorytrace.txt", "w");
	if (memoutfp == NULL) {
		printf("Error trying to open memory output file!\n");
		//return(1);
	}

	if (DEBUG) { printf("About to init array...\n"); }

	// initialize mem array
	//
	for (i=0; i<MEMSIZE; i=i+1) {
			MemArray[i] = 0;
	} // for

	if (DEBUG) { printf("About to read in obj file...\n"); }

	// read obj file into mem array (populate memory)
	//
	while (objfp != NULL)  {
			line_num += 1;
			i = fscanf(objfp, " %1c%6o ", readvalue, &objvalue);
			if (i == -1) { break; } 	// found end of file
			if (i != 2) {			// weird, dunno what to do with this, print err and continue
				if (DISPLAY) { printf("ERROR reading ascii file line %d, fscanf returned %d\n", line_num, i); }
				continue;
			}
			if (i == 2) {			// what we expected

				if (DEBUG) { printf("Found character %s and value %6o:\n", readvalue, objvalue); }

				switch (readvalue[0]) {
					case '@': mempointer = objvalue;	// set current address
							if (DEBUG) { printf("\tFound '@', setting address to %06o\n", objvalue); }
							break;
					case '-': // Found data to write, populate memory
							MemArray[mempointer] = objvalue;
							if (DEBUG) { printf("\tFound '-', setting address %06o to %06o\n", (mempointer<<1), objvalue); }
							mempointer += 1;
							break;
					case '*': // set program start address, this takes precedence
							// anything from command line
							regs[7] = objvalue;
							found_progstart = 1;
							break;

				} // switch
			} // if
	} // while

	fclose(objfp);

	if (!found_progstart) {
		printf("ERROR, no program start address specified (command line or in ascii file), exiting...\n");
		return 0;
	}

	if (DEBUG) {printf("Main:: Starting program at PC %06o\n", (unsigned short) regs[7]); }
	while (!halt_found) {
		processinstruction();
	} // while
	printf("HALT FOUND, stopping.\n");


	// print out stats here??
	//	number of instructions processed
	//	lines in memory that aren't 0


/* test output code for MemArray, only prints !0 entries */
/* test output code for MemArray, only prints !0 entries */
if (DEBUG) {
	for (i=0; i<MEMSIZE; i=i+1) {
		if (MemArray[i]) {
			printf("main: MemArray[%06o] / Address %06o = %06o\n", i, (i<<1), (unsigned short) MemArray[i]);
		} // if
	} // for
}
/*	test code for ReadMem, WriteMem  */
/*	test code for ReadMem, WriteMem  */

/*
	if (memoutfp == NULL) {
		printf("Error trying to open memorytrace.txt file!\n");
		return(1);
	}
	else {
		for (i=0100; i<0110; i=i+2) {
			WriteMem(i, i); // write address with address value

			if (ReadMem(i, MEMREAD_INST) == i) {
				printf ("write to %03o verified!\n", i);
			}
		} // for
	} // else
*/


	fclose(memoutfp);

	return (0);
}
