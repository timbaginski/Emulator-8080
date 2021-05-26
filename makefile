CC=gcc

emulator: emulator.c
	$(CC) -Wall -o emulator emulator.c 

all: emulator

clean: 
	rm emulator

