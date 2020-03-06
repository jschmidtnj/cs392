#/*******************************************************************************
# * Name        : makefile
# * Author      : Joshua Schmidt and Matt Evanago
# * Date        : 3/6/20
# * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : DLL's
# ******************************************************************************/

CC     = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors
OUT_FILE=main

all: main libsum.so

# build main
main: main.c
	$(CC) $(CFLAGS) -o $(OUT_FILE) main.c -ldl

# build libsum.so
libsum.so: sum.c
	$(CC) $(CFLAGS) -shared -fPIC -o libsum.so sum.c

clean:
	rm -f libsum.so $(OUT_FILE) $(OUT_FILE).exe
