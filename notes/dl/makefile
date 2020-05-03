CC=gcc
CFLAGS=-g -Wall -Werror -pedantic-errors
OUT_FILE=demo

all: demo dl1.so dl2.so
# -ldl links to dynamic linking library
demo: demo.c
	$(CC) $(CFLAGS) -o $(OUT_FILE) demo.c -ldl
# fPIC = position independent code (optional)
dl1.so: dl1.c
	$(CC) $(CFLAGS) -shared -fPIC -o dl1.so dl1.c
dl2.so: dl2.c
	$(CC) $(CFLAGS) -shared -fPIC -o dl2.so dl2.c
clean:
	rm -f *.so $(OUT_FILE) $(OUT_FILE).exe
