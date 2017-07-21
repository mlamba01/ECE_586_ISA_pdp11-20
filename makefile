CC=gcc

all:  main.c
	$(CC)  -o pdp11Sim main.c processinstruction.c IR_Parse.c single_exec.c double_exec.c branch_exec.c EA_calc.c -I.
