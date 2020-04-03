#/*******************************************************************************
# * Name   : makefile
# * Author : Joshua Schmidt and Matt Evanago
# * Date   : 4/3/20
# * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : minishell
# ******************************************************************************/

CC     = gcc
C_FILE = minishell.c
TARGET = minishell
CFLAGS = -Wall -g -Werror -pedantic-errors

all: minishell.c
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
