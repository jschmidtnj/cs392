#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/17/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : get sum with threads
# ******************************************************************************/

CC     = gcc
C_FILE = mtsumarray.c
TARGET = mtsumarray
CFLAGS = -g -Wall -Werror -pedantic-errors -pthread

all: mtsumarray.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
