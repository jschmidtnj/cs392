#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/27/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : chat client makefile
# ******************************************************************************/

CC     = gcc
C_FILE = chatclient.c
TARGET = chatclient
SERVER_FILE = chatserver.c
SERVER_TARGET = chatserver
CFLAGS = -g -Wall -Werror -pedantic-errors

all: chatclient.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
server:
	$(CC) $(CFLAGS) $(SERVER_FILE) -o $(SERVER_TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe $(SERVER_TARGET) $(SERVER_TARGET).exe
