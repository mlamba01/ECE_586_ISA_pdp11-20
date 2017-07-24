# ECE_586_ISA_pdp11-20

This is the implementation of PDP 11/ 20 Instruction Set Architecture Simulator which was originally developed by Digital Equipment Corporation in 1970’s. PDP 11/ 20 is a 16 bit architecture which was widely used in PDP 11 minicomputers.

Inputs:
The simulator takes an ASCII file containing 16 bit octal values as its input which will be produced by the provided object to ASCII program.

Outputs:
The simulator produces a memory trace file of the form n address where 
               n = 0 is data read 
               n = 1 is data write
               n = 2 is instruction read
and after the completion of the execution, the simulator should generate a brief summary including:
-> A memory trace file showing each instruction fetch and memory data read and write.
-> A summary, displayed on the screen upon completion of the simulation (execution of a HALT instruction) indicating the total number of instructions executed.
-> An optional display that (at every instruction fetch) succinctly indicates the contents of each register, the contents of the NZVC flags, and the currently fetched instruction.
