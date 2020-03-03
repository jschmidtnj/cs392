#/*******************************************************************************
# * Name        : makefile
# * Author      : Joshua Schmidt
# * Date        : 3/3/20
# * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : makefile for pfind
# ******************************************************************************/

OUT_FILE=pfind
CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors

all: main
main: pfind.c
	$(CC) $(CFLAGS) pfind.c -o $(OUT_FILE)
clean:
	rm -f $(OUT_FILE) $(OUT_FILE).exe
