OUT_FILE=main
CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors

all: main
# stop after the preprocessing stage; do not run the compiler proper
# gcc -E main.c -o main.i
# stop after state of compilation proper; do not assemble
# gcc -S main.i -o main.s
# compile or assemble the source files, but do not link
# gcc -c main.s -o main.o
# gcc main.o -o main
main: main.c libadd.a
	$(CC) $(CFLAGS) main.c -o ${OUT_FILE} libadd.a
add.o: add.c add.h
	$(CC) $(CFLAGS) -c add.c -o add.o
# linker
libadd.a: add.o
	ar rs libadd.a add.o
clean:
	rm -f *.i *.s *.o *.a ${OUT_FILE} ${OUT_FILE}.exe
