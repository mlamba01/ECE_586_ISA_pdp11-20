#include <stdlib.h>
#include <stdio.h>
#include "processinstruction.h"
#include "EA_calc.h"

extern short regs[8];

unsigned int Eff_Address (unsigned int mode, unsigned int reg, int word){
	unsigned int EA, X, offset;

	switch(mode){
		case 1:	EA = regs[reg];				          //Register-Deferred Mode
				break;
		case 2:		EA = regs[reg];                         // Auto increment
					if(word == 1){		         //Word instruction
						regs[reg] += 2;
					}
					else{				//Byte instruction
						regs[reg] += 1;
					}
				break;
		case 3:		                                         //Deferred Auto-increment
				EA = ReadMem(regs[reg], MEMREAD_DATA);
				if(word == 1){		         //Word instruction
					regs[reg] += 2;
				}
				else{				//Byte instruction
					regs[reg] += 1;
				}
				break;
		case 4:	if(word == 1){			                 //Auto-decrement
					regs[reg] -= 2;			//Word instruction
				}
				else{					//Byte instruction
					regs[reg] -= 1;
				}
				EA = regs[reg];
				break;
		case 5:	if(word == 1){			                //Deferred Auto-decrement
					regs[reg] -= 2;			//Word instruction
				}
				else{					//Byte instruction
					regs[reg] -= 1;
				}
				EA = ReadMem(regs[reg], MEMREAD_DATA);
				break;
		case 6:	if (regs[reg] != 7){			                //Index Addressing
					X = ReadMem(regs[7], MEMREAD_DATA);
					offset = regs[reg];
					EA = X + offset;
					regs[7] += 2;
				}
				else{					                //Relative Addressing --> PC mode
					X = ReadMem(regs[7],MEMREAD_DATA);
					regs[7] += 2;
					EA = regs[7] + X;
				}
				break;
		case 7:	if (regs[reg] != 7){			                //Deferred Index Mode
					X = ReadMem(regs[7], MEMREAD_DATA);
					offset = regs[reg];
					EA = ReadMem(X + offset, MEMREAD_DATA);
					regs[7] += 2;
				}
				else {					                //Deferred Relative Addressing --> PC mode
					X = ReadMem(regs[7],MEMREAD_DATA);
					regs[7] += 2;
					EA = ReadMem(regs[7] + X, MEMREAD_DATA);
				}
				break;
		default: break;
	}
return EA;
}
