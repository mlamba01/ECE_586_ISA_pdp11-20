#include <stdio.h>
#include <stdlib.h>
#include "processinstruction.h"
#include "IR_Parse.h"
#include "EA_calc.h"
#include "double_exec.h"


extern int psw_N;
extern int psw_Z;
extern int psw_V;
extern int psw_C;

extern short regs[8];

extern int DEBUG;
extern int DISPLAY;


void double_exec(double_Op IR2) {
	short src, dst;
	unsigned int EA_dst, EA_src;
	short result, r; 		//unsigned int result, r;
	unsigned int result_int, src_int, dst_int;

	if(IR2.mode1 == 0){
		src = regs[IR2.reg1];				//Register Mode
	}
	else{
		EA_src = Eff_Address(IR2.mode1, IR2.reg1, IR2.w);			//Other Modes
		src = ReadMem(EA_src, MEMREAD_DATA);
	}

	if(IR2.mode2 == 0){
		dst = regs[IR2.reg2];				//Register Mode
	}
	else{
		EA_dst = Eff_Address(IR2.mode2, IR2.reg2, IR2.w);				//Other Modes
		dst = ReadMem(EA_dst, MEMREAD_DATA);
	}

	src_int =(int)src;
	dst_int = (int)dst;

	switch(IR2.opcode){
		case 01:	dst = src;			//MOV instruction
					if (IR2.mode2 == 0){
						regs[IR2.reg2] = dst;						//if mode 0 is used then we write the destination register
						//printf("\n we are in the case -if statement");
					}
					else{
						WriteMem(EA_dst, dst);
					}
					if (src == 0){
						SetZero(1); //= 1;
					}
					else{
						SetZero(0); // = 0;
					}

					if (src < 0){
						SetNegative(1); //N = 1;
					}
					else{
						SetNegative(0); //N = 0;
					}

					//SetCarry(psw_C); //C = C;
					SetOverFlow(0); //V = 0;
					break;

		case 06:  	result = src + dst;					//ADD Instruction
					result_int = src_int + dst_int;
					printf("ADD Result = %d \n", result);
					if (IR2.mode2 == 0){
						regs[IR2.reg2] = result;					//if mode 0 is used then we write the destination register
					}
					else{
						WriteMem(EA_dst, result);
					}
					if (result == 0){
						SetZero(1); //Z = 1;
					}
					else{
						SetZero(0); //Z = 0;
					}
					if (result<0){
						SetNegative(1); //N = 1;
					}
					else{
						SetNegative(0); //N = 0;
					}

					////////// test for carry, if src[15] and dst[15] are both 1, C=1
					//
					//if ((((src & 0100000)>>15)==1) && (((dst & 0100000)>>15)==1)) { SetCarry(1); } //C = 1; }
					//else 	{ SetCarry(0); } //C = 0; }

					if(((result_int & 000000200000)>>16) == 1) {SetCarry(1);} //C = 1}
					else {SetCarry(0);}
					//printf("**********\n src = %b \t src_int = %b \t dst = %b \t dst_int = %b \ ********", src, src_int, dst, dst_int);
/*

					r = result & 00000000000000010000000000000000; // <-- this is an octal #, not binary
					r = r >> 16;
					if (r == 1){
						C = 1;
						result = r;
					}
					else {
						C = 0;
						result = result & 00000000000000001000000000000000;
						result = result >> 15;
					}

					//src = src >> 15;
					//dst = dst >> 15;
					if ((src & 0100000) && (dst & 0100000)){
						if (((result & 0100000)>>15) != (src & 0100000)>>15){
							SetOverFlow(1); //V = 1;
						}
						else{
							SetOverFlow(0); //V = 0;
						}
					}
					else {
						SetOverFlow(0); //V = 0;
					}
*/
					if ((src > 0) && (dst > 0) && (result < 0)) {SetOverFlow(1);}
					else if ((src < 0) && (dst < 0) && (result >= 0)) {SetOverFlow(1);}
					else {SetOverFlow(0);}

					break;
		case 16	:										//SUB Instruction
					result = dst + (~src + (short)1);
					result_int = dst_int + (~src_int + (short)1);
					printf("SUB of %d - %d, Result = %d \n", dst, src, result);
					if (IR2.mode2 == 0){
						regs[IR2.reg2] = result;					//if mode 0 is used then we write the destination register
					}
					else{
						WriteMem(EA_dst, result);
					}

					if(result == 0) { SetZero(1);} //Z = 1; }
					else 		{ SetZero(0);} //Z = 0; }

					if(result < 0) { SetNegative(1);} //N = 1; }
					else 		{ SetNegative(0);} //N = 0; }


					//result = (~result) + 1;				//2's complement representation
/*
					printf("value of result after 2's complement representation = %d \n", result);
					r = result & 00000000000000010000000000000000;
					r = result >> 16;
					printf("bit 16 of r  = %d \n", r);
					if (r == 1){
						C = 0;
						result = r;
					}
					else {
						C = 1;
						result = result & 00000000000000001000000000000000;
						result = result >> 15;
						printf("bit 15 of result = %d \n", result);
					}
*/
					//if (((~src + (short)1) & 0100000) && (dst & 0100000)) { SetCarry(1);} //C = 1; }
					//else 	{ SetCarry(0);} //C = 0; }

					if((result_int & 000000200000) > 0) {SetCarry(1);} //C = 1}
					else {SetCarry(0);}

/*					result = result >> 15;
					src = src >> 15;
					dst = dst >> 15;
					if (src != dst){
						if (src == result){
							SetOverFlow(1); //V = 1;
						}
						else {
							SetOverFlow(0); //V = 0;
						}
					}
					else {
						SetOverFlow(0); //V = 0;
					}
*/
					if(((src > 0) && (dst < 0) && (result > 0)) || ((src < 0) && (dst > 0) && (result < 0))) {SetOverFlow(1);}
					else {SetOverFlow(0);}
					break;
		case 02	:	result = src + (~dst +(short)1);	//CMP Instruction
					result_int = src_int + (~dst_int +(short)1);
					printf("CMP result = %d \n", result);

					/*r = result;
					while (r) {
						if (r & 1)
							printf("1");
						else
							printf("0");

						r >>= 1;
					}
					printf("\n");*/
					if (result == 0){
						SetZero(1); //Z = 1;
					}
					else {
						SetZero(0); //Z = 0;
					}

					if (result<0){
						SetNegative(1); //N = 1;
					}
					else {
						SetNegative(0); //N = 0;
					}

/*					r = result >> 16;
					if (r == 1){
						C = 0;
						result = r;
					}
					else {
						C = 1;
						result = result >> 15;
					}

					src = src >> 15;
					dst = dst >> 15;
					if (src != dst){
						if (dst == result){
							V = 1;
						}
						else {
							V = 0;
						}
					}
					else {
						V = 0;
					}
*/
					//if (((dst >= 0) && (dst & 0x8000)) || (((dst < 0) && ((dst & 0x10000) != 0)))) {SetCarry(0);}
					//else {SetCarry(1);}

					//if((result_int & 000000200000) > 0) {SetCarry(1);} //C = 1}
						//	else {SetCarry(0);}

					if (((~dst + (short)1) & 0100000) && (src & 0100000)) { SetCarry(1);} //C = 1; }
										else 	{ SetCarry(0);} //C = 0; }

					if(((src > 0) && (dst < 0) && (result < 0)) || ((src < 0) && (dst > 0) && (result > 0))) {SetOverFlow(1);}
					else {SetOverFlow(0);}
					break;

		case 05:	result = src | dst;				//BIS Instruction -- Performs inclusive OR
					if (IR2.mode2 == 0){
						regs[IR2.reg2] = result;			//if mode 0 is used then we write the destination register
					}
					else{
						WriteMem(EA_dst, result);
					}
					if (result == 0){
						SetZero(1); //Z = 1;
					}
					else{
						SetZero(0); //Z = 0;
					}

					//result = result >> 15;
					if (result < 0){
						SetNegative(1); //N = 1;
					}
					else {
						SetNegative(0); //N = 0;
					}

					//SetCarry(psw_C); //C = C;
					SetOverFlow(0); //V = 0;
					break;

		case 04:	src = ~src;						//BIC instruction
					result = src & dst;
					if(IR2.mode2 == 0){
						regs[IR2.reg2] = result;			//if mode 0 is used then we write the destination register
					}
					else{
						WriteMem(EA_dst, result);
					}
					if (result == 0){
						SetZero(1); //Z = 1;
					}
					else {
						SetZero(0); //Z = 0;
					}

					//result = result >> 15;
					if (result < 0){
						SetNegative(1); //N = 1;
					}
					else{
						SetNegative(0); //N = 0;
					}
					//SetCarry(psw_C); //C = C;
					SetOverFlow(0); //V = 0;
					break;

		case 03:	result = src & dst;				//BIT instruction
					if (result == 0){
						SetZero(1); //Z = 1;
					}
					else{
						SetZero(0); //Z = 0;
					}

					//result = result >> 15;
					if (result < 0){
						SetNegative(1); //N = 1;
					}
					else{
						SetNegative(0); //N = 0;
					}
					//C = C;
					SetOverFlow(0); //V = 0;
					break;

		default:	break;
	}
}



/*
typedef struct doubleOp{
    unsigned int opcode;
    unsigned int w;
    unsigned int mode1;
    unsigned int reg1;
    unsigned int mode2;
    unsigned int reg2;
}double_Op;

double_Op IR2;
short regs[8];

int psw_N = 0;
int psw_Z = 0;
int psw_V = 0;
int psw_C = 0;
int DEBUG = 1;
int DISPLAY = 1;

void double_exec();

int main () {

    //Double_Op instr;    // FIELDs: w, opcode, mode, reg
    short expectval;
    unsigned int expectZ, expectN, expectV, expectC;

    int reg1 = 0;
	int reg2 = 1;

    IR2.w = 1;        // always a word
    IR2.mode1 = 0;
	IR2.mode2 = 0;
    IR2.reg1 = reg1;	//Src
	IR2.reg2 = reg2;	//Dst

    printf("DOUBLE TEST: -----> Using reg1 %d\t reg2 %d\n", reg1, reg2);

//// MOV Instruction ////
	printf("MOV \n");
    IR2.opcode      = 01;
    regs[reg1]      = (short) 0000000; //Src
	regs[reg2]		= (short) 0177777; //Dst

    expectval   = 0000000;

    expectZ     = 1;
    expectN     = 0;
    expectV     = 0;
    expectC     = 0;

    double_exec();

    printf("\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

    if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
            IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

    if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
    if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
    if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
    if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

//// ADD Instruction ////
	printf("ADD\n");
	IR2.opcode      = 06;
    regs[reg1]      = -32768; //(short) 0077777; //Src
	regs[reg2]		= -32768; //(short) 0077777; //Dst

    expectval   = 0;

    expectZ     = 1;
    expectN     = 0;
    expectV     = 1;
    expectC     = 1;

	printf("Before ADD \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

    double_exec();

	printf("After ADD \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

    if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
            IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

    if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
    if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
    if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
    if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

//// SUB Instruction ////
	printf("SUB\n");
	IR2.opcode      = 16;
    regs[reg1]      = 1; //(short) 0077777; //Src
	regs[reg2]		= -32768; //(short) 0077777; //Dst

    expectval   = -32768;

    expectZ     = 0;
    expectN     = 0;
    expectV     = 1;
    expectC     = 1;

	printf("Before SUB \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

    double_exec();

	printf("After SUB \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

    if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
            IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

    if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
    if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
    if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
    if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


    //// COMPARE Instruction ////
    	printf("CMP\n");
    	IR2.opcode      = 02;
        regs[reg1]      = -32768; //(short) 0077777; //Src
    	regs[reg2]		= 32767; //(short) 0077777; //Dst

        expectval   = 0;

        expectZ     = 0;
        expectN     = 0;
        expectV     = 1;
        expectC     = 1;

    	printf("Before CMP \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

        double_exec();

    	printf("After CMP \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

        if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
                IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

        if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
        if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
        if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
        if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


        //// BIS Instruction ////
           	printf("BIS\n");
           	IR2.opcode      = 05;
               regs[reg1]      = -32768; //(short) 0077777; //Src
           	regs[reg2]		= 32767; //(short) 0077777; //Dst

               expectval   = -1;

               expectZ     = 0;
               expectN     = 1;
               expectV     = 0;
               expectC     = 1;

           	printf("Before BIS \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

               double_exec();

           	printf("After BIS \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

               if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
                       IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

               if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
               if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
               if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
               if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }


        //// BIC Instruction ////
           	printf("BIC\n");
              	IR2.opcode      = 04;
                regs[reg1]      = 32767; //(short) 0077777; //Src
                regs[reg2]		= 32767; //(short) 0077777; //Dst

                expectval   = 0;

                expectZ     = 1;
                expectN     = 0;
                expectV     = 0;
                expectC     = 1;

           	printf("Before BIS \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

                double_exec();

              	printf("After BIS \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

                if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
                                     IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

                if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
                if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
                if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
                if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }



       //// BIT Instruction ////
       	printf("BIT\n");
           	IR2.opcode      = 03;
            regs[reg1]      = -32768; //(short) 0077777; //Src
          	regs[reg2]		= -32768; //(short) 0077777; //Dst

            expectval   = 0;

            expectZ     = 0;
            expectN     = 1;
            expectV     = 0;
            expectC     = 1;

       	printf("Before BIT \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

            double_exec();

           	printf("After BIT \t\treg1 = %06o \treg2 = %06o \tz%d n%d v%d c%d\n", (unsigned short) regs[reg1], (unsigned short) regs[reg2], psw_Z, psw_N, psw_V, psw_C);

            if (regs[IR2.reg2] != expectval) { printf ("ERROR, expected regs[%d] = %06o, got %06o instead!\n",
               IR2.reg2, (unsigned short) expectval, (unsigned short) regs[IR2.reg2]);  }

            if (psw_Z != expectZ) { printf ("ERROR, expected Z = %d, got %d instead!\n", expectZ, psw_Z); }
            if (psw_N != expectN) { printf ("ERROR, expected N = %d, got %d instead!\n", expectN, psw_N); }
            if (psw_V != expectV) { printf ("ERROR, expected V = %d, got %d instead!\n", expectV, psw_V); }
            if (psw_C != expectC) { printf ("ERROR, expected C = %d, got %d instead!\n", expectC, psw_C); }

}

*/
