OUT_FILE_CLIENT=client.out
OUT_FILE_SERVER=server.out
CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors

all: main
main: server.c client.c
	$(CC) $(CFLAGS) client.c -o $(OUT_FILE_CLIENT)
	$(CC) $(CFLAGS) server.c -o $(OUT_FILE_SERVER)
clean:
	rm -f $(OUT_FILE_CLIENT) $(OUT_FILE_CLIENT).exe $(OUT_FILE_SERVER) $(OUT_FILE_SERVER).exe
