name = sort
flags = -Wall -Werror -pedantic-errors

sort: sort.o sorts.o
	gcc sort.o sorts.o -o $(name)

sort.o: sort.c
	gcc $(flags) -c sort.c

sorts.o: sorts.c sorts.h
	gcc $(flags) -c sorts.c

clean:
	rm -f *.o $(name) $(name).exe
