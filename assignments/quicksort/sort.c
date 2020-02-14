/*******************************************************************************
 * Name        : sort.c
 * Author      : Joshua Schmidt
 * Date        : 2/10/20
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv) {
    char **input_data[MAX_ELEMENTS];
    for (int i = 0; i < argc - 1; i++, (*input_data)++) {
        *input_data = (char *) malloc(sizeof(*argv++));
    }
    return EXIT_SUCCESS;
}
