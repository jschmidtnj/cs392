OUT_FILE_ALARM=alarms.out
OUT_FILE_PROMPT=prompt.out
CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors

all: main
main: alarms.c prompt.c
	$(CC) $(CFLAGS) alarms.c -o $(OUT_FILE_ALARM)  -lm
	$(CC) $(CFLAGS) prompt.c -o $(OUT_FILE_PROMPT)  -lm
clean:
	rm -f $(OUT_FILE) $(OUT_FILE).exe
