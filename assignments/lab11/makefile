#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/24/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : find the server makefile
# ******************************************************************************/

CC     = gcc
C_FILE = findserver.c
TARGET = findserver
CFLAGS = -g -Wall -Werror -pedantic-errors -pthread

all: findserver.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
