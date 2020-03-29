#/*******************************************************************************
# * Name        : makefile
# * Author      : Joshua Schmidt and Matt Evanago
# * Date        : 3/29/20
# * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : makefile for spfind
# ******************************************************************************/

OUT_FILE=spfind
CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors

all: main
main: spfind.c
	$(CC) $(CFLAGS) spfind.c -o $(OUT_FILE)
clean:
	rm -f $(OUT_FILE) $(OUT_FILE).exe
