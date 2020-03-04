#/*******************************************************************************
# * Name        : makefile
# * Author      : Joshua Schmidt
# * Date        : 2/10/20
# * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
# * Description : makefile for quicksort
# ******************************************************************************/

name = sort
flags = -Wall -Werror -pedantic-errors

sort: sort.o quicksort.o
	gcc $(flags) sort.o quicksort.o -o $(name)

sort.o: sort.c
	gcc $(flags) -c sort.c

quicksort.o: quicksort.c quicksort.h
	gcc $(flags) -c quicksort.c

clean:
	rm -f *.o $(name) $(name).exe
