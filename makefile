CC=gcc

run: run.c 
	$(CC) -Wall -o run run.c emulator.c

emulator: emulator.c
	$(CC) -Wall -o emulator emulator.c 


all: run 	


clean: 
	rm emulator
	rm run

