all: CSORT

CSORT: CSORT.o
	gcc -o CSORT CSORT.o

CSORT.o: CSORT.c CSORT.h
	gcc -c CSORT.c
