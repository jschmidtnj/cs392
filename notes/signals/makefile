FILE_NAME=worker.out
PRIMES_FILE_NAME=primestest.out
GCC=gcc
FLAGS=-g -Wall -Werror -pedantic-errors

all:
	$(GCC) $(FLAGS) worker.c -o $(FILE_NAME)
	$(GCC) $(FLAGS) primestest.c -o $(PRIMES_FILE_NAME) -lm
clean:
	rm -f $(FILE_NAME) $(FILE_NAME).exe $(PRIMES_FILE_NAME) $(PRIMES_FILE_NAME).exe
