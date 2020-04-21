#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/16/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : multi-threaded seive makefile
# ******************************************************************************/

CC     = gcc
C_FILE = mtsieve.c
TARGET = mtsieve
CFLAGS = -Ofast -Wall -Werror -pedantic-errors -lm -lpthread

all: mtsieve.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
