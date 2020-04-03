#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/3/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : snooze for n seconds
# ******************************************************************************/

CC     = gcc
C_FILE = snooze.c
TARGET = snooze
CFLAGS = -Wall -Werror -pedantic-errors

all: snooze.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
